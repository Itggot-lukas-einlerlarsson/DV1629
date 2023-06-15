#include <iostream>
#include "fs.h"

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
    std::cout << "FS::format()\n";
    // block nr 0 is root dir
    dir_entry* root_dir = new dir_entry[BLOCK_SIZE/DIR_ENTRY_SIZE];
    for (size_t i = 0; i < BLOCK_SIZE/DIR_ENTRY_SIZE; i++) {
        root_dir[i].size = 0;
        root_dir[i].first_blk = ROOT_BLOCK;
        root_dir[i].type = TYPE_DIR;
    }
    this->disk.write(ROOT_BLOCK, (uint8_t*)root_dir);
    delete[] root_dir;

    // block nr 1 is File Allocation Table
    uint16_t* fat = new uint16_t[BLOCK_SIZE/2]; // Whole FAT in one block, we can address 4096 / 2 = 2048 disk blocks in a partition
    fat[0] = FAT_EOF; fat[1] = FAT_EOF; // first two blocks are occupied
    for (size_t i = 2; i < BLOCK_SIZE/2; i++) {
        fat[i] = FAT_FREE;
    }
    this->disk.write(FAT_BLOCK, (uint8_t*)fat);
    delete[] fat;
    return 0;
}

// create <filepath> creates a new file on the disk, the data content is
// written on the following rows (ended with an empty row)
int
FS::create(std::string filepath)
{
    std::cout << "FS::create(" << filepath << ")\n";
    if (filepath.size() > 55) {
        std::cout << "Filename is too long (>55 characters)" << '\n';
        return -1;
    }
    dir_entry* new_file = new dir_entry;
    std::string filename = filepath.substr(filepath.find_last_of("/") + 1);
    strncpy(new_file->file_name, filename.c_str(), sizeof(filename));
    std::cout << new_file->file_name << '\n';
    new_file->access_rights = READ + WRITE + EXECUTE; // Access rights of a file or directory should be ’rw-’ or ’rwx’ when the file or directory is created. (7)
    std::string line;
    std::string data;
    std::getline(std::cin, line);
    while(line != "") {
        data += line + "\n";
        std::getline(std::cin, line);
    }
    new_file->size = data.size();
    std::cout << data << '\n';
    delete new_file;

    //write to disk....
    // new_file->first_blk = ...;
    return 0;
}

// cat <filepath> reads the content of a file and prints it on the screen
int
FS::cat(std::string filepath)
{
    std::cout << "FS::cat(" << filepath << ")\n";
    return 0;
}

// ls lists the content in the currect directory (files and sub-directories)
int
FS::ls()
{
    std::cout << "FS::ls()\n";
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
