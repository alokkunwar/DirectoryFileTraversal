//Required header files
#define _XOPEN_SOURCE 500 //This is required for nftw()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ftw.h>
#include <limits.h>

//Declaring global variable and function prototype
char *sourceDirPath;
char *destDirPath;
char *extensionToBeExcluded = NULL;
int moveMode = 0;
int totalFileCount = 0;
int totalDirCount = 0;
off_t totalFileSizeCount = 0;
void printErrorMessage(char *errorMessage);
void exitOnError();
void validateInput(int argc, char *arguments[]);
void validateCPXCmd(int noOfArgs, char *arguments[]);
void validateMVCmd(int noOfArgs, char *arguments[]);
void printMessage(char *message);
void methodToCallCopy(char *sourceDirPath);
void validateNFCmd(int noOfArgs, char *arguments[]);
void validateNDCmd(int noOfArgs, char *arguments[]);
void validateSFCmd(int noOfArgs, char *arguments[]);
void deleteFile(const char *filePath);
int writeToDestinationFile(FILE *sourceObj, FILE *destObj);

//Method to copy the file from source to destination.
void CopyToDestination(const char *sourceFilePath, const char *destFilePath) 
{
    //Opening the source file in read mode.
    FILE *sourceObj = fopen(sourceFilePath, "rb");
    //checking if there is any issue while opening the file.
    if (sourceObj == NULL) 
    {
        printErrorMessage("Error during opening the file at source");
        return;
    }

    // Opening the destination file in write mode
    FILE *destObj = fopen(destFilePath, "w");

    //Checking if there is any issue while opening the file.
    if (destObj == NULL) 
    {
        printErrorMessage("Error during opening the file at destination");
        fclose(sourceObj);
        return;
    }

    //Calling the method to write to the destination file.
    writeToDestinationFile(sourceObj, destObj);

    //closing the file.
    fclose(sourceObj);
    fclose(destObj);
}


// Method to write the content to the destination file.
int writeToDestinationFile(FILE *sourceObj, FILE *destObj)
{
    char temparray[4100];
    size_t dataRead;
    while ((dataRead = fread(temparray, 1, sizeof(temparray), sourceObj)) > 0)
    {
        fwrite(temparray, 1, dataRead, destObj);//writing to the destination file.
    }
}

//Method to create Directory recursivelly
void makeDirectory(const char *filePath) 
{
    char filePathCpy[PATH_MAX];
    char *ptr = NULL;
    size_t len;

    snprintf(filePathCpy, sizeof(filePathCpy), "%s", filePath);
    len = strlen(filePathCpy);

    if (filePathCpy[len - 1] == '/') 
    {
        filePathCpy[len - 1] = 0;
    }

    for (ptr = filePathCpy + 1; *ptr; ptr++) 
    {
        if (*ptr == '/') 
        {
            *ptr = 0;
            mkdir(filePathCpy, 0777);
            *ptr = '/';
        }
    }

    mkdir(filePathCpy, 0777);
}

//Method to move the file from source to destination.
void moveToDestination(const char *sourceFilePath, const char *destFilePath) 
{
    //Renaming it.
    if (rename(sourceFilePath, destFilePath) == -1) 
    {
        printErrorMessage("Error while renaming the file");

        //Copy the file from source to destination.
        CopyToDestination(sourceFilePath, destFilePath);

        //Deleting the source file.
        deleteFile(sourceFilePath);
       
    }
}

//Method to delete the provided file.
void deleteFile(const char *filePath)
{
    if (unlink(filePath) == -1) 
    {
        printErrorMessage("Error while deleting the file.");
    }
}

// Method to check if it is a file or directory.
int isFile(const char *filePath)
{
    //If the file path has dot then its a file else its a directory.
    char *extensionPosition = strchr(filePath, '.');
    if (extensionPosition != NULL)
        return 1;
    return 0;
}

//Method to check if  extension should be excluded or not.
int isExtensionExcluded(size_t lengthOfExtension,  size_t filePathLength, const char *filePath)
{
   return ((filePathLength > lengthOfExtension) && (strcmp(filePathLength + filePath - lengthOfExtension, extensionToBeExcluded) == 0));
}

// Method to exclude a file for the cpx command.
int excludeExtension(const char *filePath)
{
    // get the length of the extension and the length of the file path.
    size_t lengthOfExtension = strlen(extensionToBeExcluded);
    size_t filePathLength = strlen(filePath);
    if(isExtensionExcluded(lengthOfExtension, filePathLength, filePath))
        return 1;// if excluded return 1.
    else
        return 0;
}

// defined the type to store destination path.
typedef struct {
    char destinationPath[PATH_MAX];
} DestinationPath;

//Method to get the destination path.
DestinationPath getDestinationPath(const char *filePath)
{
    DestinationPath path;
    char destinationPath[PATH_MAX];
    //setting the value of destinationPath by destDirPath + filePath + strlen(sourceDirPath)
    snprintf(destinationPath, sizeof(destinationPath), "%s%s", destDirPath, filePath + strlen(sourceDirPath));
    //crreating a copy of destinationPath to make the return type DestinationPath.
    snprintf(path.destinationPath, PATH_MAX, "%s", destinationPath);

    return path;
}

