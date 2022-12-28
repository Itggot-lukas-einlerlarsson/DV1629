#include <iostream>
#include "fs.h"

FS::FS()
{
    std::cout << "FS::FS()... Creating file system\n";
}

FS::~FS()
{

}

void
FS::DiskWrite(int blockNr, std::string dInput){
  disk.write(blockNr, (uint8_t *) dInput.c_str());
}


//FindFile <file_path> <block>: Finds the file with the given path.
int FS::FindFile(std::string filePath, int block){ //Returns the dir_array index

    dir_entry toReturn = dir_entry();
    toReturn.type = 2;
    int index_return = -1;
    bool foundmatch = false;
    disk.read(block, (uint8_t *) dir_array);// gjorde dynamiskt block sök
    int i = 0;

    while(i < 64 && foundmatch == false){ //traverses the dir_array
      if(dir_array[i].type != 2){
        if(dir_array[i].file_name == filePath){
          toReturn = dir_array[i];
          foundmatch = true;
          index_return = i;
        }
      }
      i++;
    }

    return index_return;
}


//ReadFromFile <directory_entry>: reads the content of a file.
std::string FS::ReadFromFile(dir_entry readFile){
    disk.read(1, (uint8_t *) fat);
    std::string toReturn = "";
    char tempArr[BLOCK_SIZE];
    std::string tep = "";
    if(fat[readFile.first_blk] == FAT_EOF){ // If end of file
      disk.read(readFile.first_blk, (uint8_t *) tempArr);
      toReturn = tempArr;
      return toReturn; // return data as string
    }
    else{
      int currBlock = readFile.first_blk;
      while(currBlock != FAT_EOF){
        if(fat[currBlock] == FAT_EOF){
          disk.read(readFile.first_blk, (uint8_t *) tempArr);
          toReturn = tempArr + toReturn;
          currBlock = fat[currBlock];
        }
        else{
          disk.read(fat[currBlock], (uint8_t *) tempArr);
          toReturn = toReturn + tempArr;
          currBlock = fat[currBlock];
        }
      }
    }
    return toReturn;

}


//CreateFile <filename> <datainput>: Creates file with the name [name] and containing the. returns 0 if couldn't create file otherwise 1
int FS::CreateFile(std::string name, std::string input, uint8_t accRights){
    int returnValue = 0;
    dir_entry new_dir = dir_entry();
    disk.read(1, (uint8_t *) fat);
    std::vector<std::string> dividedInput;
    int sizeOfInput = input.size();
    int neededBlocks = 0;
    std::string toWrite;
    int temp = 0;
    int index = 0;
    index = FindFile(name, currBlock);
    disk.read(currBlock, (uint8_t *) dir_array);
    if(index != -1){
      std::cout << "Couldn't create file, filename taken\n";
      return returnValue;
    }

    if(sizeOfInput < 4096){ //Determines the amounts of needed blocks
      neededBlocks = 1;
    }
    else{
      int i = 2;
      bool foundSize = false;
      while(i < disk.get_no_blocks() && !foundSize){
        if(sizeOfInput < i*4096){
          foundSize = true;
          neededBlocks = i;
        }
        i++;
      }
    }
    if(neededBlocks > 0){
      for(int i = 0; i < name.length(); i++){
        new_dir.file_name[i] = name[i]; //Saves down the name
      }
      int y = 2;
      bool running = true;
      bool firstBlock = true;
      int prevIndex = 0;
      for (int b = 0; b < neededBlocks; b++){
        toWrite = input.substr(b*4095, (((b+1)*4096))); //splits the data
        dividedInput.push_back(toWrite);
      }
      int currBlock = 0;
      while(y < disk.get_no_blocks() && running){ //traverses the fat table to find the avalible blocks for data storage and saves it down to the fat table
        if(firstBlock && fat[y] == FAT_FREE && neededBlocks == 1){
          new_dir.first_blk = y;
          firstBlock = false;
          fat[y] = FAT_EOF;
          DiskWrite(y, dividedInput[currBlock]);
          currBlock++;
          running = false;
        }
        else if(firstBlock && fat[y] == FAT_FREE){
          new_dir.first_blk = y;
          firstBlock = false;
          neededBlocks--;
          DiskWrite(y, dividedInput[currBlock]);
          currBlock++;
          prevIndex = y;
        }
        else if(fat[y] == FAT_FREE && neededBlocks != 1){
          fat[prevIndex] = y;
          neededBlocks--;
          DiskWrite(y, dividedInput[currBlock]);
          currBlock++;
          prevIndex = y;
        }
        else if(fat[y] == FAT_FREE){
          fat[prevIndex] = y;
          fat[y] = FAT_EOF;
          DiskWrite(y, dividedInput[currBlock]);
          currBlock++;
          running = false;
        }
        y++;

      }

      new_dir.size = sizeOfInput;
      new_dir.type = TYPE_FILE;
      new_dir.access_rights = accRights; //saves the data to the dir_entry
      for(int a = 0; a < 64; a++){
        if(dir_array[a].type == 2){
          dir_array[a] = new_dir;
          temp = a;
          break;
        }
      }
    }
    returnValue = 1;
    disk.write(currBlock, (uint8_t *) dir_array);
    disk.write(1, (uint8_t *) fat); //saves the data to disk
    return returnValue;
}


