#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//Determines the max length of the command that could be output to gcc
#define MAX_COMMAND_LEN 4500
//Determines the maximum number of elements per line
#define SRC_INDICES_LEN 100

typedef struct BuildInfo {
    char** srcFiles;
    char** includeFolders;
    char** libFolders;
    char** libs;
    char** args;
    char** out;
    char** compiler;
} BuildInfo;

//Compile an executable named build with the command below:
// gcc buildsys.c -o build

//Currently the build command system only supports gcc, but cl.exe support may be added at a later date

//For profile, the user can specify a named profile if they want, and if they don't profile can be NULL, telling
//the function to simply look for the first profile it finds
void getBuildInfo(char* profile, char* file, BuildInfo* build_info);

bool strFindReplace(char** src, char* find, char* replace);

void prepFileString(char** str);

// The offset is for if you want to see if there is another index of the find string after the first had been found
int getIndexOf(char* src, char* find, int offset);

int getLineInfo(char*** twoDArray, char* identifier, char* file, int offset);

int main(int argc, char* argv[]) {
    char* fstring = NULL;
    prepFileString(&fstring);

    BuildInfo build_info;
    build_info.srcFiles = NULL;
    build_info.includeFolders = NULL;
    build_info.libFolders = NULL;
    build_info.libs = NULL;
    build_info.out = NULL;
    build_info.compiler = NULL;
    build_info.args = NULL;

    if (argc > 1) {
        getBuildInfo(argv[1], fstring, &build_info);
    } else {
        getBuildInfo(NULL, fstring, &build_info);
    }
    free(fstring);

    int commandCount = 0;
    char* command = (char*)malloc((MAX_COMMAND_LEN + 1) * sizeof(char));
    if (command == NULL) {
        printf("Failed to allocate memeory for command\n");
        exit(-1);
    }

    //Add compiler first
    int compiler_len = strlen(build_info.compiler[0]);
    if (compiler_len + 1 < MAX_COMMAND_LEN) {
        memcpy(command, build_info.compiler[0], compiler_len);
        commandCount += compiler_len;
        command[commandCount] = ' ';
        commandCount++;
    } else {
        printf("Maximum command length exceeded.\n");
        exit(-1);
    }

    //Next, add the source files
    for (int i = 0; build_info.srcFiles[i] != NULL; i++) {
        int src_len = strlen(build_info.srcFiles[i]);
        if (commandCount + src_len + 1 < MAX_COMMAND_LEN) {
            memcpy(command + commandCount, build_info.srcFiles[i], src_len);
            commandCount += src_len;
            command[commandCount] = ' ';
            commandCount++;
        } else {
            printf("Maximum command length exceeded.\n");
            exit(-1);            
        }
    }

    //Next, add include Folders, if they exist
    if (build_info.includeFolders != NULL) {
        char* includeArg = "-I";
        int includeArgLen = strlen(includeArg);

        for (int i = 0; build_info.includeFolders[i] != NULL; i++) {
            int includeFolderLen = strlen(build_info.includeFolders[i]);
            if (commandCount + includeFolderLen + 1 + includeArgLen < MAX_COMMAND_LEN) {
                memcpy(command + commandCount, includeArg, includeArgLen);
                commandCount += includeArgLen;

                memcpy(command + commandCount, build_info.includeFolders[i], includeFolderLen);
                commandCount += includeFolderLen;

                command[commandCount] = ' ';
                commandCount++;
            } else {
                printf("Maximum command length exceeded.\n");
                exit(-1);
            }
        }
    }

    //Next, add the lib folders, if they exist
    if (build_info.libFolders != NULL) {
        char* libFolderArg = "-L";
        int libFolderArgLen = strlen(libFolderArg);

        for (int i = 0; build_info.libFolders[i] != NULL; i++) {
            int libFolderLen = strlen(build_info.libFolders[i]);
            if (commandCount + libFolderLen + 1 + libFolderArgLen < MAX_COMMAND_LEN) {
                memcpy(command + commandCount, libFolderArg, libFolderArgLen);
                commandCount += libFolderArgLen;

                memcpy(command + commandCount, build_info.libFolders[i], libFolderLen);
                commandCount += libFolderLen;

                command[commandCount] = ' ';
                commandCount++;
            } else {
                printf("Maximum command length exceeded.\n");
                exit(-1);
            }
        }
    }

    //Next, add the libraries, if they exist
    if (build_info.libs != NULL) {
        char* libArg = "-l:";
        int libArgLen = strlen(libArg);

        for (int i = 0; build_info.libs[i] != NULL; i++) {
            int libLen = strlen(build_info.libs[i]);
            if (commandCount + libLen + 1 + libArgLen < MAX_COMMAND_LEN) {
                memcpy(command + commandCount, libArg, libArgLen);
                commandCount += libArgLen;

                memcpy(command + commandCount, build_info.libs[i], libLen);
                commandCount += libLen;

                command[commandCount] = ' ';
                commandCount++;
            } else {
                printf("Maximum command length exceeded.\n");
                exit(-1);
            }
        }
    }

    //Next, add the arguments, if they exist
    if (build_info.args != NULL) {
        for (int i = 0; build_info.args[i] != NULL; i++) {
            int argsLen = strlen(build_info.args[i]);
            if (commandCount + argsLen + 1 < MAX_COMMAND_LEN) {
                memcpy(command + commandCount, build_info.args[i], argsLen);
                commandCount += argsLen;
                command[commandCount] = ' ';
                commandCount++;
            } else {
                printf("Maximum command length exceeded.\n");
                exit(-1);
            }
        }
    }

    //Finally, add the output file
    char* outputArg = "-o ";
    int outputArgLen = strlen(outputArg);
    int outLen = strlen(build_info.out[0]);
    if (commandCount + outputArgLen + outLen + 1 < MAX_COMMAND_LEN) {
        memcpy(command + commandCount, outputArg, outputArgLen);
        commandCount += outputArgLen;
        memcpy(command + commandCount, build_info.out[0], outLen);
        commandCount += outLen;
        command[commandCount] = '\0';
    }

    printf("%s\n", command);
    system(command);

    return 0;
}

