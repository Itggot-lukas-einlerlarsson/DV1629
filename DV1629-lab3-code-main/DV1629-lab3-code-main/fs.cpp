#include <iostream>
#include "fs.h"

bool FS::privilege_check(uint8_t access_rights, uint8_t required_privilege) {
	return (access_rights & required_privilege) == required_privilege;
}

int FS::get_current_dir_blk(std::string dir_path) {
    // Om startar med / så ska den vara absolut
    // Om den inte startar med / så ska man ta pathen som går från current_path + dir_path
    std::string wanted_path;
    int current_dir_blk = 0;

    if(dir_path.find_first_of('/') == 0) {
        wanted_path = dir_path;
    } else if(dir_path == "..") {
        if(current_dir.length() > 1){
            wanted_path = current_dir.substr(0, current_dir.length()-2);
            wanted_path = wanted_path.substr(0, wanted_path.find_last_of("/"));
        }
    }
     else if(dir_path.find("../") != std::string::npos || dir_path.find("..") != std::string::npos) {
        wanted_path = current_dir + '/' +dir_path;
        // std::cout << "Original wanted path: " << wanted_path << std::endl;
        int i = 2;
        while(wanted_path.find("../") != std::string::npos || wanted_path.find("..") != std::string::npos) {
            wanted_path = wanted_path.substr(0, wanted_path.find("../")-1);
            wanted_path = wanted_path.substr(0, wanted_path.find_last_of('/'));
            if(wanted_path.empty()) break;
            wanted_path += dir_path.substr(dir_path.find("../")+2, dir_path.size());
        }
    } else {
        wanted_path = current_dir + '/' + dir_path;
    }

    // std::cout << "Result Wanted path: " << wanted_path << std::endl;

    std::stringstream ss(wanted_path);
    std::string path;
    while (!ss.eof()) {
        std::getline(ss, path, '/');
        // std::cout << "" << path << std::endl;

        uint8_t* blk = new uint8_t[4096];
        disk.read(current_dir_blk, blk);
        dir_entry* dir_entries = (dir_entry*)blk;
        for(int i = 0; i < 64; i++) {
            if(dir_entries[i].file_name == path) {
                current_dir_blk = dir_entries[i].first_blk;
            }
        }
        delete[] blk;
    }

    return current_dir_blk;
}


std::string FS::privilege_to_string(uint8_t privilege) {
    switch(privilege) {
        case 4:
            return "r--";
        break;
        case 2:
            return "-w-";
        break;
        case 1:
            return "--x";
        break;
        case 6:
            return "rw-";
        break;
        case 7:
            return "rwx";
        break;
        case 5:
            return "r-x";
        break;
        case 3:
            return "-wx";
        break;
    }
    return "---";
}

FS::FS()
{
    std::cout << "FS::FS()... Creating file system\n";
    this->current_dir = "/";
}

FS::~FS()
{

}

// formats the disk, i.e., creates an empty file system
int
FS::format()
{
    std::cout << "FS::format()\n";
    current_dir = "/";
    // uint8_t* blk = new uint8_t[4096];
    dir_entry* blk = new dir_entry[64];
    dir_entry dir;
    dir.size = 0;
    dir.first_blk = 0;
    dir.type = 2;
    for(int i = 0; i < 64; i++) {
        memcpy(&blk[i], &dir, sizeof(dir_entry));
    }
    this->disk.write(0, (uint8_t*)blk);

    uint8_t* fat = new uint8_t[4096];
    this->disk.read(1, fat);
    int16_t* fat_entries = (int16_t*)fat;

    for(int i = 0; i < sizeof(fat_entries)/sizeof(fat_entries[0]); i++) {
        if(i == 0 || i == 1) {
            fat_entries[i] = FAT_EOF;
        } else {
            fat_entries[i] = FAT_FREE;
        }
    }
    this->disk.write(1, (uint8_t*)fat_entries);

    delete[] blk;
    return 0;
}

