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


//     d::string FS::gather_info_new_dir_entry(int file_type, dir_entry* new_file, std::string filename){
// d::string FS::gather_info_new_dir_entry(int file_type, dir_entry* new_file, std::string filename){
// d::string FS::gather_info_new_dir_entry(int file_type, dir_entry* new_file, std::string filename){
// d::string FS::gather_info_new_dir_entry(int file_type, dir_entry* new_file, std::string filename){
// d::string FS::gather_info_new_dir_entry(int file_type, dir_entry* new_file, std::string filename){
// d::string FS::gather_info_new_dir_entry(int file_type, dir_entry* new_file, std::string filename){
// d::string FS::gather_info_new_dir_entry(int file_type, dir_entry* new_file, std::string filename){
// d::string FS::gather_info_new_dir_entry(int file_type, dir_entry* new_file, std::string filename){
// d::string FS::gather_info_new_dir_entry(int file_type, dir_entry* new_file, std::string filename){
// d::string FS::gather_info_new_dir_entry(int file_type, dir_entry* new_file



// g++ -std=c++11 -o filesystem main.o shell.o disk.o fs.o -fsanitize=address


FS::FS()
{
    std::cout << "FS::FS()... Creating file system\n";
}

FS::~FS()
{

}

// formats the disk, i.e., creates an empty file system
int
FS::format()
{
    // block nr 0 is root dir
    dir_entry* root_dir = new dir_entry[BLOCK_SIZE/DIR_ENTRY_SIZE];
    // dir_entry tmp;
    for (size_t i = 0; i < BLOCK_SIZE/DIR_ENTRY_SIZE; i++) {
        root_dir[i].file_name[0] = ' ';
        root_dir[i].size = 0;
        root_dir[i].first_blk = ROOT_BLOCK;
        root_dir[i].type = TYPE_DIR;
        root_dir[i].access_rights = READ + WRITE + EXECUTE;
    }
    this->disk.write(ROOT_BLOCK, (uint8_t*)root_dir);
    delete[] root_dir;
    // block nr 1 is File Allocation Table
    int16_t* fat = new int16_t[BLOCK_SIZE/2]; // Whole FAT in one block, we can address 4096 / 2 = 2048 disk blocks in a partition
    //disk write is not setup for BLOCK_SIZE/2
    fat[0] = FAT_EOF; fat[1] = FAT_EOF; // first two blocks are occupied
    for (size_t i = 2; i < BLOCK_SIZE/2; i++) {
        fat[i] = FAT_FREE;
        std::cout << fat[i] << ' ';
    }
    std::cout << '\n';
    this->disk.write(FAT_BLOCK, (uint8_t*)fat);
    this->current_working_dir = "/";
    delete[] fat;
    return 0;
}

std::string FS::gather_info_new_dir_entry(int file_type, dir_entry* new_file, std::string filename){
    // gather info about dir entry
    strncpy(new_file->file_name, filename.c_str(), sizeof(filename));
    new_file->access_rights = READ + WRITE + EXECUTE; // Access rights of a file or directory should be ’rw-’ or ’rwx’ when the file or directory is created. (7)
    new_file->type = file_type;
    std::string line;
    std::string data;
    std::getline(std::cin, line);
    while(line != "") {
        data += line + "\n";
        std::getline(std::cin, line); // cin max is 4096 kek
    }
    std::cout << "sizeo: " <<  data.size() << '\n';
    std::cout << "max sizeo: " << data.max_size() << '\n';
    new_file->size = data.size();
    return data;
}

