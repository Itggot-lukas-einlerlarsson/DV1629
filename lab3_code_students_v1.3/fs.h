#include <iostream>
#include <cstdint>
#include "disk.h"

#include <cstring>
#include <cmath>
#include <vector>
#include <sstream>

#ifndef __FS_H__
#define __FS_H__

#define ROOT_BLOCK 0
#define FAT_BLOCK 1
#define FAT_FREE 0
#define FAT_EOF -1

#define TYPE_FILE 0
#define TYPE_DIR 1
#define READ 0x04
#define WRITE 0x02
#define EXECUTE 0x01

#define DIR_ENTRY_SIZE 64 //sizeof(test_dir)
// #define MAX_NO_FILES_IN_DIR 64 //BLOCK_SIZE/sizeof(test_dir)

struct dir_entry {
    char file_name[56]; // name of the file / sub-directory
    uint32_t size; // size of the file in bytes
    uint16_t first_blk; // index in the FAT for the first block of the file
    uint8_t type; // directory (1) or file (0)
    uint8_t access_rights; // read (0x04), write (0x02), execute (0x01)
};

class FS {
private:
    Disk disk;
    // int16_t fat[BLOCK_SIZE/2];

    /* ----------   Help functions/private variables  ---------- */
    // more info about each function in their declaration
    std::string current_working_dir;
    int current_dir_block;
    std::string gather_info_new_dir_entry(int file_type, dir_entry* new_file, std::string filename);
    std::string gather_info_old_dir_entry(dir_entry* dir, dir_entry* new_file, std::string filename);
    int check_if_file_in_dir(dir_entry* dir, int file_type, const char file_name[56]);
    int save_entry_on_disk(std::string data, int16_t* fat, dir_entry* new_file);
    int save_entry_in_dir(int current_dir_block, dir_entry* dir, dir_entry* new_file);
    std::string get_filename(std::string filepath);
    std::string privilege_string(uint8_t privilege);
    std::vector<int> get_dir_blocks(std::string dirpath);
    int check_if_dir_full(int dir_block);
    int destination_dir_check(dir_entry* dir, std::string destpath, std::string destname);
    int privilege_check(uint8_t access_rights, uint8_t required_privilege);


public:
    // precreated
    FS();
    ~FS();
    // formats the disk, i.e., creates an empty file system
    int format();
    // create <filepath> creates a new file on the disk, the data content is
    // written on the following rows (ended with an empty row)
    int create(std::string filepath);
    // cat <filepath> reads the content of a file and prints it on the screen
    int cat(std::string filepath);
    // ls lists the content in the current directory (files and sub-directories)
    int ls();

    // cp <sourcepath> <destpath> makes an exact copy of the file
    // <sourcepath> to a new file <destpath>
    int cp(std::string sourcepath, std::string destpath);
    // mv <sourcepath> <destpath> renames the file <sourcepath> to the name <destpath>,
    // or moves the file <sourcepath> to the directory <destpath> (if dest is a directory)
    int mv(std::string sourcepath, std::string destpath);
    // rm <filepath> removes / deletes the file <filepath>
    int rm(std::string filepath);
    // append <filepath1> <filepath2> appends the contents of file <filepath1> to
    // the end of file <filepath2>. The file <filepath1> is unchanged.
    int append(std::string filepath1, std::string filepath2);

    // mkdir <dirpath> creates a new sub-directory with the name <dirpath>
    // in the current directory
    int mkdir(std::string dirpath);
    // cd <dirpath> changes the current (working) directory to the directory named <dirpath>
    int cd(std::string dirpath);
    // pwd prints the full path, i.e., from the root directory, to the current
    // directory, including the current directory name
    int pwd();

    // chmod <accessrights> <filepath> changes the access rights for the
    // file <filepath> to <accessrights>.
    int chmod(std::string accessrights, std::string filepath);
};

#endif // __FS_H__