// create <filepath> creates a new file on the disk, the data content is
// written on the following rows (ended with an empty row)
int
FS::create(std::string filepath)
{
    // std::cout << "FS::create(" << filepath << ")\n";
    if(filepath.size() > 55) {
        std::cout << "FS::create(" << filepath << ") -> filepath too long (>55)" << std::endl;
        return 1;
    }


    dir_entry* file = new dir_entry;

    // Init file name
    std::string filename = filepath.substr(filepath.find_last_of("/") + 1);
    std::strcpy(file->file_name, filename.c_str());

    // Init file size
    std::string data = " ", data_size;
    std::getline(std::cin, data);
    while(data != "") {
        data_size += data + "\n";
        std::getline(std::cin, data);
    }
    file->size = data_size.size();
    float nr_needed_blksf = (float)data_size.size() / 4096;
    int nr_needed_blks = ceil(nr_needed_blksf);
    
    // Init file type
    file->type = 0; // (1) = directory, (0) = file

    // Init file access rights
    file->access_rights = READ + WRITE; // beror på access rights

    // Init (Save) data
    uint8_t* blk = new uint8_t[4096];
    this->disk.read(1, blk);
    int16_t* fat_entries = (int16_t*)blk;
    int block_no;

    std::vector<int> blk_needed;
    for(int i = 2; i < 2048; i++) {
        if(fat_entries[i] == FAT_FREE) {
            if(nr_needed_blks > 1) {
                blk_needed.push_back(i);
                nr_needed_blks--;
            } else {
                blk_needed.push_back(i);
                break;
            }
        }
    }
    file->first_blk = blk_needed[0];

    int current_blk;
    int tmp_blk = blk_needed[0];
    if (blk_needed.size() == 1) {
        fat_entries[tmp_blk] = FAT_EOF;
        this->disk.write(tmp_blk, (uint8_t*)(char*)data_size.c_str());
    } else {
        for(int i = 0; i < blk_needed.size(); i++) {
            current_blk = tmp_blk;
            tmp_blk = blk_needed[i];
            fat_entries[current_blk] = tmp_blk;
            this->disk.write(tmp_blk, (uint8_t*)(char*)data_size.substr(i*4096, (i+1)*4096).c_str());
        }
        fat_entries[tmp_blk] = FAT_EOF;
    }
    this->disk.write(1, (uint8_t*)fat_entries);


    // Maybe add error handling here
    delete[] blk, fat_entries;

    // See + Write to root
    int current_dir_blk = get_current_dir_blk(current_dir);
    // std::cout << current_dir_blk << std::endl;
    blk = new uint8_t[4096];
    this->disk.read(current_dir_blk, blk);
    dir_entry* dir_entries = (dir_entry*)blk;


    // std::cout << current_dir_blk;

    for(int i = 0; i < 64; i++) {
        if(dir_entries[i].type == 2) {
            memcpy(&dir_entries[i], file, sizeof(dir_entry));
            this->disk.write(current_dir_blk, blk);
            delete[] blk, dir_entries;
            return 0;
        }
    }

    std::cout << "FS::create(" << filepath << ") -> disk entry is full (>64)" << std::endl;
    return 1;    
}

// cat <filepath> reads the content of a file and prints it on the screen
int
FS::cat(std::string filepath)
{
    // std::cout << "FS::cat(" << filepath << ")\n";
 
    std::string sourcepath_dir = filepath.substr(0, filepath.find_last_of('/')+1);
    std::string sourcepath_filename = filepath.substr(filepath.find_last_of('/')+1, filepath.size());
    int current_dir_blk = get_current_dir_blk(sourcepath_dir);


    uint8_t* blk = new uint8_t[4096];
    this->disk.read(current_dir_blk, blk);
    dir_entry* dir_entries = (dir_entry*)blk;

    uint8_t* blk_2 = new uint8_t[4096];
    this->disk.read(1, blk_2);
    int16_t* fat_entries = (int16_t*)blk_2;

    std::string data;
    for(int i = 0; i < 64; i++) {
        if(dir_entries[i].file_name == sourcepath_filename && dir_entries[i].type == 0) {
            if(!privilege_check(dir_entries[i].access_rights, READ)) {
                std::cout << "----- You don't have the required rights to read this file -----" << std::endl;
                return 1;
            }
            int nr_needed_blks = ceil((float)dir_entries[i].size / 4096), current_blk = dir_entries[i].first_blk; 

            for(int j = 0; j < nr_needed_blks; j++) {
                uint8_t* file = new uint8_t[4096];
                this->disk.read(current_blk, file);
                std::string output = (char*)file;
                std::cout << output.substr(0, std::min((int)dir_entries[i].size - 4096*j, 4096));
                current_blk = fat_entries[current_blk];
            }
            std::cout << std::endl;
            return 0;
        }
    }

    std::cout << "FS::Cat(" << filepath << ") -> Could not find filepath" << std::endl;
    return 1;
}