//CreateDir<dirname> <block>: Creates a directory with the given name and starting at the block as given as parameters.
void FS::CreateDir(std::string name, int16_t block){
    disk.read(block, (uint8_t *) dir_array);
    disk.read(1, (uint8_t *) fat);
    dir_entry new_dir = dir_entry();

    int index = FindFile(name, block);

    if(index != -1){
      std::cout << "Name is taken\n";
      return;
    }



    bool running = true;
    for(int i = 0; i < name.length(); i++){
      new_dir.file_name[i] = name[i];
    }
    int y = 0;
    while(y < disk.get_no_blocks() && running){ //finds avalible fat slots
        if(fat[y] == FAT_FREE){
          new_dir.first_blk = y;
          fat[y] = FAT_EOF;
          running = false;
        }
        y++;
    }

    new_dir.size = 0;
    new_dir.type = TYPE_DIR; //hard coded data as specified for directories
    new_dir.access_rights = 0;

    for(int a = 0; a < 64; a++){
      if(dir_array[a].type == 2){
        dir_array[a] = new_dir;
        break;
      }
    }
    disk.write(block, (uint8_t *) dir_array);
    disk.write(1, (uint8_t *) fat);
    dir_entry tempArr[64];
    for (int i = 0; i < 64; i++){
      tempArr[i] = dir_entry();
      tempArr[i].type = 2;
    }
    tempArr[0].file_name[0] = '.'; //adds the .. folder that links back to the parent
    tempArr[0].file_name[1] = '.';
    tempArr[0].type = 1;
    tempArr[0].first_blk = block;
    disk.write(new_dir.first_blk, (uint8_t *) tempArr);

}

std::vector<std::string> FS::splitStr(std::string input){ //splits the given string and returns a vector with the elements of the string
    std::vector<std::string> toReturn;
    std::string delimiter = "/";
    size_t pos = 0;
    std::string tempStr;


    while ((pos = input.find(delimiter)) != std::string::npos) {
        tempStr = input.substr(0, pos);
        toReturn.push_back(tempStr);
        input.erase(0, pos + 1);
    }
    toReturn.push_back(input);
    return toReturn;
}


//access_helper <access_rights>: Is converting the accessrights from uint8_t to string.
std::string FS::access_helper(uint8_t acc_rights) {

  std::string out;
  switch (acc_rights) {
    case 0:
      out = "---";
      break;
    case 4:
      out = "r--";
      break;
    case 2:
      out = "-w-";
      break;
    case 1:
      out = "--x";
      break;
    case 6:
      out = "rw-";
      break;
    case 3:
      out = "-wx";
      break;
    case 5:
      out = "r-x";
      break;
    case 7:
      out = "rwx";
      break;
    default:
      out = "invalid input";
      break;

  }
  return out;
}


//access_checker <access_rights> <path>: Is checking if access is granted to the given path.
int FS::access_checker(uint8_t acc_rig, std::string path) {

      int passed = 0;
      int index;
      index = FindFile(path, currBlock);
      disk.read(currBlock, (uint8_t *) dir_array);
      int tmpBlock = currBlock;
      std::vector<std::string> inputVector = splitStr(path);

      for(int i = 0; i < inputVector.size()-1; i++){
        index = FindFile(inputVector[i], tmpBlock); // Ger index i dir_array
        disk.read(tmpBlock, (uint8_t *) dir_array);

        if(index == -1) {
          std::cout <<"File not found" << std::endl;
          passed = -1;
        }
        tmpBlock = dir_array[index].first_blk; // Slutar på mappen som håller filen

      }
      index = FindFile(inputVector[inputVector.size()-1], tmpBlock);

      if(dir_array[index].type != 0){
        std::cout << "Error: Its not a file" << std::endl;
        return 0;
      }
      else {
        if(acc_rig == dir_array[index].access_rights) { // Kolla om access rights är lika
          passed = 1;
        }
      }
  return passed;
}



//isDir <dirpath>: Is checking if given path belongs to dir, file or if exists. Returns true if dir otherwise false.
bool FS::isDir(std::string path) {

      bool isDir = false;
      int index;
      index = FindFile(path, currBlock);
      disk.read(currBlock, (uint8_t *) dir_array);
      int tmpBlock = currBlock;
      std::vector<std::string> inputVector = splitStr(path);

      for(int i = 0; i < inputVector.size()-1; i++){
        index = FindFile(inputVector[i], tmpBlock); // Ger index i dir_array
        disk.read(tmpBlock, (uint8_t *) dir_array);

        if(index == -1) {// No such file
          return 0;
        }
        tmpBlock = dir_array[index].first_blk; // Slutar på mappen som håller filen
      }
      index = FindFile(inputVector[inputVector.size()-1], tmpBlock);

      if(dir_array[index].type == 1){ // Är det en dir
          isDir = true;
      }

  return isDir;



}

