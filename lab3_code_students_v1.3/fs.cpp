#include <iostream>
#include "fs.h"

// todo: skapa help funktioner för:
// --Ngt fucked händer när create har större size än 16 data bokstäver
// 1. see om filepath är absolute filepath eller CWD filename
//      detta möjligör detta lättare att förstå användarens input
// 2. få vilket block man arbetar i för current directory
//      detta gör att man lättare kan arbeta i CWD, underlättar massa redundans.
// 3. kolla om filnamn redan finns i directory.
// 4. det går enbart att skapa en fil utan att man fuckar upp minnet med sanitizer
// 5. gå igenom memory leaks i create file
// 6. ev. sätt data till BLOCK_SIZE
// 7. Filnamn ska inte ha med sin directory i sig.
// 8. Create o cat verkar ha problem med filer större än BLOCK_SIZE
    //  det är alltså något fel på antingen skrivningen av det på disk eller avläsningen.
// 9. en tom file verkar ha size 0 fortfarande, så ls listar den inte.
// 10. disk write är fucked med faten. aj karamba aja
    //  filer större än BLOCK_SIZE funkar ej kekleon
    // det är inte fel på fat, det är fel på skrivningen.
// 11. fixat, nu är det bara index som skrivs åt fel håll. ja eller så är det något somfortfarande är fel.
// 12. När man skriver på disk sker ofta heap overflow med en karaktär. kolla upp detta sker även på klara.
// 13. när fil adderas till directory ska size ökas, annars börjar den på ett  pga ".." filen
// 14. mv behöver fixas, filer ändrar inte namn.
// 15. Om det får ej finnas mer än 64 entries i en dictionary
// 16. fixa för absolute paths!
// 17. om man tar bort en fil och sedan lägger till den
//      och ssedan appendar den till annan fil så fuckas blocken upp? yes.
// ska hantera /mnt/c/Users/lukas/Downloads/Assignments/VT23/OS/gh/DV1629/lab3_code_students_v1.3
//      och /mnt/c/Users/lukas/Downloads/Assignments/VT23/OS/gh/DV1629/lab3_code_students_v1.3/
// ev gör om current_dir_block och current_working_dir till this->
// TODO: just nu, om dir inte finns så kmr den till root_dir, vilket är ok
    //  --> viktigast, löser alla andra problem. men se om du kan göra så att cerr och returnerar "No such dir"
    // sedan kan den föflytta sig mellan dir -> annat dir utan gå igenom root.
    // CURRY//dir/test/test/test

// g++ -std=c++11 -o filesystem main.o shell.o disk.o fs.o -fsanitize=address

// runtests:
// failade att ta hand om det finns mer än 64 entries i en dir
// rm måste kolla om filen finns först..
// mkdir måste kolla om fil redan finns..
// pwd verkar inte fungera korrekt om man går tbx till root dir
// behöver fixa privilegium kontroller..
// varför måste en fil vara minst 16 bitar?
// TASK 1:
// varför måste en fil vara minst 16 bitar?
// TASK 2:
// Khorosho
// TASK 3:
// absolute paths: cp och mv OK,create kvar: rm, append, cat, chmod, mkdir
    // rmdir används för att ta bort dir i ubuntu, behöver ej beakta dirs i rm
// gör hjälp funktioner när du arbetar med absolute paths.
// task 4:
// man kan mv en fil a till root även om a redan finns i root

// alles klar med testerna, ska fixa paths för rm, append, cat, chmod
// det finns en sak med testerna i test1. sizen är fel! debugga. ev måste size vara minst 16.
// EV: så om size < 16 -> size = 16. klart
// mv kan inte föra över större filer. klart
// ls listar inte dirs först, försumbart?
// from lab desc: Access rights of a file or directory should be ’rw-’ or ’rwx’ when the file or directory is created.


// Constructor
FS::FS()
{
    current_dir_block = ROOT_BLOCK; //private class variable
    current_working_dir = "/"; //private class variable
}

// Destructor
FS::~FS()
{

}

// formats the disk, i.e., creates an empty file system
int
FS::format()
{
    // block nr 0 is root dir, format all entries.
    dir_entry* root_dir = new dir_entry[BLOCK_SIZE/DIR_ENTRY_SIZE];
    for (size_t i = 0; i < BLOCK_SIZE/DIR_ENTRY_SIZE; i++) {
        root_dir[i].file_name[0] = ' ';
        root_dir[i].size = 0;
        root_dir[i].first_blk = ROOT_BLOCK; // tmp
        root_dir[i].type = TYPE_DIR;
        root_dir[i].access_rights = READ + WRITE + EXECUTE;
    }
    this->disk.write(ROOT_BLOCK, (uint8_t*)root_dir);
    delete[] root_dir;
    // block nr 1 is File Allocation Table, set all blocks as free except first two blocks.
    int16_t* fat = new int16_t[BLOCK_SIZE/2]; // Whole FAT in one block, we can address 4096 / 2 = 2048 disk blocks in a partition
    fat[0] = FAT_EOF; fat[1] = FAT_EOF; // first two blocks are occupied
    for (size_t i = 2; i < BLOCK_SIZE/2; i++) {
        fat[i] = FAT_FREE;
    }
    this->disk.write(FAT_BLOCK, (uint8_t*)fat);
    this->current_working_dir = "/"; // set CWD to root dir
    this->current_dir_block = ROOT_BLOCK;
    delete[] fat;
    return 0;
}