int FS::check_if_file_in_CWD(dir_entry* current_working_dir, dir_entry* new_file){
    for (size_t i = 0; i < 64; i++) {
        // std::cout << current_working_dir[i].file_name << "\t" << new_file->file_name << '\n';
        if (strcmp(current_working_dir[i].file_name, new_file->file_name) == 0) {
            if (new_file->type == TYPE_FILE) {
                std::cerr << "There already exist a file with that name!" << '\n';
            }
            if (new_file->type == TYPE_DIR) {
                std::cerr << "There already exist a directory with that name!" << '\n';
            }
            delete[] current_working_dir; delete new_file;
            return -1;
        }
    }
    return 0;
}

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
            std::cerr << "DISK is FULL!" << '\n';
            return -1;
        }
    }
    for (size_t i = 0; i < free_blocks.size(); i++) {
        std::cout << "freeblocks: " << free_blocks[i] << '\n';
    }
    new_file->first_blk = free_blocks[0];
    // int temp_block = free_blocks[0];
    // int current_block;
    // do {
    //     current_block = temp_block;
    //     temp_block = free_blocks[count];
    //     fat[current_block] = temp_block;
    //     std::cout << "WRITING: in block " << current_block << ":\n"<< data.substr(count*BLOCK_SIZE, (count+1)*BLOCK_SIZE) << '\n';
    //     this->disk.write(current_block, (uint8_t*)(char*)data.substr(count*BLOCK_SIZE, (count+1)*BLOCK_SIZE).c_str());
    //     count++;
    // } while(count < free_blocks.size());
    // fat[temp_block] = FAT_EOF;
    if (free_blocks.size() == 1) {
        fat[free_blocks[0]] = FAT_EOF;
        std::cout << "WRITING: in block " << free_blocks[0] << ":\n"<< data << '\n';
        this->disk.write(free_blocks[0], (uint8_t*)(char*)data.c_str());
    } else {
        int i = 0;
        while (i <= free_blocks.size()) {
            if (i+1 < free_blocks.size()) {
                fat[free_blocks[i]] = free_blocks[i+1];
                std::cout << "WRITING: in block " << free_blocks[i] << ":\n"<< data.substr(i*BLOCK_SIZE, (i+1)*BLOCK_SIZE) << '\n';
                this->disk.write(free_blocks[i], (uint8_t*)(char*)data.substr(i*BLOCK_SIZE, (i+1)*BLOCK_SIZE).c_str());
                i++;
            } else {
                std::cout << "WRITING: in block " << free_blocks[i] << ":\n"<< data.substr(i*BLOCK_SIZE, (i+1)*BLOCK_SIZE) << '\n';
                this->disk.write(free_blocks[i], (uint8_t*)(char*)data.substr(i*BLOCK_SIZE, (i+1)*BLOCK_SIZE).c_str());
                fat[free_blocks[i]] = FAT_EOF;
                break;
            }
        }
    }
    this->disk.write(FAT_BLOCK, (uint8_t*)fat);
    // for (size_t i = 0; i < free_blocks.size(); i++) {
        //     current_block = temp_block;
        //     temp_block = free_blocks[i];
        //     fat[current_block] = temp_block;
        //     std::cout << "WRITING: in block " << current_block << ":\n"<< data.substr(i*BLOCK_SIZE, (i+1)*BLOCK_SIZE) << '\n';
        //     this->disk.write(current_block, (uint8_t*)(char*)data.substr(i*BLOCK_SIZE, (i+1)*BLOCK_SIZE).c_str());
        // }
    // std::cout << "FAT values: " << '\n';
    // for (size_t i = 0; i < BLOCK_SIZE/2; i++) {
    //     std::cout << fat[i] << ' ';
    // }
    return 0;
}

int FS::save_entry_on_CWD(int current_dir_block, dir_entry* current_working_dir, dir_entry* new_file){
    for (size_t i = 0; i < 64; i++) {
        if (current_working_dir[i].size == 0) {
            memcpy(&current_working_dir[i], new_file, sizeof(dir_entry));
            break;
        }
    }
    for (size_t i = 0; i < BLOCK_SIZE/DIR_ENTRY_SIZE; i++) {
        std::cout << current_working_dir[i].first_blk << ' ';
    } std::cout << '\n';
    this->disk.write(current_dir_block, (uint8_t*)current_working_dir);
    return 0;
}

// create <filepath> creates a new file on the disk, the data content is
// written on the following rows (ended with an empty row)
int
FS::create(std::string filepath)
{
    // gather info about dir entry
    dir_entry* new_file = new dir_entry;
    std::string filename;
    if (filepath.at(0) == '/') {
        filename = filepath.substr(filepath.find_last_of("/") + 1);
    } else {
        filename = filepath;
    }
    if (filename.size() > 55) {
        std::cerr << "Filename is too long (>55 characters)" << '\n';
        delete new_file;
        return -1;
    }
    // filename = this->current_working_dir + filename;
    std::string data = this->gather_info_new_dir_entry(TYPE_FILE, new_file, filename);

    // see if in CWD, if not -> ok write.
    dir_entry* root_dir = new dir_entry[BLOCK_SIZE/DIR_ENTRY_SIZE];
    int debug = this->disk.read(ROOT_BLOCK, (uint8_t*)root_dir);
    // check if file already exists
    if (check_if_file_in_CWD(root_dir, new_file) != 0) {
        return -1;
    }

    // save entry on disk
    // int16_t* fat = new int16_t[BLOCK_SIZE/2];
    uint8_t* tmp = new uint8_t[4096];
    debug = this->disk.read(FAT_BLOCK, tmp);
    int16_t* fat = (int16_t*)tmp;
    if (FS::save_entry_on_disk(data, fat, new_file) != 0) {
        delete[] fat;
        return -1;
    }
    delete[] fat;

    //add to CWD
    FS::save_entry_on_CWD(ROOT_BLOCK, root_dir, new_file);
    delete new_file; delete[] root_dir;
    return 0;
}