// formats the disk, i.e., creates an empty file system
int
FS::format()
{
    //sätt upp FATtable
    fat[ROOT_BLOCK] = FAT_EOF;
    fat[FAT_BLOCK] = FAT_EOF;
    std::vector<uint16_t> empty;
    for(int i = 0; i < 64; i++){
      dir_array[i] = dir_entry();
      dir_array[i].type = 2;
    }

    for(int i = 2; i < disk.get_no_blocks(); i++){ // Sätter hela fattablet till FAT_FREE
      fat[i] = FAT_FREE;
    }
    disk.write(0, (uint8_t *) dir_array);
    disk.write(1, (uint8_t *) fat);
    cwd = "/";
    currBlock = 0;
    parents = empty;
    std::cout << "FS::format()\n";
    return 0;
}

// create <filepath> creates a new file on the disk, the data content is
// written on the following rows (ended with an empty row)
int
FS::create(std::string filepath)
{
    uint16_t tmpBlock = currBlock;
    if(filepath[0] == '/'){ //adds absolute path
      filepath.erase(0,1);
      currBlock = 0;
    }
    std::vector<std::string> inputVector = splitStr(filepath);
    std::string input = "";
    std::string wInput = "";
    int temp = currBlock;
    int currIndex = -1;
    while (true){//get input
      getline(std::cin, input);

      if(input.size() == 0){
        break;
      }
      else{
        wInput = wInput + input;
      }
    }
    if(inputVector.size() == 1){ //creates the file with the given input and name
      CreateFile(filepath, wInput, READ + WRITE);
    }
    else{//In case the given string is a path
      for(int i = 0; i < inputVector.size() - 1; i++){
        currIndex = FindFile(inputVector[i], currBlock);
        disk.read(currBlock, (uint8_t *) dir_array);
        if(currIndex == -1 || dir_array[currIndex].type == 2){
          std::cout << "Couldn't find path to requested directory!\n";
          currBlock = tmpBlock;
          return 0;
        }
        currBlock = dir_array[currIndex].first_blk;
      }

      CreateFile(inputVector[inputVector.size()-1], wInput, READ + WRITE);
      currBlock = temp;
    }
    currBlock = tmpBlock;
    filepath = "/" + filepath;
    std::cout << "FS::create(" << filepath << ")\n";
    return 0;
}

// cat <filepath> reads the content of a file and prints it on the screen
int
FS::cat(std::string filepath)
{
    uint16_t tmpBlock = currBlock;
    if(filepath[0] == '/'){
      filepath.erase(0,1);
      currBlock = 0;
    }

    if(FindFile(filepath, currBlock) != -1) {
      // Checking for read permissions
      if(access_checker(4, filepath) == 1 || access_checker(5, filepath) == 1 || access_checker(6, filepath) == 1 || access_checker(7, filepath) == 1 ) {
        disk.read(1, (uint8_t *) fat);
        int temp = currBlock;
        int currBlockIndex = currBlock;
        int currIndex = -1;
        std::vector<std::string> inputVector = splitStr(filepath);
        if(inputVector.size() == 1){
          int index = FindFile(filepath, currBlock);
          disk.read(currBlock, (uint8_t *) dir_array);
          std::string output;
          if(dir_array[index].type != 2) {
            output = ReadFromFile(dir_array[index]);  // Läs från disk mha helper function
            std::cout << output << std::endl; // skriver datan i terminalen
          }
          else {
            std::cout << "No file to cat" << std::endl;
          }
        }
        else{
          currBlockIndex = currBlock;
          for(int i = 0; i < inputVector.size() - 1; i++){
            currIndex = FindFile(inputVector[i], currBlockIndex);
            disk.read(currBlockIndex, (uint8_t *) dir_array);
            if(currIndex == -1){
              std::cout << "Couldn't find path to requested directory!\n";
              currBlock = tmpBlock;
              return 0;
            }
            currBlockIndex = dir_array[currIndex].first_blk;
          }
          currBlock = currBlockIndex;
          cat(inputVector[inputVector.size()-1]);
          currBlock = temp;
        }
      }
      else {
        std::cout << "Error: No rights for this file" << std::endl;
      }
    }
    else {
      std::cout << "Error: no file to cat" << std::endl;
    }
    currBlock = tmpBlock;
    return 0;
}

// ls lists the content in the currect directory (files and sub-directories)
int
FS::ls()
{
  disk.read(currBlock, (uint8_t * ) dir_array);
  std::string acc_rights = "";
  int acc_rights_len = 3; // alltid samma längd

  std::string type = "";
  int type_len = 0;

  std::string name = "";
  int name_len = 0;

  std::string size = "";
  int size_len = 0;

  std::string out = "";

  std::cout << "name";
  std::cout << std::setw(59);
  std::cout << "type";
  std::cout << std::setw(21);
  std::cout << "accessrights";
  std::cout << std::setw(15);
  std::cout << "size" << std::endl;


  for(int i = 0; i < 64; i++) {

    if(dir_array[i].type == 0 || dir_array[i].type == 1) { // if a file is found then collect info and print to terminal

      type = std::to_string(dir_array[i].type);
      type_len = std::to_string(dir_array[i].type).length();

      name = dir_array[i].file_name;
      name_len = name.length();

      size = std::to_string(dir_array[i].size);
      size_len = std::to_string(dir_array[i].size).length();

      acc_rights = access_helper(dir_array[i].access_rights);
      std::cout << name << std::setw(61-name_len);
      std::cout << type <<std::setw(16-type_len);
      std::cout << acc_rights << std::setw(25-acc_rights_len);
      std::cout << size << std::endl;
    }
  }

  std::cout << "FS::ls()\n";
  return 0;

}