// This function takes the filename from a filepath
std::string FS::get_filename(std::string filepath) {
    std::string filename;
    if (filepath.find("/") != std::string::npos) {
        filename = filepath.substr(filepath.find_last_of("/") + 1);
    } else {
        filename = filepath;
    }
    return filename;
}

// This function gathers info about a new dir entry, used in the create() function.
std::string FS::gather_info_new_dir_entry(int file_type, dir_entry* new_file, std::string filename){
    // Start of new directory entry
    strncpy(new_file->file_name, filename.c_str(), sizeof(filename));
    new_file->access_rights = READ + WRITE + EXECUTE; // Access rights of a file or directory should be ’rw-’ or ’rwx’ when the file or directory is created. (7)
    new_file->type = file_type;

    // Gather user input information
    std::string line;
    std::string data = "";
    std::getline(std::cin, line);
    while(line != "") {
        data += line + "\n";
        std::getline(std::cin, line);
    }
    if (data.size() < 16) {
        data.resize(16, *" "); // size of file needs to be larger than 16 för att vara en entry
    }
    new_file->size = data.size();
    return data;
}

// This function has different return values for checking if a file exist, is a file or a dir
int FS::check_if_file_in_dir(dir_entry* dir, int file_type, const char file_name[56]){
    for (size_t i = 0; i < BLOCK_SIZE/DIR_ENTRY_SIZE; i++) {
        if (strcmp(dir[i].file_name, file_name) == 0) {
            if (dir[i].type == TYPE_FILE) {
                return -1;
            }
            if (dir[i].type == TYPE_DIR) {
                return -2;
            }
        }
    }
    return 0;
}

// This function gathers info about free blocks and then
// writes the data and updates the FAT table
int FS::save_entry_on_disk(std::string data, int16_t* fat, dir_entry* new_file){
    float no_needed_blocks_fl = (float)data.size() / BLOCK_SIZE;
    int no_needed_blocks = 1 + floor(no_needed_blocks_fl);
    std::vector<int> free_blocks;
    for (size_t i = 2; i < BLOCK_SIZE/2; i++) {
        if (fat[i] == FAT_FREE) {
            if (no_needed_blocks > 1) {
                free_blocks.push_back(i);
                no_needed_blocks--;
            } else {
                free_blocks.push_back(i);
                break;
            }
        }
        if (i == (BLOCK_SIZE/2-1)) {
            std::cerr << "Disk is full." << '\n';
            return -1;
        }
    }
    new_file->first_blk = free_blocks[0];
    int writing_block_size = BLOCK_SIZE-1; //-1
    std::string data_part;
    if (free_blocks.size() == 1) {
        fat[free_blocks[0]] = FAT_EOF;
        this->disk.write(free_blocks[0], (uint8_t*)(char*)data.c_str()); //
    } else {
        int i = 0;
        while (i <= free_blocks.size()) {
            if (i+1 < free_blocks.size()) {
                fat[free_blocks[i]] = free_blocks[i+1];
                data_part = data.substr(i*writing_block_size, (i+1)*writing_block_size); // EV. här ligger felet! fixed
                this->disk.write(free_blocks[i], (uint8_t*)(char*)data_part.c_str());
                i++;
            } else {
                data_part = data.substr(i*writing_block_size, (i+1)*writing_block_size);
                this->disk.write(free_blocks[i], (uint8_t*)(char*)data_part.c_str());
                fat[free_blocks[i]] = FAT_EOF;
                break;
            }
        }
    }
    this->disk.write(FAT_BLOCK, (uint8_t*)fat);
    return 0;
}

// This function saves the new file info in the given directory
int FS::save_entry_in_dir(int current_dir_block, dir_entry* dir, dir_entry* new_file){
    for (size_t i = 0; i < BLOCK_SIZE/DIR_ENTRY_SIZE; i++) {
        // std::cout << current_working_dir[i].file_name << '\t';
        if (dir[i].size == 0) {
            memcpy(&dir[i], new_file, sizeof(dir_entry));
            break;
        }
    }
    this->disk.write(current_dir_block, (uint8_t*)dir);
    return 0;
}

// This function checks wheather a direcotyr is full or not
int FS::check_if_dir_full(int dir_block){
    dir_entry* dir = new dir_entry[BLOCK_SIZE/DIR_ENTRY_SIZE];
    int debug = this->disk.read(dir_block, (uint8_t*)dir);
    int count = 0;
    int index = 0;
    while (index < BLOCK_SIZE/DIR_ENTRY_SIZE) {
        if (dir[index].size != 0) {
            count++;
        }
        index++;
    }
    if (count == 64) {
        return -1;
    }
    return 0;
}