// ls lists the content in the currect directory (files and sub-directories)
int
FS::ls()
{
    // find blk associated with the current directory, start looking from root forward
    int current_dir_blk = get_current_dir_blk(current_dir);

    std::cout << "name" << "\t type \taccessrights \t" << "size" << std::endl;
    uint8_t* blk = new uint8_t[4096];
    this->disk.read(current_dir_blk, blk);
    dir_entry* dir_entries = (dir_entry*)blk;

    if(current_dir_blk != 0) {
        std::cout << ".." << "\t dir\t-\t\t" << "-" << std::endl;
    }

    for(int i = 0; i < 64; i++) {
        if(dir_entries[i].type == 1){
            std::cout << dir_entries[i].file_name << "\t dir\t" << privilege_to_string(dir_entries[i].access_rights) << "\t\t" << "-" << std::endl;
        }
    }
    for(int i = 0; i < 64; i++) {
        if(dir_entries[i].size != 0) {
            std::cout << dir_entries[i].file_name << "\t file\t" << privilege_to_string(dir_entries[i].access_rights) << "\t\t" << std::to_string(dir_entries[i].size) << std::endl;
        }
    }

    delete[] blk, dir_entries;
    return 0;
}

// cp <sourcepath> <destpath> makes an exact copy of the file
// <sourcepath> to a new file <destpath>
int
FS::cp(std::string sourcepath, std::string destpath)
{
    // std::cout << "FS::cp(" << sourcepath << "," << destpath << ")\n";

    std::string sourcepath_dir = sourcepath.substr(0, sourcepath.find_last_of('/')+1);
    std::string sourcepath_filename = sourcepath.substr(sourcepath.find_last_of('/')+1, sourcepath.size());
    int source_dir_blk = get_current_dir_blk(sourcepath_dir);

    std::string destpath_dir = destpath.substr(0, destpath.find_last_of('/')+1);
    std::string destpath_filename = destpath.substr(destpath.find_last_of('/')+1, destpath.size());
    int dest_dir_blk = get_current_dir_blk(destpath_dir);
    if(destpath_filename == "..") {
        dest_dir_blk = get_current_dir_blk(destpath_filename);
        destpath_filename = sourcepath_filename;
    }

    uint8_t* blk = new uint8_t[4096];
    this->disk.read(source_dir_blk, blk);
    dir_entry* dir_entries = (dir_entry*)blk;
    dir_entry* source_dir;
    bool sourcepath_found = false;
    
    for(int i = 0; i < 64; i++) {
        if(dir_entries[i].file_name == sourcepath_filename) {
            sourcepath_found = true;
            source_dir = &dir_entries[i];
            std::strcpy(source_dir->file_name, destpath_filename.c_str());
            break;
        }
    }

    if(sourcepath_found == false) {
        std::cout << "FS::cp(" << sourcepath << ", " <<  destpath << ") -> cant cp a non-existing file" << std::endl;
        return 1;
    }

    blk = new uint8_t[4096];
    this->disk.read(dest_dir_blk, blk);
    dir_entries = (dir_entry*)blk;

    for(int i = 0; i < 64; i++) {
        if(dir_entries[i].file_name == destpath_filename && dir_entries[i].type == 0) {
            std::cout << "FS::cp(" << sourcepath << ", " <<  destpath << ") -> cant cp to an existing file" << std::endl;
            return 1;
        }
        if(dir_entries[i].file_name == destpath_filename && dir_entries[i].type == 1) {
            dest_dir_blk = get_current_dir_blk(destpath_dir + destpath_filename);
            std::strcpy(source_dir->file_name, sourcepath_filename.c_str());
            break;
        }
    }
 
    blk = new uint8_t[4096];
    this->disk.read(1, blk);
    int16_t* fat_entries = (int16_t*)blk;
    int nr_needed_blks = ceil((float)source_dir->size / 4096);

    std::vector<int> blk_needed;
    for(int i = 2; i < 2048; i++) {
        if(fat_entries[i] == FAT_FREE) {
            if(nr_needed_blks > 1) {
                blk_needed.push_back(i);
                nr_needed_blks--;
            } else {
                blk_needed.push_back(i);
                break;
            }
        }
    }

    int source_blk = source_dir->first_blk, cpy_blk = blk_needed[0];
    source_dir->first_blk = cpy_blk;
    uint8_t* content = new uint8_t[4096];
    this->disk.read(source_blk, content);
    this->disk.write(cpy_blk, content);

    for(int i = 1; i < blk_needed.size(); i++) {
        fat_entries[cpy_blk] = blk_needed[i];
        
        cpy_blk = fat_entries[cpy_blk];
        source_blk = fat_entries[source_blk];

        content = new uint8_t[4096];
        this->disk.read(source_blk, content);
        this->disk.write(blk_needed[0], content);
    }
    fat_entries[cpy_blk] = FAT_EOF;
    this->disk.write(1, (uint8_t*)fat_entries);

    // Maybe add error handling here
    delete[] blk, fat_entries;

    // See + Write to block
    blk = new uint8_t[4096];
    this->disk.read(dest_dir_blk, blk);
    dir_entries = (dir_entry*)blk;

    for(int i = 0; i < 64; i++) {
        if(dir_entries[i].type == 2) {
            memcpy(&dir_entries[i], source_dir, sizeof(dir_entry));
            this->disk.write(dest_dir_blk, blk);
            delete[] blk, dir_entries;
            return 0;
        }
    }

    return 0;
}