// cp <sourcepath> <destpath> makes an exact copy of the file
// <sourcepath> to a new file <destpath>
int
FS::cp(std::string sourcepath, std::string destpath)
{
    uint8_t accRights = READ;
    uint16_t currBlock1 = currBlock;
    uint16_t currBlock2 = currBlock;

    if(sourcepath[0] == '/'){ //handles absolute paths
      sourcepath.erase(0, 1);
      currBlock1 = 0;
    }
    if(destpath[0] == '/'){
      destpath.erase(0, 1);
      currBlock2 = 0;
    }

    int permListSrc[4] = {4, 5, 6, 7}; // Read permissions
    bool passSrc = 0;

    for(int i = 0; i<sizeof(permListSrc)/sizeof(permListSrc[0]); i++) {
      if(access_checker(permListSrc[i], sourcepath) == 1) {  // om läsrättigheter uppfyllda
        passSrc = 1;
      }
    }
    std::vector<std::string> inputVector = splitStr(sourcepath);
    int tmpBlock = currBlock1;
    int index = -1;
    int findSrc = -1;
    index = FindFile(inputVector[inputVector.size()-1], currBlock1); // Ger index i dir_array

    if(index != -1) { // Checks if the file exists
      if(passSrc == 1) {  // checks if the permissions is furfilled

        int currBlockIndexIn = currBlock1;
        int currBlockIndexOut = currBlock2;
        int index = -1;
        int temp = currBlock;
        std::vector<std::string> inputVector = splitStr(sourcepath);
        std::vector<std::string> outputVector = splitStr(destpath);
        if(inputVector.size() == 1 && outputVector.size() == 1){
          index = FindFile(sourcepath, currBlock1);
          accRights = dir_array[index].access_rights;

          disk.read(currBlock1, (uint8_t *) dir_array);
          std::string ret;
          if(dir_array[index].type != 0){
            std::cout << "Unable to find object\n";
            return 0;
          }
          else{
            ret = ReadFromFile(dir_array[index]); //finds the data from the source file
            index = FindFile(destpath, currBlock2);
            disk.read(currBlock2, (uint8_t *) dir_array);
            std::string tempName = destpath;
            if(dir_array[index].type == 1){
              currBlock = dir_array[index].first_blk;
              tempName = sourcepath;
            }
            CreateFile(tempName, ret, accRights);
            currBlock = temp;
          }
        }
        else if(inputVector.size() == 1){
          index = FindFile(sourcepath, currBlock1);
          accRights = dir_array[index].access_rights;
          disk.read(currBlock1, (uint8_t *) dir_array);
          std::string ret;
          if(dir_array[index].type != 0){
            std::cout << "Unable to find object\n";
            return 0;
          }
          else{
            ret = ReadFromFile(dir_array[index]);
            for(int i = 0; i < outputVector.size() - 1; i++){
              index = FindFile(outputVector[i], currBlockIndexOut);
              disk.read(currBlockIndexOut, (uint8_t *) dir_array);
              if(index == -1){
                std::cout << "Couldn't find path to requested directory!\n";
                return 0;
              }
              currBlockIndexOut = dir_array[index].first_blk;
            }
            currBlock = currBlockIndexOut; // currBlockIndexOut is where we want to write
            std::string tempName = outputVector[outputVector.size()-1];
            index = FindFile(outputVector[outputVector.size()-1], currBlockIndexOut);
            disk.read(currBlockIndexOut, (uint8_t *) dir_array);
            if(dir_array[index].type == 1){
              currBlock = dir_array[index].first_blk;
              tempName = sourcepath;
            }

            CreateFile(tempName, ret, accRights);
            currBlock = temp;
          }
        }
        else if(outputVector.size() == 1){
          std::string ret;
          for(int i = 0; i < inputVector.size() - 1; i++){
            index = FindFile(inputVector[i], currBlockIndexIn);
            disk.read(currBlockIndexIn, (uint8_t *) dir_array);
            if(index == -1){
              std::cout << "Couldn't find path to requested directory!\n";
              return 0;
            }
            currBlockIndexIn = dir_array[index].first_blk;
          }
          index = FindFile(inputVector[inputVector.size() - 1], currBlockIndexIn);
          disk.read(currBlockIndexIn, (uint8_t *) dir_array);
          accRights = dir_array[index].access_rights;
          if(dir_array[index].type != 0){
            std::cout << "Unable to find object\n";
            return 0;
          }
          else{
            ret = ReadFromFile(dir_array[index]);
            index = FindFile(destpath, currBlock2);
            disk.read(currBlock2, (uint8_t *) dir_array);
            std::string tempName = destpath;
            if(dir_array[index].type == 1){
              currBlock = dir_array[index].first_blk;
              tempName = inputVector[inputVector.size()-1];
            }
            CreateFile(tempName, ret, accRights);
            currBlock = temp;
          }
        }
        else{
          std::string ret;
          for(int i = 0; i < inputVector.size() - 1; i++){
            index = FindFile(inputVector[i], currBlockIndexIn);
            disk.read(currBlockIndexIn, (uint8_t *) dir_array);
            if(index == -1){
              std::cout << "Couldn't find path to requested directory!\n";
              return 0;
            }
            currBlockIndexIn = dir_array[index].first_blk;
          }
          index = FindFile(inputVector[inputVector.size() - 1], currBlockIndexIn);
          accRights = dir_array[index].access_rights;
          disk.read(currBlockIndexIn, (uint8_t *) dir_array);
          ret = ReadFromFile(dir_array[index]);
          for(int i = 0; i < outputVector.size() - 1; i++){
            index = FindFile(outputVector[i], currBlockIndexOut);
            disk.read(currBlockIndexOut, (uint8_t *) dir_array);
            if(index == -1){
              std::cout << "Couldn't find path to requested directory!\n";
              return 0;
            }
            currBlockIndexOut = dir_array[index].first_blk;
          }
          currBlock = currBlockIndexOut;
          std::string tempName = outputVector[outputVector.size()-1];
          index = FindFile(outputVector[outputVector.size()-1], currBlockIndexOut);
          disk.read(currBlockIndexOut, (uint8_t *) dir_array);
          if(outputVector[outputVector.size()-1] == "" || dir_array[index].type == 1){
            tempName = inputVector[inputVector.size()-1];
            currBlock = dir_array[index].first_blk;
          }
          CreateFile(tempName, ret, accRights);
          currBlock = temp;
        }
        sourcepath = "/" + sourcepath;
        destpath = "/" + destpath;
        std::cout << "FS::cp(" << sourcepath << "," << destpath << ")\n";
      }
      else {
        std::cout << "No access rights for file" << std::endl;
      }
    }
    else {
      std::cout << "Can't find file" << '\n';
    }
    return 0;
}