// create <filepath> creates a new file on the disk, the data content is
// written on the following rows (ended with an empty row)
int
FS::create(std::string filepath)
{
    // gather info about dir entry
    dir_entry* new_file = new dir_entry;
    std::string filename = get_filename(filepath);

    // check errors:
    if (filename.size() > 55) {
        std::cerr << "Filename is too long (>55 characters)." << '\n';
        delete new_file;
        return -1;
    }
    if (this->check_if_dir_full(current_dir_block) == -1) {
        std::cerr << "Directory is full." << '\n';
        delete new_file;
        return -1;
    }

    // gather info about new entry
    std::string data = this->gather_info_new_dir_entry(TYPE_FILE, new_file, filename);

    // check if file already exists in dir
    dir_entry* dir = new dir_entry[BLOCK_SIZE/DIR_ENTRY_SIZE];
    int debug = this->disk.read(current_dir_block, (uint8_t*)dir);
    int dest_dir_bool = destination_dir_check(dir, filepath, filename);
    if (check_if_file_in_dir(dir, new_file->type, new_file->file_name) != 0) {
        std::cerr << "There already exist a file with that name." << '\n';
        delete[] dir; delete new_file;
        return -1;
    }

    // save entry on disk
    uint8_t* tmp = new uint8_t[BLOCK_SIZE];
    debug = this->disk.read(FAT_BLOCK, tmp);
    int16_t* fat = (int16_t*)tmp;
    if (FS::save_entry_on_disk(data, fat, new_file) != 0) {
        // cerr in function, memory full
        delete[] fat;
        return -1;
    }
    delete[] fat;

    //add to dir
    if (dest_dir_bool == 0) {
        std::cerr << "Use mkdir to create a directory." << '\n';
        delete new_file; delete[] dir;
        return -1;
    } else {
        debug = FS::save_entry_in_dir(current_dir_block, dir, new_file);
    }
    delete new_file; delete[] dir;
    return 0;
}

// cat <filepath> reads the content of a file and prints it on the screen
int
FS::cat(std::string filepath)
{
    // gather input info
    std::string filename = get_filename(filepath);
    if (filename.size() > 55) {
        std::cerr << "Filename is too long and doesn't exist (>55 characters)." << '\n';
        return -1;
    }

    dir_entry* dir = new dir_entry[BLOCK_SIZE/DIR_ENTRY_SIZE];
    int debug = this->disk.read(current_dir_block, (uint8_t*)dir);

    // see if destname is a dir
    int dest_dir_bool = destination_dir_check(dir, filepath, filename);
    int dir_block = current_dir_block;

    // see if file can be found and where its first block is
    int index_block;
    int file_found = -1;
    int file_size;

    // if file is dir but in pwd -> cannot cat a dir
    if (dest_dir_bool == 0 && check_if_file_in_dir(dir, TYPE_DIR, filename.c_str()) != 0) {
        std::cerr << "File is a directory." << '\n';
        delete[] dir;
        return -1;
    }

    // if destination is a dir but not in pwd -> get the file wanted.
    if (dest_dir_bool == 0) {
        // dest is directory, get block
        filepath = filepath.substr(0, filepath.rfind("/")+1);
        std::string original_path = current_working_dir;
        std::vector<int> blocks = get_dir_blocks(filepath);
        current_working_dir = original_path;
        for (size_t i = 0; i < blocks.size(); i++) {
            if (blocks[i] == -1) {
                std::cerr << "Directory was not found." << '\n';
                delete[] dir;
                return -1;
            }
        }
        dir_block = blocks[blocks.size()-1];
        debug = this->disk.read(dir_block, (uint8_t*)dir);
    }
    for (size_t i = 0; i < BLOCK_SIZE/DIR_ENTRY_SIZE; i++) {
        if (dir[i].file_name == filename) {
            if (dir[i].type == TYPE_DIR) {
                std::cerr << "File is a directory." << '\n';
                delete[] dir;
                return -1;
            }
            if (privilege_check(dir[i].access_rights, READ) != 0) {
                std::cerr << "You dont have the permission to read this file." << '\n';
                delete[] dir;
                return -2;
            }
            index_block = dir[i].first_blk;
            file_size = dir[i].size;
            file_found = 0;
            break;
        }
    }
    if (file_found != 0) {
        delete[] dir;
        std::cerr << "File not found." << '\n';
        return -1;
    }
    delete[] dir;

    // Read each block via FAT
    uint8_t* tmp = new uint8_t[BLOCK_SIZE];
    debug = this->disk.read(FAT_BLOCK, tmp);
    int16_t* fat = (int16_t*)tmp;
    uint8_t* block = new uint8_t[BLOCK_SIZE];
    std::string file_data;
    int count = 0;
    do {
        debug = this->disk.read(index_block, block);
        std::string file_data = (char*)block;
        std::cout << file_data;
        index_block = fat[index_block];
        count++;
    } while (index_block != FAT_EOF && index_block != FAT_FREE);
    delete[] fat, block;
    return 0;
}

// returns the priviliege int in string form
std::string FS::privilege_string(uint8_t privilege) {
    switch(privilege) {
        case 4:
            return "r--"; // 100
        case 2:
            return "-w-"; // 010
        case 1:
            return "--x"; // 001
        case 6:
            return "rw-"; // 110
        case 7:
            return "rwx"; // 111
        case 5:
            return "r-x"; // 101
        case 3:
            return "-wx"; // 011
    }
    return "---"; // 000
}

// ls lists the content in the currect directory (files and sub-directories)
int
FS::ls()
{
    dir_entry* dir = new dir_entry[BLOCK_SIZE/DIR_ENTRY_SIZE];
    int debug = this->disk.read(current_dir_block, (uint8_t*)dir);
    std::cout << "name\t type\t accessrights\t size" << '\n';
    for (size_t i = 0; i < BLOCK_SIZE/DIR_ENTRY_SIZE; i++) {
        if (dir[i].size != 0) {
            std::cout << dir[i].file_name << "\t ";
            if (dir[i].type == TYPE_FILE) {
                std::cout << "file" << "\t ";
                std::cout << privilege_string(dir[i].access_rights) << "\t ";
                std::cout << dir[i].size << "\t ";
            } else {
                std::cout << "dir" << "\t ";
                std::cout << privilege_string(dir[i].access_rights) << "\t ";
                std::cout << "-" << "\t ";
            }
            std::cout << '\n';
        }
    }
    delete[] dir;
    return 0;
}