void prepFileString(char** str) {
    FILE* fptr = fopen("build.set", "rb");
    if (fptr == NULL) {
        printf("There was an error opening the file build.set\nMake sure that the file exists with that specific name\nand that it is in the same directory as build.exe\n");
        exit(-1);
    }
    fseek(fptr, 0L, SEEK_END);
    int fsize = ftell(fptr) / sizeof(char);
    fseek(fptr, 0L, SEEK_SET);

    char* fbuffer = (char*)malloc((fsize+1) * sizeof(char));
    if (fbuffer == NULL) {
        printf("Failed to allocate memory for string buffer\n");
        exit(-1);
    }

    fread(fbuffer, sizeof(char), fsize, fptr);
    fbuffer[fsize] = '\0';
    fclose(fptr);

    //Clean up the file string by getting rid of spaces and carriage return characters
    while(strFindReplace(&fbuffer, " ", ""));
    while(strFindReplace(&fbuffer, "\r", ""));

    *str = fbuffer;
}

int getIndexOf(char* src, char* find, int offset) {
    int srcLen = strlen(src);
    int findLen = strlen(find);

    for (int i = offset; i < srcLen - findLen; i++) {
        int count = 0;
        for (int j = 0; j < findLen; j++) {
            if (src[i + j] == find[j]) {
                count++;
            } else {
                break;
            }
        }

        if (count == findLen) {
            return i;
        }
    }

    //-1 indicates that the string find was not found in the src string
    return -1;
}

//Syntax:
//profile : m1 = {
//    src = main.c, glad.c
//    include = C:\Users\colec\..
//    libFolder = C:\Users\colec\..
//    libs = libglfw3.a, libgdi32.a
//    args = -Werror
//    out = main
//}
void getBuildInfo(char* profile, char* file, BuildInfo* build_info) {
    int offset;
    if (profile == NULL) {
        offset = 0;
    } else {
        char* str1 = "profile:";
        int str1Len = strlen(str1);
        int profileLen = strlen(profile);

        char* profileSearch = (char*)malloc((str1Len + profileLen + 4) * sizeof(char));
        if (profileSearch == NULL) {
            printf("Failed to allocate memory for profileSearch variable\n");
            exit(-1);
        }
        memcpy(profileSearch, str1, str1Len);
        memcpy(profileSearch + str1Len, profile, profileLen);
        profileSearch[str1Len + profileLen] = '=';
        profileSearch[str1Len + profileLen + 1] = '{';
        profileSearch[str1Len + profileLen + 2] = '\0';

        offset = getIndexOf(file, profileSearch, 0);
        if (offset == -1) {
            printf("Could not find profile named %s. Check spelling to verify what was typed into the command line.\n", profile);
            exit(-1);
        }
        free(profileSearch);
    }

    if (getLineInfo(&build_info->srcFiles, "src=", file, offset) == -1) {
        printf("Failed to find any source files. Please include 'src=' with at least one file specified.\n");
        exit(-1);
    }
    getLineInfo(&build_info->includeFolders, "include=", file, offset);
    getLineInfo(&build_info->libFolders, "libFolder=", file, offset);
    getLineInfo(&build_info->libs, "libs=", file, offset);
    getLineInfo(&build_info->args, "args=", file, offset);
    if (getLineInfo(&build_info->compiler, "compiler=", file, offset) == -1) {
        printf("Failed to find any compiler. Please specify compiler path with 'compiler='.\n");
        exit(-1);
    }
    if (getLineInfo(&build_info->out, "out=", file, offset)) {
        printf("Failed to find any output file destination. Please specify output file with 'out='.\n");
        exit(-1);
    }
}