// cat <filepath> reads the content of a file and prints it on the screen
int
FS::cat(std::string filepath)
{
    dir_entry* root_dir = new dir_entry[BLOCK_SIZE/DIR_ENTRY_SIZE];
    int debug = this->disk.read(ROOT_BLOCK, (uint8_t*)root_dir);
    // gather input info
    std::string filename;
    if (filepath.at(0) == '/') {
        filename = filepath.substr(filepath.find_last_of("/") + 1);
    } else {
        filename = filepath;
    }
    if (filename.size() > 55) {
        std::cerr << "Filename is too long and doesn't exist (>55 characters)." << '\n';
        delete[] root_dir;
        return -1;
    }

    // see if file can be found and where its first block is
    int index_block;
    int file_found = -1;
    int file_size;
    for (size_t i = 0; i < 64; i++) {
        if (root_dir[i].file_name == filename) {
            index_block = root_dir[i].first_blk;
            file_size = root_dir[i].size;
            file_found = 0;
            break;
        }
    }
    if (file_found != 0) {
        delete[] root_dir;
        std::cout << "file not found." << '\n';
        return -1;
    }
    delete[] root_dir;

    // Read each block via FAT, EV har inte testat med flera blocks överskridandefiler
    uint8_t* tmp = new uint8_t[BLOCK_SIZE];
    debug = this->disk.read(FAT_BLOCK, tmp);
    int16_t* fat = (int16_t*)tmp;
    uint8_t* block = new uint8_t[BLOCK_SIZE];
    std::string file_data;
    int count = 0;
    do {
        debug = this->disk.read(index_block, block);
        std::cout << "index block " << index_block << ":\n";
        std::string file_data = (char*)block;
        std::cout << file_data << '\n';
        // std::cout << file_data.substr(0, std::min(file_size - BLOCK_SIZE*count, BLOCK_SIZE));
        // std::cout << file_data.substr(count*BLOCK_SIZE, (count+1)*BLOCK_SIZE) << '\n';
        index_block = fat[index_block];
        count++;
    } while (index_block != FAT_EOF && index_block != FAT_FREE);
    delete[] fat, block;
    return 0;
}

// ls lists the content in the currect directory (files and sub-directories)
int
FS::ls()
{
    std::cout << "FS::ls()\n";
    dir_entry* root_dir = new dir_entry[BLOCK_SIZE/DIR_ENTRY_SIZE];
    int debug = this->disk.read(ROOT_BLOCK, (uint8_t*)root_dir);
    std::cout << "name\t\t\t\tsize\t\t\tfirst_blk" << '\n';
    for (size_t i = 0; i < 64; i++) {
        if (root_dir[i].size != 0) {
            std::cout << root_dir[i].file_name << "\t\t\t" << root_dir[i].size << "\t\t\t" << root_dir[i].first_blk << '\n';
        }
    }
    return 0;
}

// cp <sourcepath> <destpath> makes an exact copy of the file
// <sourcepath> to a new file <destpath>
int
FS::cp(std::string sourcepath, std::string destpath)
{
    std::cout << "FS::cp(" << sourcepath << "," << destpath << ")\n";
    return 0;
}

// mv <sourcepath> <destpath> renames the file <sourcepath> to the name <destpath>,
// or moves the file <sourcepath> to the directory <destpath> (if dest is a directory)
int
FS::mv(std::string sourcepath, std::string destpath)
{
    std::cout << "FS::mv(" << sourcepath << "," << destpath << ")\n";
    return 0;
}

// rm <filepath> removes / deletes the file <filepath>
int
FS::rm(std::string filepath)
{
    std::cout << "FS::rm(" << filepath << ")\n";
    return 0;
}

// append <filepath1> <filepath2> appends the contents of file <filepath1> to
// the end of file <filepath2>. The file <filepath1> is unchanged.
int
FS::append(std::string filepath1, std::string filepath2)
{
    std::cout << "FS::append(" << filepath1 << "," << filepath2 << ")\n";
    return 0;
}

// mkdir <dirpath> creates a new sub-directory with the name <dirpath>
// in the current directory
int
FS::mkdir(std::string dirpath)
{
    std::cout << "FS::mkdir(" << dirpath << ")\n";
    return 0;
}

// cd <dirpath> changes the current (working) directory to the directory named <dirpath>
int
FS::cd(std::string dirpath)
{
    // if current_working_dir == "/" -> ".." <=> "."
    // if current_working_dir != "/" -> ".." != "."
    std::cout << "FS::cd(" << dirpath << ")\n";
    return 0;
}

// pwd prints the full path, i.e., from the root directory, to the current
// directory, including the currect directory name
int
FS::pwd()
{
    std::cout << "FS::pwd()\n";
    return 0;
}

// chmod <accessrights> <filepath> changes the access rights for the
// file <filepath> to <accessrights>.
int
FS::chmod(std::string accessrights, std::string filepath)
{
    std::cout << "FS::chmod(" << accessrights << "," << filepath << ")\n";
    return 0;
}