// mv <sourcepath> <destpath> renames the file <sourcepath> to the name <destpath>,
// or moves the file <sourcepath> to the directory <destpath> (if dest is a directory)
int
FS::mv(std::string sourcepath, std::string destpath)
{

    uint16_t currBlock1 = currBlock;
    uint16_t currBlock2 = currBlock;
    bool sourceAbs = 0;
    if(sourcepath[0] == '/'){
      sourcepath.erase(0, 1); //handles absolute paths
      currBlock1 = 0;
      sourceAbs = 1;
    }
    if(destpath[0] == '/'){
      destpath.erase(0, 1);
      currBlock2 = 0;
    }
    int permListSrc[4] = {4, 5, 6, 7}; // Read permissions
    int passSrc = 0;

    for(int i = 0; i<sizeof(permListSrc)/sizeof(permListSrc[0]); i++) {
      if(access_checker(permListSrc[i], sourcepath) == 1) {
        passSrc = 1;
      }
    }
    int index = -1;
    int findSrc = -1;
    int tmpBlock1 = currBlock1;
    int tmpBlock2 = currBlock2;
    uint8_t accRights = READ;



    std::vector<std::string> inputVector1 = splitStr(sourcepath);
    if (inputVector1.size() < 2) {
      index = FindFile(inputVector1[0], tmpBlock1);    // looks for srcfile
      if(index != -1) {
        findSrc = 1;
      }
    }
    else {
      for(int i = 0; i < inputVector1.size()-1; i++){ // looks for srcfile
        index = FindFile(inputVector1[i], tmpBlock1); // Ger index i dir_array
        disk.read(tmpBlock1, (uint8_t *) dir_array);

        if(index != -1) {
          findSrc = 1;
        }
      }
    }

    if(dir_array[index].type == 0) {
      accRights = dir_array[index].access_rights;
    }

    bool dir = isDir(destpath);
    int canRemove = 0;
    if(findSrc == 1) { // Om Srcfilen finns

      if(passSrc == 1) {    //Om permissions är uppnådda eller att det är en mapp

        std::vector<std::string> inputVector = splitStr(sourcepath);
        std::vector<std::string> outputVector = splitStr(destpath);
        int currBlockIndex = currBlock1;
        int tempBlock = currBlock; // tempBlock is to reset currblock later in function.
        int index;
        std::string ret;  // ret is the data in file
        index = FindFile(inputVector[0], currBlockIndex); // Finds dir

        if(index == -1){
          std::cout << "Couldn't find path to requested directory!\n";
        }

        for(int i = 0; i < inputVector.size() - 1; i++){
          index = FindFile(inputVector[i], currBlockIndex); // Hittar rätt block där filen ligger
          disk.read(currBlockIndex, (uint8_t *) dir_array);
          if(index == -1){
            std::cout << "Couldn't find path to requested directory!\n";
            return 0;
          }
          currBlockIndex = dir_array[index].first_blk;
        }

        if(inputVector.size() != 1){
          index = FindFile(inputVector[inputVector.size() - 1], currBlockIndex);
        }
        disk.read(currBlockIndex, (uint8_t *) dir_array);
        ret = ReadFromFile(dir_array[index]);

        currBlockIndex = currBlock2;
        for(int i = 0; i < outputVector.size()-1; i++){
          index = FindFile(outputVector[i], currBlockIndex);
          disk.read(currBlockIndex, (uint8_t *) dir_array);
          if(index == -1){
            std::cout << "Couldn't find path to requested directory!\n";
            return 0;
          }
          currBlockIndex = dir_array[index].first_blk;
        }
        index = FindFile(outputVector[outputVector.size()-1], currBlockIndex);
        disk.read(currBlockIndex, (uint8_t *) dir_array);

        if(dir_array[index].type == 0 && index != -1){
          std::cout << "File name taken!\n";
          return 0;
        }
        else if(dir_array[index].type == 1 && index != -1){
          currBlock = dir_array[index].first_blk;
          canRemove = CreateFile(inputVector[inputVector.size()-1], ret, accRights);
          if(canRemove == 1) {
            if(sourceAbs == 1){
              sourcepath = "/" + sourcepath;
              currBlock = currBlock1;
            }
            currBlock = tempBlock;
            rm(sourcepath);
            if(sourcepath[0] == '/'){
              sourcepath.erase(0, 1);
            }
          }
          currBlock = tempBlock;
        }
        else{
          currBlock = currBlockIndex;
          canRemove = CreateFile(outputVector[outputVector.size()-1], ret, accRights);
          if(canRemove == 1) {
            currBlock = tempBlock;
            if(sourceAbs == 1){
              sourcepath = "/" + sourcepath;
              currBlock = currBlock1;
            }
            rm(sourcepath);
            if(sourcepath[0] == '/'){
              sourcepath.erase(0, 1);
            }
          }
          currBlock = tempBlock;
        }
        sourcepath = "/" + sourcepath;
        destpath = "/" + destpath;
        std::cout << "FS::mv(" << sourcepath << "," << destpath << ")\n";
      }
      else {
        std::cout << "No access rigths" << std::endl;
      }
    }
    else {
      std::cout << "Can't find file" << std::endl;
    }
    return 0;
}