// mv <sourcepath> <destpath> renames the file <sourcepath> to the name <destpath>,
// or moves the file <sourcepath> to the directory <destpath> (if dest is a directory)
int
FS::mv(std::string sourcepath, std::string destpath)
{
    // std::cout << "FS::mv(" << sourcepath << "," << destpath << ")\n";
    std::string sourcepath_dir = sourcepath.substr(0, sourcepath.find_last_of('/')+1);
    std::string sourcepath_filename = sourcepath.substr(sourcepath.find_last_of('/')+1, sourcepath.size());
    // if(sourcepath_dir == sourcepath_filename) sourcepath_dir = "/";
    int source_dir_blk = get_current_dir_blk(sourcepath_dir);

    std::string destpath_dir = destpath.substr(0, destpath.find_last_of('/')+1);
    std::string destpath_filename = destpath.substr(destpath.find_last_of('/')+1, destpath.size());
    // if(destpath_dir == destpath_filename) destpath_dir = "/";
    int dest_dir_blk = get_current_dir_blk(destpath_dir);
    if(destpath_filename == "..") {
        dest_dir_blk = get_current_dir_blk(destpath_filename);
        destpath_filename = sourcepath_filename;
    }

    uint8_t* blk = new uint8_t[4096];
    this->disk.read(dest_dir_blk, blk);
    dir_entry* dir_entries = (dir_entry*)blk;

    for(int i = 0; i < 64; i++) {
        if(dir_entries[i].file_name == destpath_filename && dir_entries[i].type == 0) {
            std::cout << "FS::cp(" << sourcepath << ", " <<  destpath << ") -> cant cp to an existing file" << std::endl;
            return 1;
        }
        if(dir_entries[i].file_name == destpath_filename && dir_entries[i].type == 1) {
            dest_dir_blk = get_current_dir_blk(destpath_dir + destpath_filename);
            destpath_filename = sourcepath_filename;
            break;
        }
    }

    if(source_dir_blk == dest_dir_blk) {
        // std::cout << source_dir_blk;
        uint8_t* blk = new uint8_t[4096];
        this->disk.read(source_dir_blk, blk);
        dir_entry* dir_entries = (dir_entry*)blk;

        for(int i = 0; i < 64; i++) {
            if(dir_entries[i].file_name == destpath_filename) {
                std::cout << "FS::mv(" << sourcepath << ", " <<  destpath << ") -> Can't overwrite existing file" << std::endl;
                return 1;
            }
        }

        for(int i = 0; i < 64; i++) {
            if(dir_entries[i].file_name == sourcepath_filename) {
                // std::cout << "yo: " << sourcepath_filename << std::endl;
                // std::cout << std::to_string(block_no) << std::endl;
                std::strcpy(dir_entries[i].file_name, destpath_filename.c_str());
                this->disk.write(0, blk);
                delete[] blk, dir_entries;
                return 0;
            }
        }
    } else {
        uint8_t* blk = new uint8_t[4096];
        this->disk.read(source_dir_blk, blk);
        dir_entry* dir_entries = (dir_entry*)blk;
        dir_entry* source_dir;

        for(int i = 0; i < 64; i++) {
            if(dir_entries[i].file_name == sourcepath_filename) {
                source_dir = &dir_entries[i];
            }
        }

        blk = new uint8_t[4096];
        this->disk.read(1, blk);
        int16_t* fat_entries = (int16_t*)blk;
        int nr_needed_blks = ceil((float)source_dir->size / 4096);

        std::vector<int> blk_needed;
        for(int i = 2; i < 2048; i++) {
            if(fat_entries[i] == FAT_FREE) {
                if(nr_needed_blks > 1) {
                    blk_needed.push_back(i);
                    nr_needed_blks--;
                } else {
                    blk_needed.push_back(i);
                    break;
                }
            }
        }

        int source_blk = source_dir->first_blk, cpy_blk = blk_needed[0];
        source_dir->first_blk = cpy_blk;
        uint8_t* content = new uint8_t[4096];
        this->disk.read(source_blk, content);
        this->disk.write(cpy_blk, content);

        for(int i = 1; i < blk_needed.size(); i++) {
            fat_entries[cpy_blk] = blk_needed[i];

            cpy_blk = fat_entries[cpy_blk];
            source_blk = fat_entries[source_blk];
            fat_entries[source_blk] = FAT_FREE;

            content = new uint8_t[4096];
            this->disk.read(source_blk, content);
            this->disk.write(blk_needed[0], content);
        }
        fat_entries[cpy_blk] = FAT_EOF;
        this->disk.write(1, (uint8_t*)fat_entries);

        blk = new uint8_t[4096];
        this->disk.read(dest_dir_blk, blk);
        dir_entries = (dir_entry*)blk;


        for(int i = 0; i < 64; i++) {
            if(dir_entries[i].type == 2) {
                std::strcpy(source_dir->file_name, destpath_filename.c_str());
                source_dir->first_blk = cpy_blk;
                memcpy(&dir_entries[i], source_dir, sizeof(dir_entry));
                this->disk.write(dest_dir_blk, blk);
                break;
            }
        }

        blk = new uint8_t[4096];
        this->disk.read(source_dir_blk, blk);
        dir_entries = (dir_entry*)blk;
        dir_entry* empty_dir = new dir_entry; empty_dir->size = 0;

        for(int i = 0; i < 64; i++) {
            if(dir_entries[i].file_name == sourcepath_filename) {
                memcpy(&dir_entries[i], empty_dir, sizeof(dir_entry));
                this->disk.write(source_dir_blk, (uint8_t*)dir_entries);
                delete[] blk, dir_entries, fat_entries;
                return 0;
            }
        }
    }


    std::cout << "FS::mv(" << sourcepath << ", " <<  destpath << ") -> Couldnt find sourcepath" << std::endl;
    return 1;
}