// This function gathers data from already existing entry, used in cp and append functions
std::string FS::gather_info_old_dir_entry(dir_entry* dir, dir_entry* new_file, std::string filename){
    // see if file can be found and where its first block is
    int index_block;
    int file_size;
    int debug;

    // gather info on existing file
    for (size_t i = 0; i < BLOCK_SIZE/DIR_ENTRY_SIZE; i++) {
        if (dir[i].file_name == filename) {
            index_block = dir[i].first_blk;
            file_size = dir[i].size;
            new_file->size = dir[i].size;
            new_file->access_rights = dir[i].access_rights;
            break;
        }
    }

    // Read each block via FAT, gather the entry's data
    uint8_t* tmp = new uint8_t[BLOCK_SIZE];
    debug = this->disk.read(FAT_BLOCK, tmp);
    int16_t* fat = (int16_t*)tmp;
    uint8_t* block = new uint8_t[BLOCK_SIZE];
    std::string file_data;
    int count = 0;
    do {
        debug = this->disk.read(index_block, block);
        file_data += (char*)block;
        index_block = fat[index_block];
        count++;
    } while (index_block != FAT_EOF && index_block != FAT_FREE);
    delete[] fat, block; // här är det memory leaks någonstans EV
    return file_data; // return the daat
}

// Check if the filepath given includes another direcotory, not PWD
int FS::destination_dir_check(dir_entry* dir, std::string destpath, std::string destname){
    int dest_dir_bool = -1;
    if (destpath.find("/") != std::string::npos || destpath.find(".") != std::string::npos ) {
        dest_dir_bool = 0;
    } else {
        for (size_t i = 0; i < BLOCK_SIZE/DIR_ENTRY_SIZE; i++) {
            if (strcmp(dir[i].file_name, destname.c_str()) == 0) {
                if (dir[i].type == TYPE_DIR) {
                    dest_dir_bool = 0;
                }
            }
        }
    }
    return dest_dir_bool;
}

// cp <sourcepath> <destpath> makes an exact copy of the file
// <sourcepath> to a new file <destpath>
int
FS::cp(std::string sourcepath, std::string destpath)
{
    // gather info about dir entry
    dir_entry* new_file = new dir_entry;
    std::string sourcename = get_filename(sourcepath);
    std::string destname = get_filename(destpath);
    if (sourcename.size() > 55 || destname.size() > 55) {
        std::cerr << "Filename of destination/source file is too long (>55 characters)." << '\n';
        delete new_file;
        return -1;
    }
    new_file->type = TYPE_FILE;


    dir_entry* dir = new dir_entry[BLOCK_SIZE/DIR_ENTRY_SIZE];
    dir_entry* dest_dir = new dir_entry[BLOCK_SIZE/DIR_ENTRY_SIZE];
    int debug = this->disk.read(current_dir_block, (uint8_t*)dir);
    // check if source exist in cwd
    if (check_if_file_in_dir(dir, TYPE_FILE, sourcename.c_str()) == 0) {
        delete[] dir, dest_dir; delete new_file;
        std::cerr << "That sourcefile doesnt exist." << '\n';
        return -1;
    }
    // check if source is a dir
    if (check_if_file_in_dir(dir, TYPE_DIR, sourcename.c_str()) == -2) {
        delete[] dir, dest_dir; delete new_file;
        std::cerr << "That sourcefile is a dir." << '\n';
        return -1;
    }

    // see if destname is a dir
    int dest_dir_bool = destination_dir_check(dir, destpath, destname);
    int dir_block = current_dir_block;
    if (dest_dir_bool == 0) {
        // dest is directory, get the correct block
        strncpy(new_file->file_name, sourcename.c_str(), sizeof(sourcename));
        std::string original_path = current_working_dir;
        std::vector<int> blocks = get_dir_blocks(destpath);
        current_working_dir = original_path;
        for (size_t i = 0; i < blocks.size(); i++) {
            if (blocks[i] == -1) {
                std::cerr << "Directory was not found." << '\n';
                delete[] dir, dest_dir; delete new_file;
                return -1;
            }
        }
        dir_block = blocks[blocks.size()-1];
        if (this->check_if_dir_full(dir_block) == -1) {
            std::cerr << "Directory is full." << '\n';
            delete[] dir, dest_dir; delete new_file;
            return -1;
        }
        // see if file already in dest directory
        debug = this->disk.read(dir_block, (uint8_t*)dest_dir);
        if (check_if_file_in_dir(dest_dir, new_file->type, new_file->file_name) != 0) {
            std::cerr << "There already exist a file with that name in that directory." << '\n';
            delete[] dir, dest_dir; delete new_file;
            return -1;
        }
    } else {
        strncpy(new_file->file_name, destname.c_str(), sizeof(destname));
        if (this->check_if_dir_full(current_dir_block) == -1) {
            std::cerr << "Directory is full!" << '\n';
            delete[] dir, dest_dir; delete new_file;
            return -1;
        }
        // see if dest in CWD
        if (check_if_file_in_dir(dir, new_file->type, new_file->file_name) != 0) {
            std::cerr << "There already exist a file with that name." << '\n';
            delete[] dir, dest_dir; delete new_file;
            return -1;
        }
    }

    std::string data = this->gather_info_old_dir_entry(dir, new_file, sourcename);

    // save entry on disk
    uint8_t* tmp = new uint8_t[BLOCK_SIZE];
    debug = this->disk.read(FAT_BLOCK, tmp);
    int16_t* fat = (int16_t*)tmp;
    if (FS::save_entry_on_disk(data, fat, new_file) != 0) {
        delete[] dir, dest_dir; delete new_file;
        return -1;
    }
    delete[] fat;

    //add to entry to dir
    if (dest_dir_bool == 0) {
        debug = FS::save_entry_in_dir(dir_block, dest_dir, new_file);
    } else {
        debug = FS::save_entry_in_dir(current_dir_block, dir, new_file);
    }
    delete[] dir, dest_dir; delete new_file;
    return 0;
}