// rm <filepath> removes / deletes the file <filepath>
int
FS::rm(std::string filepath)
{
  uint16_t tmpBlock = currBlock;
  if(filepath[0] == '/'){ //handles absolute paths
    filepath.erase(0, 1);
    currBlock = 0;
  }
  disk.read(1, (uint8_t *) fat);
  int temp = currBlock;
  int currBlockIndex = currBlock;
  int currIndex = -1;
  std::vector<std::string> inputVector = splitStr(filepath);

  if(inputVector.size() == 1){
    if(inputVector[0] == ".."){ //Makes the .. undeleteable
      std::cout << "Can't remove the .. directory!\n";
      currBlock = tmpBlock;
      return 0;
    }
    int index = FindFile(filepath, currBlock);
    disk.read(currBlock, (uint8_t *) dir_array);
    if(index == -1){
      std::cout << "Cant't find file2\n";
      currBlock = tmpBlock;
      return 0;
    }

    if(dir_array[index].type == 0){

      bool exit = false;
      int counter = 0;
      int16_t next_block = fat[dir_array[index].first_blk];
      int16_t prev_block = dir_array[index].first_blk;
      dir_array[index].type = 2;

      if (next_block == FAT_EOF) {
        fat[dir_array[index].first_blk] = FAT_FREE; // Sätt block to FAT_FREE
      }
      else {

        while (next_block != FAT_EOF) {
          fat[prev_block] = FAT_EOF;

          prev_block = next_block;
          next_block = fat[next_block];
        }
      }

      disk.write(currBlock, (uint8_t *) dir_array);// Write changes to disk
      disk.write(1, (uint8_t *) fat);
      filepath = "/" + filepath;
      std::cout << "FS::rm(" << filepath << ")\n";

    }
    else if(dir_array[index].type == 1){ // If its a dir
      disk.read(dir_array[index].first_blk, (uint8_t *) dir_array);
      int count = 0;
      for(int i = 0; i < 64; i++){  // Counts files in dir
        if(dir_array[i].type != 2){
          count++;
        }
      }
      if(count > 1){  // if elements in dir > 1 --> dir not empty
        std::cout << "Directory not empty!\n";
        currBlock = tmpBlock;
        return 0;
      }
      disk.read(currBlock, (uint8_t *) dir_array);
      fat[dir_array[index].first_blk] = FAT_FREE;
      dir_array[index].type = 2;
      disk.write(currBlock, (uint8_t *) dir_array);
      disk.write(1, (uint8_t *) fat);
      filepath = "/" + filepath;
      std::cout << "FS::rm(" << filepath << ")\n";
    }
    else {
      std::cout << "No file to remove" << std::endl;
      currBlock = tmpBlock;
      return 0;
    }

  }
  else{
    currBlockIndex = currBlock;
    for(int i = 0; i < inputVector.size() - 1; i++){
        currIndex = FindFile(inputVector[i], currBlockIndex);
        disk.read(currBlockIndex, (uint8_t *) dir_array);
        if(currIndex == -1){
          std::cout << "Couldn't find path to requested directory!\n";
          currBlock = tmpBlock;
          return 0;
        }
        currBlockIndex = dir_array[currIndex].first_blk;

    }
    currBlock = currBlockIndex;
    rm(inputVector[inputVector.size()-1]);
    currBlock = temp;
    filepath = "/" + filepath;
  }
  currBlock = tmpBlock;
  return 0;
}