// rm <filepath> removes / deletes the file <filepath>
int
FS::rm(std::string filepath)
{
    // std::cout << "FS::rm(" << filepath << ")\n";

    dir_entry* empty_dir = new dir_entry; empty_dir->size = 0;
    bool filepath_found = false;

    int current_dir_blk = get_current_dir_blk(current_dir);
    uint8_t* blk = new uint8_t[4096];
    this->disk.read(current_dir_blk, blk);
    dir_entry* dir_entries = (dir_entry*)blk;
    int nr_needed_blks, frst_blk;

    for(int i = 0; i < 64; i++) {
        if(dir_entries[i].file_name == filepath) {
            filepath_found = true;
            nr_needed_blks = ceil((float)dir_entries[i].size / 4096);
            frst_blk = dir_entries[i].first_blk;
            memcpy(&dir_entries[i], empty_dir, sizeof(dir_entry));
            this->disk.write(current_dir_blk, blk);
            delete[] blk, dir_entries;
        }
    }
    if(filepath_found == false) {
        std::cout << "FS::rm(" << filepath << ") -> cant remove non-existing file" << std::endl;
        return 1;
    }

    blk = new uint8_t[4096];
    this->disk.read(1, blk);
    uint16_t* fat_entries = (uint16_t*)blk;

    int current_blk;
    int tmp_blk = frst_blk;
    for(int i = 0; i < nr_needed_blks; i++) {
        current_blk = tmp_blk;
        tmp_blk = fat_entries[current_blk];
        fat_entries[current_blk] = FAT_FREE;
    }
    this->disk.write(1, (uint8_t*)fat_entries);

    delete[] blk, fat_entries;

    return 0;
}