// mv <sourcepath> <destpath> renames the file <sourcepath> to the name <destpath>,
// or moves the file <sourcepath> to the directory <destpath> (if dest is a directory)
int
FS::mv(std::string sourcepath, std::string destpath)
{
    std::string sourcename = get_filename(sourcepath);
    std::string destname = get_filename(destpath);
    // error checking
    if (sourcename.size() > 55 || destname.size() > 55) {
        std::cerr << "Filename of destination/source file is too long (>55 characters)" << '\n';
        return -1;
    }

    // check if it exists in dir
    dir_entry* dir = new dir_entry[BLOCK_SIZE/DIR_ENTRY_SIZE];
    dir_entry* dest_dir = new dir_entry[BLOCK_SIZE/DIR_ENTRY_SIZE];
    int debug = this->disk.read(current_dir_block, (uint8_t*)dir);
    // check if source exist.
    if (check_if_file_in_dir(dir, TYPE_FILE, sourcename.c_str()) == 0) {
        delete[] dir, dest_dir;
        std::cerr << "That sourcefile doesnt exist." << '\n';
        return -1;
    }

    // see if destname is a dir
    int dest_dir_bool = destination_dir_check(dir, destpath, destname);
    int dir_block = current_dir_block;
    if (dest_dir_bool == 0) {
        // dest is directory, get block
        std::string original_path = current_working_dir;
        std::vector<int> blocks = get_dir_blocks(destpath);
        current_working_dir = original_path;
        for (size_t i = 0; i < blocks.size(); i++) {
            if (blocks[i] == -1) {
                std::cerr << "Directory was not found." << '\n';
                delete[] dir, dest_dir;
                return -1;
            }
        }
        dir_block = blocks[blocks.size()-1];

        // see if file already in dest directory etc
        debug = this->disk.read(dir_block, (uint8_t*)dest_dir);
        if (this->check_if_dir_full(dir_block) == -1) {
            std::cerr << "Directory is full." << '\n';
            delete[] dir, dest_dir;
            return -1;
        }
        if (check_if_file_in_dir(dest_dir, TYPE_FILE, sourcename.c_str()) != 0) {
            std::cerr << "There already exist a file with that name in that directory." << '\n';
            delete[] dir, dest_dir;
            return -1;
        }
    } else {
        // see if dest in CWD, if not -> ok write.
        if (check_if_file_in_dir(dir, TYPE_FILE, destname.c_str()) != 0) {
            std::cerr << "There already exist a file with that name." << '\n';
            delete[] dir, dest_dir;
            return -1;
        }
    }

    // move files
    dir_entry* entry_handler = new dir_entry;
    for (size_t i = 0; i < BLOCK_SIZE/DIR_ENTRY_SIZE; i++) {
        if (strcmp(dir[i].file_name, sourcename.c_str()) == 0) {
            if (dir[i].type == TYPE_FILE) {
                // get info about entry
                memcpy(entry_handler, &dir[i], sizeof(dir_entry));
                // see if only change its name
                if (dest_dir_bool != 0) {
                    strncpy(dir[i].file_name, destname.c_str(), sizeof(destname));
                    this->disk.write(current_dir_block, (uint8_t*)dir);
                    delete[] dir, dest_dir; delete entry_handler;
                    return 0;
                } else {
                    dir[i].file_name[0] = ' ';
                    dir[i].size = 0;
                    dir[i].first_blk = current_dir_block; // EV
                    dir[i].type = TYPE_DIR;
                    dir[i].access_rights = READ + WRITE + EXECUTE;
                    this->disk.write(current_dir_block, (uint8_t*)dir);
                    break;
                }
            } else { // EV, kanske funkar
                std::cerr << "Cannot move directory." << '\n';
                delete[] dir, dest_dir;
                return -1;
            }
        }
    }
    // then add to new dir if dir was destination
    if (dest_dir_bool == 0) {
        if (check_if_dir_full(dir_block)) {
            delete[] dir, dest_dir; delete entry_handler;
            std::cerr << "Destination directory is full." << '\n';
            return -1;
        }
        for (size_t i = 0; i < BLOCK_SIZE/DIR_ENTRY_SIZE; i++) {
            if (dest_dir[i].size == 0) {
                memcpy(&dest_dir[i], entry_handler, sizeof(dir_entry));
                this->disk.write(dir_block, (uint8_t*)dest_dir);
                delete[] dir, dest_dir; delete entry_handler;
                return 0;
            }
        }
    }
    return 0;
}