// append <filepath1> <filepath2> appends the contents of file <filepath1> to
// the end of file <filepath2>. The file <filepath1> is unchanged.
int
FS::append(std::string filepath1, std::string filepath2)
{

      uint16_t currBlock1 = currBlock;
      uint16_t currBlock2 = currBlock;

      if(filepath1[0] == '/'){ //handles absolute paths
        filepath1.erase(0, 1);
        currBlock1 = 0;
      }
      if(filepath2[0] == '/'){
        filepath2.erase(0, 1);
        currBlock2 = 0;
      }

      int permListSrc[4] = {4, 5, 6, 7}; // Read permissions
      int permListDest[4] = {2, 3, 6, 7}; // Write permissions
      bool passSrc = false;
      bool passDest = false;

      int index1;
      int index2;
      uint8_t accRights = READ;
      int tmpBlock1 = currBlock1;
      int tmpBlock2 = currBlock2;
      int tmpBlock3 = currBlock;

      std::string ret1 = "";
      std::string ret2 = "";

      std::vector<std::string> inputVector1 = splitStr(filepath1);
      std::vector<std::string> inputVector2 = splitStr(filepath2);
     
      for(int i = 0; i < inputVector1.size() - 1; i++){
        int index = FindFile(inputVector1[i], tmpBlock1); // Ger index i dir_array
        disk.read(tmpBlock1, (uint8_t *) dir_array);

        if(index == -1) { // if file1 do not exists
          std::cout <<"Can't find file" << std::endl;
          return 0;
        }
        tmpBlock1 = dir_array[index].first_blk; // Slutar på mappen som håller filen
      }

      int index = FindFile(inputVector1[inputVector1.size()-1], tmpBlock1);
      if(dir_array[index].type != 0 || index == -1){ // if not a file
        std::cout << "No file found" << std::endl;
        return 0;
      }
      else {
        ret1 = ReadFromFile(dir_array[index]);
      }

      // Hämtar data och info från filepath2
      for(int i = 0; i < inputVector2.size()-1; i++){
        int index = FindFile(inputVector2[i], tmpBlock2); // Ger index i dir_array
        disk.read(tmpBlock2, (uint8_t *) dir_array);

        if(index == -1) {  // if file2 dont exits
          std::cout <<"No such file " << std::endl;
          return 0;
        }

        tmpBlock2 = dir_array[index].first_blk; // Slutar på mappen håller filen
      }
      index = FindFile(inputVector2[inputVector2.size()-1], tmpBlock2);
      accRights = dir_array[index].access_rights;
      if(dir_array[index].type != 0 || index == -1){
        std::cout << "No such file" << std::endl;
        return 0;
      }
      else {
        ret2 = ReadFromFile(dir_array[index]);
      }

      for(int i = 0; i<sizeof(permListSrc)/sizeof(permListSrc[0]); i++) {
        if(access_checker(permListSrc[i], filepath1) == 1) {  // Checks if file1 has the right permissions
          passSrc = 1;
        }
      }
      for(int i = 0; i<sizeof(permListDest)/sizeof(permListDest[0]); i++) {
          for(int j = 0; j<sizeof(permListSrc)/sizeof(permListSrc[0]); j++) {

            if(access_checker(permListDest[i], filepath2) == 1 && access_checker(permListSrc[j], filepath2) == 1) {// Checks for read & qrite permissions
              passDest = 1; //
            }
          }
      }
      if(passSrc == 1 && passDest == 1) {

        ret2 = ret2 + "\n";
        ret2 = ret2 + ret1; // Appends data to filepath2

        rm(filepath2);
        disk.read(tmpBlock1, (uint8_t *) dir_array);
        currBlock = tmpBlock2;

        CreateFile(inputVector2[inputVector2.size()-1], ret2, accRights);
        currBlock = tmpBlock3;
        std::cout << "FS::append(" << filepath1 << "," << filepath2 << ")\n";
      }
      else {
          std::cout << "No access rights" << std::endl;
      }

    return 0;
}

// mkdir <dirpath> creates a new sub-directory with the name <dirpath>

int
FS::mkdir(std::string dirpath)
{
    uint16_t tmpBlock = currBlock;
    if(dirpath[0] == '/'){ //handles absolute paths
      dirpath.erase(0, 1);
      currBlock = 0;
    }
    std::vector<std::string> dividedString;
    dividedString = splitStr(dirpath);

    int currIndex;
    int currBlockIndex = currBlock;
    if(dividedString.size() == 1){
        CreateDir(dirpath, currBlock);
    }
    else{
        currBlockIndex = currBlock;
        for(int i = 0; i < dividedString.size() - 1; i++){
          currIndex = FindFile(dividedString[i], currBlockIndex);
          disk.read(currBlockIndex, (uint8_t *) dir_array);
          if(currIndex == -1 || dir_array[currIndex].type == 0){
            std::cout << "Couldn't find path to requested directory!\n";
            currBlock = tmpBlock;
            return 0;
          }
          currBlockIndex = dir_array[currIndex].first_blk;
        }

        CreateDir(dividedString[dividedString.size()-1], currBlockIndex);
    }
    dirpath = "/" + dirpath;
    currBlock = tmpBlock;
    std::cout << "FS::mkdir(" << dirpath << ")\n";
    return 0;
}