// append <filepath1> <filepath2> appends the contents of file <filepath1> to
// the end of file <filepath2>. The file <filepath1> is unchanged.
int
FS::append(std::string filepath1, std::string filepath2)
{

    // std::cout << "FS::append(" << filepath1 << "," << filepath2 << ")\n";
    int current_dir_blk = get_current_dir_blk(current_dir);
    uint8_t* blk = new uint8_t[4096];
    this->disk.read(current_dir_blk, blk);
    dir_entry* dir_entries = (dir_entry*)blk;
    bool filepath1_found = false, filepath2_found = false;

    dir_entry *file2_copy, *file_copy;

    for(int i = 0; i < 64; i++) {
        if(dir_entries[i].file_name == filepath1) {
            filepath1_found = true;
            file_copy = &dir_entries[i];
        }
        if(dir_entries[i].file_name == filepath2) {
            filepath2_found = true;
            file2_copy = &dir_entries[i];
        }
    }

    if(filepath1_found == false || filepath2_found == false) {
        std::cout << "FS::append(" << filepath1 << "," << filepath2 << ") -> Could not find either of filepaths\n";
        return 1;
    }


    uint8_t* fat_ent = new uint8_t[4096];
    this->disk.read(1, fat_ent);
    int16_t* fat_entries = (int16_t*)fat_ent;
    std::string data;
    int nr_needed_blks = ceil((float)file_copy->size / 4096), current_blk = file_copy->first_blk;


    
    if(!privilege_check(file_copy->access_rights, READ) || !privilege_check(file2_copy->access_rights, WRITE)) {
        std::cout << "----- You don't have the required rights to read this file -----" << std::endl;
        return 1;
    }
// Get data from filepath1
    uint8_t* file = new uint8_t[4096];
    this->disk.read(current_blk, file);
    // this->disk.write(blk_needed[i], file);
    data += (char*)file;

    for(int i = 1; i < nr_needed_blks; i++) {
        file = new uint8_t[4096];
        this->disk.read(current_blk, file);
        // this->disk.write(blk_needed[i], file);
        data += (char*)file;
        // fat_entries[file_blk] = blk_needed[i];

        // file_blk = fat_entries[file_blk];
        current_blk = fat_entries[current_blk];
    }

    int file_blk = file2_copy->first_blk;
    while(fat_entries[file_blk] != FAT_EOF) {
        file_blk = fat_entries[file_blk];
    }

    int rest_size = 4096-file2_copy->size % 4096;
    file = new uint8_t[4096];
    this->disk.read(file_blk, file);
    std::string file_s = (char*)file;
    file_s += data.substr(0, std::min((int)data.size(),rest_size));
    this->disk.write(file_blk, (uint8_t*)file_s.c_str());

    nr_needed_blks = ceil((float)(data.size() - std::min((int)data.size(),rest_size)) / 4096);
    std::vector<int> blk_needed;
    for(int i = 2; i < 2048; i++) {
        if(fat_entries[i] == FAT_FREE) {
            if(nr_needed_blks > 1) {
                blk_needed.push_back(i);
                nr_needed_blks--;
            } else {
                blk_needed.push_back(i);
                break;
            }
        }
    }
    std::string testimus_prime = data.substr(std::min((int)data.size(),rest_size), data.size());
    this->disk.write(blk_needed[0], (uint8_t*)testimus_prime.c_str());
    fat_entries[file_blk] = blk_needed[0];
    
    for(int i = 1; i < blk_needed.size(); i++) {
        int yo = data.size() + file_s.size() - std::min((int)data.size(),rest_size);
        this->disk.write(blk_needed[i], (uint8_t*)data.substr(std::min(4096, yo)).c_str());
        fat_entries[file_blk] = blk_needed[i];
        file_blk = fat_entries[file_blk];
        yo -= std::min(4096, yo);
    }
    file2_copy->size += file_copy->size;
    this->disk.write(current_dir_blk, (uint8_t*)dir_entries);

    delete[] blk, dir_entries, file;

    return 0;
}