// rm <filepath> removes / deletes the file <filepath>
int
FS::rm(std::string filepath)
{
    std::string filename = get_filename(filepath);
    if (filename.size() > 55) {
        std::cerr << "Filename is too long (>55 characters)." << '\n';
        return -1;
    }
    dir_entry* dir = new dir_entry[BLOCK_SIZE/DIR_ENTRY_SIZE];
    int debug = this->disk.read(current_dir_block, (uint8_t*)dir);
    // see if in dir, if not -> error.
    // see if destname is a dir
    int dest_dir_bool = destination_dir_check(dir, filepath, filename);
    int dir_block = current_dir_block;

    // check if file already exists
    int found = -1;
    for (size_t i = 0; i < BLOCK_SIZE/DIR_ENTRY_SIZE; i++) {
        if (strcmp(dir[i].file_name, filename.c_str()) == 0) {
            found = 0;
            if (dir[i].type == TYPE_FILE) {
                int index_block;
                index_block = dir[i].first_blk;
                dir[i].file_name[0] = ' ';
                dir[i].size = 0;
                dir[i].first_blk = current_dir_block; // EV
                dir[i].type = TYPE_DIR;
                dir[i].access_rights = READ + WRITE + EXECUTE;
                this->disk.write(current_dir_block, (uint8_t*)dir);
                uint8_t* tmp = new uint8_t[BLOCK_SIZE];
                debug = this->disk.read(FAT_BLOCK, tmp);
                int16_t* fat = (int16_t*)tmp; // ev delete tmp
                int tmp_block;
                while (index_block != FAT_EOF && index_block != FAT_FREE) {
                    tmp_block = fat[index_block];
                    fat[index_block] = FAT_FREE;
                    index_block = tmp_block;
                }
                fat[index_block] = FAT_FREE; // recursive fat free.
                this->disk.write(FAT_BLOCK, (uint8_t*)fat);
                delete[] dir, fat;
                return 0;
            }
            if (dir[i].type == TYPE_DIR) {
                std::cerr << "This file is a directory." << '\n';
                delete[] dir;
                return -1;
            }
        }
    }
    delete[] dir;
    if (found == -1) {
        std::cerr << "File not found." << '\n';
        return -1;
    }
    return 0;
}

// append <filepath1> <filepath2> appends the contents of file <filepath1> to
// the end of file <filepath2>. The file <filepath1> is unchanged.
int
FS::append(std::string filepath1, std::string filepath2)
{
    // gather info about dir entry
    std::string sourcename = get_filename(filepath1);
    std::string destname = get_filename(filepath2);
    if (sourcename.size() > 55 || destname.size() > 55) {
        std::cerr << "Filename of destination/source file is too long (>55 characters)." << '\n';
        return -1;
    }
    // see if source in CWD, if not -> bad write.
    dir_entry* dir = new dir_entry[BLOCK_SIZE/DIR_ENTRY_SIZE];
    int debug = this->disk.read(current_dir_block, (uint8_t*)dir);
    // här behövs först kollas om source finns.
    if (check_if_file_in_dir(dir, TYPE_FILE, sourcename.c_str()) == 0) {
        delete[] dir;
        std::cerr << "That sourcefile doesnt exist." << '\n';
        return -1;
    }
    if (check_if_file_in_dir(dir, TYPE_FILE, destname.c_str()) == 0) {
        delete[] dir;
        std::cerr << "That destinationfile doesnt exist." << '\n';
        return -1;
    }

    dir_entry* entry_handler = new dir_entry;
    std::string data = this->gather_info_old_dir_entry(dir, entry_handler, destname);
    data += this->gather_info_old_dir_entry(dir, entry_handler, sourcename);
    int data_size = entry_handler->size;
    if (privilege_check(entry_handler->access_rights, READ) != 0) {
        std::cerr << "You dont have the permission to read this file." << '\n';
        delete[] dir; delete entry_handler;
        return -2;
    }

    uint8_t* tmp = new uint8_t[BLOCK_SIZE]; // behlöver deleteas. memory leaks.
    debug = this->disk.read(FAT_BLOCK, tmp);
    int16_t* fat = (int16_t*)tmp;

    for (size_t i = 0; i < BLOCK_SIZE/DIR_ENTRY_SIZE; i++) {
        if (strcmp(dir[i].file_name, destname.c_str()) == 0) {
            if (privilege_check(dir[i].access_rights, WRITE) != 0) {
                std::cerr << "You dont have the permission to write to this file." << '\n';
                delete[] dir; delete entry_handler;
                return -2;
            }
            if (dir[i].type == TYPE_FILE) {
                dir[i].size += data_size;
                memcpy(entry_handler, &dir[i], sizeof(dir_entry));
                int index_block = entry_handler->first_blk;
                int tmp_block;
                while (index_block != FAT_EOF && index_block != FAT_FREE) {
                    tmp_block = fat[index_block];
                    fat[index_block] = FAT_FREE;
                    index_block = tmp_block;
                }
                fat[entry_handler->first_blk] = FAT_FREE;
                break;
            } // EV else här ist.
            if (dir[i].type == TYPE_DIR) {
                std::cerr << "Destination file is a directory." << '\n';
                delete[] dir; delete entry_handler;
                return -1;
            }
        }
    }

    // save entry on disk
    // std::cout << "DATA: " << data << '\n';
    if (FS::save_entry_on_disk(data, fat, entry_handler) != 0) {
        delete[] fat; delete entry_handler; delete[] dir;
        return -1;
    }
    // fat[index_block] = entry_handler->first_blk;
    // this->disk.write(FAT_BLOCK, (uint8_t*)fat);
    delete[] fat;

    //add to CWD
    this->disk.write(current_dir_block, (uint8_t*)dir);
    delete entry_handler; delete[] dir;
    return 0;
}