// cd <dirpath> changes the current (working) directory to the directory named <dirpath>
int
FS::cd(std::string dirpath)
{
    uint16_t tmpBlock = currBlock;
    std::string tmpPwd = cwd;
    if(dirpath[0] == '/'){ //handles absolute paths
      dirpath.erase(0, 1);
      currBlock = 0;
      cwd = '/';
    }
    if(dirpath == ""){
      dirpath = "/" + dirpath;
      std::cout << "FS::cd(" << dirpath << ")\n";
      return 0;
    }
    std::vector<std::string> firstv = splitStr(dirpath);
    int index = FindFile(firstv[0], currBlock);
    std::string tmpCwd = cwd;
    disk.read(currBlock, (uint8_t *) dir_array);
    if(index != -1 && dir_array[index].type == 1) {// Om dir finns
          std::vector<std::string> folders = splitStr(dirpath);
          std::vector<std::string> path = splitStr(cwd);
          int block = currBlock;
          int index = -1;
          if(cwd == "/"){
            tmpCwd = "";
          }
          for (int i = 0; i < folders.size(); i++){
            index = FindFile(folders[i], block);
            disk.read(block, (uint8_t *) dir_array);
            if (index == -1 || dir_array[index].type == 0){
              std::cout << "Could not find directory\n";
              currBlock = tmpBlock;
              cwd = tmpPwd;
              return 0;
            }
            else{
              if((dir_array[index].file_name == folders[folders.size()-1]) && i == folders.size() - 1){
                if(folders[i] != "..") {
                  currBlock = dir_array[index].first_blk;
                  cwd = tmpCwd + "/" + dir_array[index].file_name;
                }
                else {
                  currBlock = dir_array[index].first_blk;
                  std::vector<std::string> v = splitStr(tmpCwd);
                  tmpCwd = "";

                  for(int j = 1; j < v.size()-1; j++){
                    tmpCwd = tmpCwd + "/" + v[j];
                  }
                  if(tmpCwd == ""){
                    tmpCwd = "/";
                  }
                  cwd = tmpCwd;

                }
              }
              else{
                if(folders[i] != "..") { //preforms the move
                  block = dir_array[index].first_blk;
                  tmpCwd = tmpCwd + "/" + dir_array[index].file_name;
                }
                else {
                  block = dir_array[index].first_blk;
                  std::vector<std::string> v = splitStr(tmpCwd);
                  tmpCwd = "";
                  for(int j = 1; j < v.size()-1; j++){
                    tmpCwd = tmpCwd + "/" + v[j];
                  }
                }
              }
            }
          }
        }

   else {
      std::cout << "Could not find directory" << std::endl;
    }

    dirpath = "/" + dirpath;
    std::cout << "FS::cd(" << dirpath << ")\n";
    return 0;
}

// pwd prints the full path, i.e., from the root directory, to the current
// directory, including the currect directory name
int
FS::pwd()
{
    std::cout << cwd << std::endl; // prints current working directory
    std::cout << "FS::pwd()\n";
    return 0;
}

// chmod <accessrights> <filepath> changes the access rights for the
// file <filepath> to <accessrights>.
int
FS::chmod(std::string accessrights, std::string filepath)
{
    uint16_t tmpBlock = currBlock;
    if(filepath[0] == '/'){ //handles absolute paths
      filepath.erase(0, 1);
      currBlock = 0;
    }
    int index;
    int acc_rights = std::stoi(accessrights); // converts access rights from int to string
    index = FindFile(filepath, currBlock);  // Finds the file, -1 if not found
    disk.read(currBlock, (uint8_t *) dir_array);
    if(index == -1) {
      std::cout << "Coudln't find file\n";
      currBlock = tmpBlock;
      return 0;
    }

    std::vector<std::string> inputVector = splitStr(filepath);

    for(int i = 0; i < inputVector.size()-1; i++){
      index = FindFile(inputVector[i], tmpBlock); // Ger index i dir_array
      disk.read(tmpBlock, (uint8_t *) dir_array);

      if(index == -1) {
        std::cout <<"File does not exists" << std::endl;
        currBlock = tmpBlock;
        return 0;
      }
      tmpBlock = dir_array[index].first_blk; // Slutar på mappen håller filen
    }

    index = FindFile(inputVector[inputVector.size()-1], tmpBlock);
    if(dir_array[index].type != 0 || dir_array[index].type == -1){  // Checks if the given file exists and if it is a file.
      std::cout << "Error: something wring with the given arguments" << std::endl;
      currBlock = tmpBlock;
      return 0;
    }
    else {
      dir_array[index].access_rights = acc_rights;
    }

    disk.write(tmpBlock, (uint8_t *) dir_array);
    currBlock = tmpBlock;
    std::cout << "FS:chmod(" << accessrights << "," << filepath << ")\n";
    return 0;

}