// mkdir <dirpath> creates a new sub-directory with the name <dirpath>
// in the current directory
int
FS::mkdir(std::string dirpath)
{
    // std::cout << "FS::mkdir(" << dirpath << ")\n";
    if(dirpath.size() > 55) {
        std::cout << "FS::mkdir(" << dirpath << ") -> filepath too long (>55)" << std::endl;
        return 1;
    }

    int current_dir_blk = get_current_dir_blk(dirpath);
    // std::cout << dirpath;
    uint8_t* blk = new uint8_t[4096];
    this->disk.read(current_dir_blk, blk);
    dir_entry* curr_dir_entries = (dir_entry*)blk;

    for(int i = 0; i < 64; i++) {
        if(curr_dir_entries[i].file_name == dirpath) {
            std::cout << "FS::mkdir(" << dirpath << ") -> can't create dir with this name" << std::endl;
            return 1; 
        }
    }

    dir_entry* file = new dir_entry;

    // Init file name
    std::string filename = dirpath.substr(dirpath.find_last_of("/") + 1);
    std::strcpy(file->file_name, filename.c_str());

    // Init dir type
    file->type = 1; // (1) = directory, (0) = file

    // Init file access rights
    file->access_rights = READ + WRITE; // beror på access rights

    // Init (Save) data
    blk = new uint8_t[4096];
    this->disk.read(1, blk);
    int16_t* fat_entries = (int16_t*)blk;
    int block_no;
    int nr_needed_blks = 1;

    std::vector<int> blk_needed;
    for(int i = 2; i < 2048; i++) {
        if(fat_entries[i] == FAT_FREE) {
            if(nr_needed_blks > 1) {
                blk_needed.push_back(i);
                nr_needed_blks--;
            } else {
                blk_needed.push_back(i);
                break;
            }
        }
    }
    // std::cout << blk_needed[0] << std::endl;
    file->first_blk = blk_needed[0];
    fat_entries[blk_needed[0]] = FAT_EOF;
    file->size = 0;
    this->disk.write(1, (uint8_t*)fat_entries);

    // Fill directory with empty dir_entries
    dir_entry* current_dir_entries = new dir_entry[64];
    dir_entry dir;
    dir.size = 0;
    dir.first_blk = 0;
    dir.type = 2;
    for(int i = 0; i < 64; i++) {
        memcpy(&current_dir_entries[i], &dir, sizeof(dir_entry));
    }
    this->disk.write(file->first_blk, (uint8_t*)current_dir_entries);


    // Maybe add error handling here
    delete[] blk, fat_entries;

    // See + Write to root
    current_dir_blk = get_current_dir_blk(dirpath);
    blk = new uint8_t[4096];
    this->disk.read(current_dir_blk, blk);
    dir_entry* dir_entries = (dir_entry*)blk;

    for(int i = 0; i < 64; i++) {
        if(dir_entries[i].type == 2) {
            memcpy(&dir_entries[i], file, sizeof(dir_entry));
            this->disk.write(current_dir_blk, blk);
            delete[] blk, dir_entries;
            return 0;
        }
    }

    std::cout << "FS::mkdir(" << dirpath << ") -> disk entry is full (>64)" << std::endl;
    return 1;
}