//Method to copy files and directories.
int copyFilesAndDirectories(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {

    //if file
    if(isFile(fpath)) 
    {
        // Condition for the cpx command if a file type needs to be excluded.
        if(extensionToBeExcluded && excludeExtension(fpath)) 
            return 0;
    
        // Getting the destination path by calling the method getDestinationPath.    
        DestinationPath destination = getDestinationPath(fpath);
        //creating a copy of destination var to make the directory.
        char *destinationPathCopy = strdup(destination.destinationPath);

        //removing the file name from the destination path A1/Dir1/Dir2/abc.txt to A1/Dir1/Dir2
        char *lastSlash = strrchr(destinationPathCopy, '/');
        if (lastSlash != NULL)
        *lastSlash = '\0';
        
        // Now destinationPathCopy has directory path now calling this method to create directory.
        makeDirectory(destinationPathCopy);

        // Condition to check for cpx or mv command.
        if (moveMode) 
            moveToDestination(fpath,destination.destinationPath); // if mv then calling method to move to destination.
        else 
            CopyToDestination(fpath, destination.destinationPath);// if cpx then calling method to copy to destination.
    } 
    else if (typeflag == FTW_D) 
    {
        //if directory then create it.
        DestinationPath destination = getDestinationPath(fpath);
        mkdir(destination.destinationPath, 0777);
    }

    return 0;
}

//Method to remove or delete the file.
int removeDirectoryOrFile(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) 
{
    //If directory delete it else if file delete it.
    if (typeflag == FTW_DP || typeflag == FTW_D) 
    {
        if (rmdir(fpath) == -1) 
        {
            printErrorMessage("Error while removing the directory");
        }
    } 
    else 
    {
        deleteFile(fpath);
    }

    return 0;
}

//Main method
int main(int argc, char *argv[]) {

    //Method to validate all the inputs for Assignment 1.
    validateInput(argc, argv);

    //Free source and destination.
    free(sourceDirPath);
    free(destDirPath);

    return 0;
}

//Method to print error message.
void printErrorMessage(char *errorMessage)
{
    perror(errorMessage);
}

//Method to exit.
void exitOnError()
{
     exit(EXIT_FAILURE);
}

//Method to create the directory
void createDirectory(char *pathToCreateDir)
{
    struct stat st = {0};
    if (stat(pathToCreateDir, &st) == -1) 
    {
        if (mkdir(pathToCreateDir, 0777) == -1) 
        {
            printErrorMessage("There is some error in creating the destination directory");
            exitOnError();
        }
    }
}

//Method to validate the input for all the five commands
void validateInput(int noOfArgs, char *arguments[])
{
    //retreving the first argument to distinguish the command
    char *inputCommand =  arguments[1];

    //Condition for NF
    if(strcmp(inputCommand, "-nf") == 0)
    {
        
        validateNFCmd(noOfArgs, arguments);
    
    }
     //Condition for ND
    else if(strcmp(inputCommand, "-nd") == 0)
    {
         validateNDCmd(noOfArgs, arguments);

    }
     //Condition for SF
    else if(strcmp(inputCommand, "-sf") == 0)
    {
         validateSFCmd(noOfArgs, arguments);

    }
     //Condition for CPX
    else if(strcmp(inputCommand, "-cpx") == 0)
    {
        validateCPXCmd(noOfArgs, arguments);

    }
     //Condition for MV
    else if(strcmp(inputCommand, "-mv") == 0)
    {
        validateMVCmd(noOfArgs, arguments);
    }
    else
    {
        printf("Please enter a valid command");
        exitOnError();
    }
}

//Method to print a message to the console.
void printMessage(char *message)
{
    printf(message);
    printf("\n");
}

//Method to validate the path
void validatePath(char *folderPath, char *pathType)
{
    // Creating error message. Here pathType can be a Source or Destination.
    char ErrorMessage[PATH_MAX];
        snprintf(ErrorMessage, sizeof(ErrorMessage), "%s%s", pathType, "path is not valid. Please provide the correct path");

    if (folderPath == NULL) 
    {
        printErrorMessage(ErrorMessage);
        exitOnError();
    }
}

//Method to calculate the tital File count at the given path.
int filesRootedAtDir(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    if (typeflag == FTW_F) 
    {
        totalFileCount++; // if file, increment the count.
    }
    return 0; // return 0 means to tell nftw() to continue
}

//Method to validate the NF command
void validateNFCmd(int noOfArgs, char *arguments[])
{
    //Condition to validate the NF command.
    if(noOfArgs != 3)
    {
        printMessage("Please enter a valid command for NF in -nf [root_dir] format.");
        exitOnError();
    }
    else
    {
        //Calling the method to calculate the number of files in a given directory.
        sourceDirPath = realpath(arguments[2], NULL);
        //Calling method to validate the directory.
        validatePath(sourceDirPath, "Source Directory ");
        if (nftw(sourceDirPath, filesRootedAtDir, 20, FTW_PHYS) == -1) 
        {
            printErrorMessage("There is some issue in the nftw");
            exitOnError();
        }
        printf("Total number of files in the subtree rooted at given dir: %d\n", totalFileCount);

    }
}

//Method to count the number of dirs at the given path.
int countOfSubDirRootedAtDir(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    if (typeflag == FTW_D  && ftwbuf->level > 0) 
    {
        totalDirCount++; // if directory, increment the count.
    }
    return 0; // return 0 means to tell nftw() to continue
}

//Method to validate the ND command.
void validateNDCmd(int noOfArgs, char *arguments[])
{   
    //Condition to validate the ND command.
    if(noOfArgs != 3)
    {
        printMessage("Please enter a valid command for ND in -nd [root_dir] format.");
        exitOnError();
    }
    else
    {
        //Calling the method to calculate the number of sub directories in a given directory.
        sourceDirPath = realpath(arguments[2], NULL);
        //Calling method to validate the directory.
        validatePath(sourceDirPath, "Source Directory ");
        if (nftw(sourceDirPath, countOfSubDirRootedAtDir, 20, FTW_PHYS) == -1) 
        {
            printErrorMessage("There is some issue in the nftw");
            exitOnError();
        }

        printf("Total number of sub directories in the given path are: %d\n", totalDirCount);
    }
}

//Method to calculate the file size in bytes at the given path.
int filesSizeRootedAtDir(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    if (typeflag == FTW_F) {
        totalFileSizeCount += sb->st_size; //If file, then add the respective file size to the total size
    }
    return 0; // return 0 means to tell nftw() to continue
}

//Method to validate the SF command.
void validateSFCmd(int noOfArgs, char *arguments[])
{
    //Condition to validate the SF command.
    if(noOfArgs != 3)
    {
        printMessage("Please enter a valid command for SF in -sf [root_dir] format.");
        exitOnError();
    }
    else
    {
        //Calling the method to calculate the file size in bytes in a given directory. 
        sourceDirPath = realpath(arguments[2], NULL);
        //Calling method to validate the directory.
        validatePath(sourceDirPath, "Source Directory ");
        if (nftw(sourceDirPath, filesSizeRootedAtDir, 20, FTW_PHYS) == -1) 
        {
            printErrorMessage("There is some issue in the nftw");
            exitOnError();
        }

        printf("Total files size in the subtree rooted at given dir: %d bytes\n", totalFileSizeCount);
    }
}

//Method to validate the CPX command.
void validateCPXCmd(int noOfArgs, char *arguments[])
{
    //Condition to validate the CPX command.
    if(noOfArgs < 4 || noOfArgs > 5)
    {
        printMessage("Please enter a valid command for CPX in -cpx [ source_dir] [destination_dir] [file extension] format.");
        exitOnError();
    }
    else
    {
        //Calling the method to copy the entire sub directory to detination directory.
        sourceDirPath = realpath(arguments[2], NULL);
        //creating a copy of destination var to make the directory.
        destDirPath = strdup(arguments[3]);
        //calling method to validate the directory.
        validatePath(sourceDirPath, "Source Directory ");

        //Condition to validate the cpx command if a file type needs to be excluded.
        if (noOfArgs == 5) 
        {
            extensionToBeExcluded = arguments[4];
        }
        //Calling the method to create the directory
        createDirectory(destDirPath);
        methodToCallCopy(sourceDirPath);
        
    }
}

//Method to copy files and directories by calling copyFilesAndDirectories.
void methodToCallCopy(char *sourceDirPath)
{
    if (nftw(sourceDirPath, copyFilesAndDirectories, 64, FTW_PHYS) == -1) 
    {
        printErrorMessage("There is some issue in the nftw");
        exitOnError();
    }
}

//Method to validate the MV command.
void validateMVCmd(int noOfArgs,  char *arguments[])
{
    //Condition to validate the MV command.
    if(noOfArgs != 4)
    {
        printMessage("Please enter a valid command for mv in -mv [ source_dir] [destination_dir] format.");
        exitOnError();
    }
    else
    {
       //Calling the method to move the entire sub directory to destination directory.
       sourceDirPath = realpath(arguments[2], NULL);
       //creating a copy of destination var to make the directory.
       destDirPath = strdup(arguments[3]);
       //calling method to validate the directory.
       validatePath(sourceDirPath, "Source Directory "); 

       //Setting the moveMode 1
       moveMode = 1;
       //Calling the method to create the directory
       createDirectory(destDirPath);
       methodToCallCopy(sourceDirPath);
       if (nftw(sourceDirPath, removeDirectoryOrFile, 64, FTW_DEPTH | FTW_PHYS) == -1) 
       {
            printErrorMessage("There is some issue in the nftw");
            exitOnError();
       }
    }
}