// mkdir <dirpath> creates a new sub-directory with the name <dirpath>
// in the current directory
int
FS::mkdir(std::string dirpath)
{
    // get one free block nr
    // then write it to disk
    // update FAT
    std::string filename = get_filename(dirpath);
    if (filename.size() > 55) {
        std::cerr << "Filename is too long (>55 characters)." << '\n';
        return -1;
    }
    if (this->check_if_dir_full(current_dir_block) == -1) {
        std::cerr << "Directory is full." << '\n';
        return -1;
    }
    int debug;
    dir_entry* dir = new dir_entry[BLOCK_SIZE/DIR_ENTRY_SIZE];
    dir_entry* dest_dir = new dir_entry[BLOCK_SIZE/DIR_ENTRY_SIZE];
    debug = this->disk.read(current_dir_block, (uint8_t*)dir);

    // if absolute path -> here needs to attain correct filename and root block.
    uint8_t* tmp = new uint8_t[BLOCK_SIZE]; // behlöver deleteas. memory leaks.
    debug = this->disk.read(FAT_BLOCK, tmp);
    int16_t* fat = (int16_t*)tmp;
    int free_block;
    for (size_t i = 2; i < BLOCK_SIZE/2; i++) {
        if (fat[i] == FAT_FREE) {
            free_block = i;
            break;
        }
        if (i == (BLOCK_SIZE/2-1)) {
            std::cerr << "Disk is full." << '\n';
            delete[] fat, dir, dest_dir;
            return -1;
        }
    }

    // see if destname is a dir
    int dest_dir_bool = destination_dir_check(dir, dirpath, filename);
    int dir_block = current_dir_block;
    if (dest_dir_bool == 0) {
        // dest is directory, get block
        dirpath = dirpath.substr(0, dirpath.rfind("/")+1);
        std::string original_path = current_working_dir;
        std::vector<int> blocks = get_dir_blocks(dirpath);
        current_working_dir = original_path;
        for (size_t i = 0; i < blocks.size(); i++) {
            if (blocks[i] == -1) {
                std::cerr << "Directory was not found." << '\n';
                delete[] fat, dir, dest_dir;
                return -1;
            }
        }
        dir_block = blocks[blocks.size()-1];
        // see if file already in dest directory
        if (this->check_if_dir_full(dir_block) == -1) {
            std::cerr << "Directory is full." << '\n';
            delete[] fat, dir, dest_dir;
            return -1;
        }
        debug = this->disk.read(dir_block, (uint8_t*)dest_dir);
        if (check_if_file_in_dir(dest_dir, TYPE_FILE, filename.c_str()) != 0) {
            std::cerr << "There already exist a file with that name in that directory." << '\n';
            delete[] fat, dir, dest_dir;
            return -1;
        }
    } else {
        if (check_if_file_in_dir(dir, TYPE_FILE, filename.c_str()) != 0) {
            delete[] dir, dest_dir;
            std::cerr << "That file already exist." << '\n';
            return -1;
        }
    }

    dir_entry* new_dir = new dir_entry[BLOCK_SIZE/DIR_ENTRY_SIZE];
    std::string prev_dir_name = "..";
    strncpy(new_dir[0].file_name, prev_dir_name.c_str(), sizeof(prev_dir_name));
    new_dir[0].size = 2;
    if (dest_dir_bool == 0) {
        new_dir[0].first_blk = dir_block; // previous block
    } else {
        new_dir[0].first_blk = current_dir_block; // previous block
    }
    new_dir[0].type = TYPE_DIR;
    new_dir[0].access_rights = READ + WRITE + EXECUTE;
    for (size_t i = 1; i < BLOCK_SIZE/DIR_ENTRY_SIZE; i++) {
        new_dir[i].file_name[0] = ' ';
        new_dir[i].size = 0;
        new_dir[i].first_blk = free_block;
        new_dir[i].type = TYPE_DIR;
        new_dir[i].access_rights = READ + WRITE + EXECUTE;
    }

    dir_entry* new_file = new dir_entry;
    strncpy(new_file->file_name, filename.c_str(), sizeof(filename));
    new_file->size = BLOCK_SIZE/DIR_ENTRY_SIZE;
    new_file->first_blk = free_block;
    new_file->type = TYPE_DIR;
    new_file->access_rights =  READ + WRITE + EXECUTE;
    if (dest_dir_bool == 0) {
        // std::cout << "/* message */" << '\n';
        debug = FS::save_entry_in_dir(dir_block, dest_dir, new_file);
    } else {
        debug = FS::save_entry_in_dir(current_dir_block, dir, new_file);
    }
    debug = this->disk.write(free_block, (uint8_t*)new_dir);
    fat[free_block] = FAT_EOF;
    debug = this->disk.write(FAT_BLOCK, (uint8_t*)fat);

    delete[] fat, dir, new_dir, dest_dir;
    delete new_file;
    return 0;
}