// cd <dirpath> changes the current (working) directory to the directory named <dirpath>
int
FS::cd(std::string dirpath)
{
    // std::cout << "FS::cd(" << dirpath << ")\n";
    std::string wanted_path, dir_name;
    if(dirpath == ".."){
        if(current_dir.length() > 1){
            current_dir = current_dir.substr(0, current_dir.length()-2);
            current_dir = current_dir.substr(0, current_dir.find_last_of("/"));
            return 0;
        }
    } else if (dirpath.find('/') == std::string::npos) {
        wanted_path = current_dir;
        dir_name = dirpath;
    } else {
        if(dirpath.find_first_of('/') == 0) {
            wanted_path = dirpath;
        } else if(dirpath.find("../") != std::string::npos) {
            wanted_path = current_dir +dirpath;
            // std::cout << "Original wanted path: " << wanted_path << std::endl;
            int i = 2;
            while(wanted_path.find("../") != std::string::npos) {
                wanted_path = wanted_path.substr(0, wanted_path.find("../")-1);
                wanted_path = wanted_path.substr(0, wanted_path.find_last_of('/'));
                if(wanted_path.empty()) break;
                wanted_path += dirpath.substr(dirpath.find("../")+2, dirpath.size());
            }
        } else {
            wanted_path = current_dir + '/' + dirpath;
        }

        if(wanted_path.empty()) wanted_path = "/";

        current_dir = wanted_path;
        return 0;

        // std::cout << "FINAL : " << wanted_path;

        if(wanted_path.find_last_of('/') > 1) {
            wanted_path.substr(0, wanted_path.find_last_of('/')-1);
        } else {
            current_dir = wanted_path;
            return 0;
        }

        // std::cout << "WANTED PAAAAATH: " << wanted_path << std::endl;
        dir_name = wanted_path.substr(wanted_path.find_last_of("/") + 1);
        // std::cout << "DIR NAME: " << dir_name << std::endl;
    }


    // int current_blk = get_current_dir_blk(dirpath);
    int current_blk = get_current_dir_blk(wanted_path);
    uint8_t* blk = new uint8_t[4096];
    disk.read(current_blk, blk);
    dir_entry* dir_entries = (dir_entry*)blk;
    for(int i = 0; i < 64; i++) {
        if(dir_entries[i].file_name == dir_name && dir_entries[i].type == 1) {
            if(current_dir.size() > 1) {
                current_dir += '/';
            }
            current_dir += dir_name;
            delete[] blk;
            return 0;
        }
    }
    std::cout << "FS::cd(" << dirpath << ") -> Can't cd into non-existing dir\n" << std::endl;
    delete[] blk;
    return 1;
}

// pwd prints the full path, i.e., from the root directory, to the current
// directory, including the currect directory name
int
FS::pwd()
{
    // std::cout << "FS::pwd()\n";
    if(current_dir.empty()) current_dir = "/";
    std::cout << current_dir << std::endl;
    return 0;
}

// chmod <accessrights> <filepath> changes the access rights for the
// file <filepath> to <accessrights>.
int
FS::chmod(std::string accessrights, std::string filepath)
{
    std::cout << "FS::chmod(" << accessrights << "," << filepath << ")\n";

    std::string sourcepath_dir = filepath.substr(0, filepath.find_last_of('/')+1);
    std::string sourcepath_filename = filepath.substr(filepath.find_last_of('/')+1, filepath.size());
    int current_dir_blk = get_current_dir_blk(sourcepath_dir);
    uint8_t* blk = new uint8_t[4096];
    this->disk.read(current_dir_blk, blk);
    dir_entry* dir_entries = (dir_entry*)blk;

     for(int i = 0; i < 64; i++) {
        if(dir_entries[i].file_name == sourcepath_filename) {
            dir_entries[i].access_rights = (uint8_t)std::stoi(accessrights);
            this->disk.write(current_dir_blk, blk);
            delete[] blk, dir_entries;
        }
    }
    return 0;
}