//This assumes null terminated strings
bool strFindReplace(char** src, char* find, char* replace) {
    int srcLen = strlen(*src);
    int findLen = strlen(find);
    int replaceLen = strlen(replace);

    for (int i = 0; i < srcLen-findLen; i++) {
        int count = 0;
        for (int j = 0; j < findLen; j++) {
            if (i+j < srcLen) {
                if ((*src)[i+j] == find[j]) {
                    count++;
                } else {
                    break;
                } 
            }
        }

        if (count == findLen) {
            if (findLen == replaceLen) {
                memcpy(*src + i, replace, replaceLen);
                return true;
            } else if (replaceLen > findLen) {
                (*src) = (char*)realloc((*src), (srcLen - findLen + replaceLen + 1) * sizeof(char));
                if ((*src) == NULL) {
                    printf("Failed to allocate memory in strFindReplace\n");
                    exit(-1);
                }

                char* tempstr2 = (char*)malloc((srcLen - i - findLen + 1) * sizeof(char));
                memcpy(tempstr2, *src + i + findLen, srcLen - i - findLen);

                memcpy(*src + i, replace, replaceLen);
                srcLen = srcLen - findLen + replaceLen;
                memcpy(*src + i + replaceLen, tempstr2, srcLen - i - replaceLen);

                free(tempstr2);
                (*src)[srcLen] = '\0';
                return true;
            } else if (replaceLen < findLen) {
                char* tempstr2 = (char*)malloc((srcLen - i - findLen + 1) * sizeof(char));
                memcpy(tempstr2, *src + i + findLen, srcLen - i - findLen);

                memcpy(*src + i, replace, replaceLen);
                memcpy(*src + i + replaceLen, tempstr2, srcLen - i - findLen);
                free(tempstr2);
                (*src)[srcLen - findLen + replaceLen] = '\0';
                return true;
            }
        }
    }

    return false;
}

int getLineInfo(char*** twoDArray, char* identifier, char* file, int offset) {
    int identifierLen = strlen(identifier);

    int closedCurlyIndex = getIndexOf(file, "}", offset);
    if (closedCurlyIndex == -1) {
        printf("Failed to find closing curly brace (}). Please make sure all profiles are enclosed by curly braces\n");
        exit(-1);
    }

    int srcFileIndex = getIndexOf(file, identifier, offset);
    if (srcFileIndex == -1) {
        //This will be used specifically to ensure that certain key elements are included, such as specifying files to be compiled
        return -1;
    } else if (srcFileIndex > closedCurlyIndex) {
        return -1;
    }
    int srcCount = 0;
    int srcIndices[SRC_INDICES_LEN];
    for (int i = srcFileIndex+identifierLen; i < closedCurlyIndex + 1; i++) {
        if (file[i] == ',' || file[i] == '\n' || file[i] == '}') {
            if (srcCount >= SRC_INDICES_LEN) {
                printf("Program is unable to handle more than %d elements in one compilation.\n", SRC_INDICES_LEN);
                exit(-1);
            }
            srcIndices[srcCount] = i;
            srcCount++;
            //Make sure the loop stops once a newline is reached or an ending curly brace in order to prevent it from
            //going onto lines other the one that defines the src files
            if (file[i] == '\n' || file[i] == '}') {
                break;
            }
        }
    }
    if (srcCount == 0) {
        printf("Found '%s', but no files or paths were specified. Please enter at least one file or path to be compiled.\n", identifier);
        exit(-1);
    }
    *twoDArray = (char**)malloc((srcCount+1) * sizeof(char*));
    (*twoDArray)[srcCount] = NULL;
    if (*twoDArray == NULL) {
        printf("Failed to allocate memory for source files 2D array. (location 1)\n");
        exit(-1);
    }
    for (int i = 0; i < srcCount; i++) {
        if (i == 0) {
            int strLen = srcIndices[0] - (srcFileIndex + identifierLen);
            (*twoDArray)[0] = (char*)malloc((strLen + 1) * sizeof(char));
            if ((*twoDArray)[0] == NULL) {
                printf("Failed to allocate memory for source files 2D array. (location 2)\n");
                exit(-1);
            }
            for (int j = srcFileIndex+identifierLen; j < srcIndices[0]; j++) {
                (*twoDArray)[0][j - srcFileIndex - identifierLen] = file[j];
            }
            (*twoDArray)[0][strLen] = '\0';
        } else {
            int strLen = srcIndices[i] - (srcIndices[i-1] + 1);
            (*twoDArray)[i] = (char*)malloc((strLen + 1) * sizeof(char));
            if ((*twoDArray)[i] == NULL) {
                printf("Failed to allocate memory for source files 2D array. (location 3)\n");
                exit(-1);
            }
            for (int j = srcIndices[i-1]+1; j < srcIndices[i]; j++) {
                (*twoDArray)[i][j - srcIndices[i-1] -1] = file[j];
            }
            (*twoDArray)[i][strLen] = '\0';
        }
    }

    return 0;
}