std::vector<int>FS::get_dir_blocks(std::string dirpath) {
    std::string delimiter = "/";
    std::vector<int> dir_blocks;
    dir_blocks.clear();
    int found = -1;
    dir_entry* dir = new dir_entry[BLOCK_SIZE/DIR_ENTRY_SIZE];
    int debug = this->disk.read(current_dir_block, (uint8_t*)dir);
    if (dirpath.find(delimiter)==std::string::npos) {
        // if (no) absolute path or relative path
        for(int i = 0; i < BLOCK_SIZE/DIR_ENTRY_SIZE; i++) {
            if(dir[i].file_name == dirpath && dir[i].type == TYPE_DIR) {
                dir_blocks.push_back(dir[i].first_blk);
                found = 0;
            }
        }
        if (found == -1) {
            delete[] dir;
            dir_blocks.push_back(-1);
            return dir_blocks;
        } else {
            if (current_working_dir == "/") {
                current_working_dir += dirpath;
            } else {
                if (dirpath == "..") {
                    current_working_dir = current_working_dir.substr(0, current_working_dir.find_last_of('/'));
                    if (current_working_dir.empty()) {
                        current_working_dir = "/";
                    }
                } else {
                    current_working_dir += "/" + dirpath;
                }
            }
            return dir_blocks;
        }
    }
    int tmp_dir_block = current_dir_block;
    if (dirpath.rfind(delimiter, 0) == 0) {
        // absolute path
        dir_blocks.push_back(ROOT_BLOCK);
        current_working_dir = "/";
        tmp_dir_block = ROOT_BLOCK;
        if (dirpath.size() == 1) {
            delete[] dir;
            return dir_blocks;
        }
        dirpath = dirpath.substr(1, dirpath.size());
    }
    // split dirpath by delimiter and parse through
    char last_char = dirpath[dirpath.size()-1];
    if (last_char != '/') {
        dirpath += delimiter;
    }
    int done = -1;
    std::string token = "";
    int index = 0;
    // int tmp_dir_block = current_dir_block;
    while (done == -1) {
        // get token and shorten dirpath
        token = dirpath.substr(0, dirpath.find(delimiter));
        if (token.empty() == true || dirpath.empty() == true) {
            done = 0;
            continue;
        } else {
            dirpath = dirpath.substr(dirpath.find(delimiter)+1, dirpath.size());
        }

        // gather new dir block
        dir = new dir_entry[BLOCK_SIZE/DIR_ENTRY_SIZE];
        debug = this->disk.read(tmp_dir_block, (uint8_t*)dir);
        found = -1;
        for(int i = 0; i < BLOCK_SIZE/DIR_ENTRY_SIZE; i++) {
            if(dir[i].file_name == token) {
                dir_blocks.push_back(dir[i].first_blk);
                tmp_dir_block = dir[i].first_blk;
                found = 0;
            }
        }
        delete[] dir;

        // see if dir was found or not
        if (found == -1) {
            dir_blocks.push_back(-1);
            return dir_blocks;
        }

        // update current_working_dir
        if (current_working_dir == "/") {
            current_working_dir += token;
        } else {
            if (token == "..") {
                current_working_dir = current_working_dir.substr(0, current_working_dir.find_last_of('/'));
                if (current_working_dir.empty()) {
                    current_working_dir = "/";
                }
            } else {
                current_working_dir += "/" + token;
            }
        }
    }
    // current_dir_block = 0;
    // current_working_dir = dirpath;
    return dir_blocks;
}

// cd <dirpath> changes the current (working) directory to the directory named <dirpath>
int
FS::cd(std::string dirpath)
{
    std::string original_path = current_working_dir;
    std::vector<int> blocks = get_dir_blocks(dirpath);
    for (size_t i = 0; i < blocks.size(); i++) {
        if (blocks[i] == -1) {
            std::cerr << "Directory was not found." << '\n';
            current_working_dir = original_path;
            return -1;
        }
    }
    current_dir_block = blocks[blocks.size()-1];
    return 0;
}

// pwd prints the full path, i.e., from the root directory, to the current
// directory, including the currect directory name
int
FS::pwd()
{
    std::cout << this->current_working_dir << '\n';
    return 0;
}

int FS::privilege_check(uint8_t access_rights, uint8_t required_privilege) {
    if ((access_rights & required_privilege) == required_privilege) {
        return 0;
    }
	return -1;
}

// chmod <accessrights> <filepath> changes the access rights for the
// file <filepath> to <accessrights>.
int
FS::chmod(std::string accessrights, std::string filepath)
{
    std::string filename = get_filename(filepath);
    if (filename.size() > 55) {
        std::cerr << "Filename of file is too long (>55 characters)." << '\n';
        return -1;
    }
    for (size_t i = 0; i < accessrights.size(); i++) {
        if (accessrights[i] < '0' || accessrights[i] > '9' ) {
            std::cerr << "The give accessrights aren't numerical." << '\n';
            return -1;
        }
    }
    if (std::stoi(accessrights) > 8 || std::stoi(accessrights) < 0) {
        std::cerr << "Accessrights not implementable." << '\n';
        return -1;
    }
    //check if it exists in dir
    dir_entry* dir = new dir_entry[BLOCK_SIZE/DIR_ENTRY_SIZE];
    int debug = this->disk.read(current_dir_block, (uint8_t*)dir);
    // här behövs först kollas om source finns.
    if (check_if_file_in_dir(dir, TYPE_FILE, filename.c_str()) == 0) {
        delete[] dir;
        std::cerr << "That file doesnt exist." << '\n';
        return -1;
    }

    // see if dest in CWD, if not -> ok write.
    for (size_t i = 0; i < BLOCK_SIZE/DIR_ENTRY_SIZE; i++) {
        // std::cout << current_working_dir[i].file_name << "\t" << new_file->file_name << '\n';
        if (strcmp(dir[i].file_name, filename.c_str()) == 0) {
            // doesnt matter dir/file
            dir[i].access_rights = std::stoi(accessrights);
            this->disk.write(current_dir_block, (uint8_t*)dir);
            delete[] dir;
            return 0;
        }
    }
    return 0;
}
