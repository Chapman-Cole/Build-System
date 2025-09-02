#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Determines the max length of the command that could be output to gcc
#define MAX_COMMAND_LEN 4500
// Determines the maximum number of elements per line
#define SRC_INDICES_LEN 100

// Used inside the BuildInfo struct for determining how compiler formatting should be setup
// IMPORTANT: MAKE SURE THAT THE NUMBER DEFINED FOR EACH FORMAT CORRESPONDS TO ITS INDEX IN THE
// compilerFormatIdentifiers ARRAY
#define COMPILER_FORMAT_GCC_CLANG 0
#define COMPILER_FORMAT_MSVC 1

const char* compilerFormatIdentifiers[] = {
    "gcc/clang",
    "msvc"};

#define COMPILER_FORMAT_IDENTIFIERS_LEN sizeof(compilerFormatIdentifiers) / sizeof(compilerFormatIdentifiers[0])

typedef struct BuildInfo {
    char** srcFiles;
    char** includeFolders;
    char** libFolders;
    char** libs;
    char** args;
    char** out;
    char** compilerPath;
    unsigned int compilerFormat;
} BuildInfo;

// Compile an executable named build with the command below:
//  gcc buildsys.c -o build

// Currently the build command system only supports gcc, but cl.exe support may
// be added at a later date

// For profile, the user can specify a named profile if they want, and if they
// don't profile can be NULL, telling the function to simply look for the first
// profile it finds
void getBuildInfo(char* profile, char* file, BuildInfo* build_info);

bool strFindReplace(char** src, char* find, char* replace);

void prepFileString(char** str);

// The offset is for if you want to see if there is another index of the find
// string after the first had been found
int getIndexOf(char* src, char* find, int offset);

int getLineInfo(char*** twoDArray, char* identifier, char* file, int offset);

int getCompilerFormatType(char* file, int offset);

int compilerOutputGccClang(BuildInfo* build_info, bool printOnly);

int main(int argc, char* argv[]) {
    char* fstring = NULL;
    prepFileString(&fstring);

    BuildInfo build_info;
    build_info.srcFiles = NULL;
    build_info.includeFolders = NULL;
    build_info.libFolders = NULL;
    build_info.libs = NULL;
    build_info.out = NULL;
    build_info.compilerPath = NULL;
    build_info.args = NULL;

    // If -generateCommand is passed into build.exe as the first argument, it will only generate the command
    // and not invoke the compiler
    bool printOnly = false;
    if (argc > 1) {
        if (strcmp(argv[1], "-printOnly") == 0) {
            printOnly = true;
            if (argc > 2) {
                getBuildInfo(argv[2], fstring, &build_info);
            } else {
                getBuildInfo(NULL, fstring, &build_info);
            }
        } else {
            getBuildInfo(argv[1], fstring, &build_info);
        }
    } else {
        getBuildInfo(NULL, fstring, &build_info);
    }
    free(fstring);

    switch (build_info.compilerFormat) {
        case COMPILER_FORMAT_GCC_CLANG:
            compilerOutputGccClang(&build_info, printOnly);
            break;
        case COMPILER_FORMAT_MSVC:
            printf("Not completed yet.\n");
            break;
        default:
            printf("Compiler Format was not properly specified.\n");
            return -1;
    }

    return 0;
}

void prepFileString(char** str) {
    FILE* fptr = fopen("build.set", "rb");
    if (fptr == NULL) {
        printf(
            "There was an error opening the file build.set\nMake sure that the "
            "file exists with that specific name\nand that it is in the same "
            "directory as build.exe\n");
        exit(-1);
    }
    fseek(fptr, 0L, SEEK_END);
    int fsize = ftell(fptr) / sizeof(char);
    fseek(fptr, 0L, SEEK_SET);

    char* fbuffer = (char*)malloc((fsize + 1) * sizeof(char));
    if (fbuffer == NULL) {
        printf("Failed to allocate memory for string buffer\n");
        exit(-1);
    }

    fread(fbuffer, sizeof(char), fsize, fptr);
    fbuffer[fsize] = '\0';
    fclose(fptr);

    // Clean up the file string by getting rid of spaces and carriage return
    // characters
    while (strFindReplace(&fbuffer, " ", ""));
    while (strFindReplace(&fbuffer, "\r", ""));
    while (strFindReplace(&fbuffer, "\n", ""));

    *str = fbuffer;
}

int getIndexOf(char* src, char* find, int offset) {
    int srcLen = strlen(src);
    int findLen = strlen(find);

    // The +1 in srcLen - findLen + 1 is necessary because srcLen - findLen would go up to src minus the number of characters in
    // in findLen, but this means that if the string to be found is at the very end of the src string, then it will not be checked properly
    // Example: src = conscientious, find = us
    //          srcLen = 13, findLen = 2
    //          srcLen - findLen = 11
    //          conscientio | us
    //               11       2
    //  As shown in the example above, srcLen - findLen would result in i only ever getting to the character 'o', and then
    //  in the j loop it would check at index i and at index i + 1, and then stop, which would completely miss the us that is indeed in the string
    // Proper Way:
    //       srcLen - findLen + 1 = 12
    //       conscientiou | s
    //            12        1
    //  In the proper way i will reach the 'u' character, and then compare at indices i and i + 1, correctly detecting the 'us'
    //  in the source string
    //  ** Put simply, we need to make it so that i has the potential to reach the first character in the find string if it is present
    //     present in the source string. **
    for (int i = offset; i < srcLen - findLen + 1; i++) {
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

// Syntax:
// profile : m1 = {
//     src = main.c, glad.c
//     include = C:\Users\colec\..
//     libFolder = C:\Users\colec\..
//     libs = libglfw3.a, libgdi32.a
//     args = -Werror
//     out = main
// }
void getBuildInfo(char* profile, char* file, BuildInfo* build_info) {
    int offset;
    if (profile == NULL) {
        offset = 0;
    } else {
        char* str1 = "profile:";
        int str1Len = strlen(str1);
        int profileLen = strlen(profile);

        char* profileSearch =
            (char*)malloc((str1Len + profileLen + 4) * sizeof(char));
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
            printf(
                "Could not find profile named %s. Check spelling to verify "
                "what was typed into the command line.\n",
                profile);
            exit(-1);
        }
        free(profileSearch);
    }

    if (getLineInfo(&build_info->srcFiles, "src=[", file, offset) == -1) {
        printf(
            "Failed to find any source files. Please include 'src=[' with at "
            "least one file specified.\n");
        exit(-1);
    }
    if (getLineInfo(&build_info->compilerPath, "compilerPath=[", file, offset) == -1) {
        printf(
            "Failed to find a compiler path. Please specify compiler path with "
            "'compilerPath=['.\n");
        exit(-1);
    }
    if (getLineInfo(&build_info->out, "out=[", file, offset) == -1) {
        printf(
            "Failed to find any output file destination. Please specify output "
            "file with 'out=['.\n");
        exit(-1);
    }
    if ((build_info->compilerFormat = getCompilerFormatType(file, offset)) == -1) {
        printf("Failed to find any information about the compiler format type.\nThe available options are:\n");
        for (int i = 0; i < COMPILER_FORMAT_IDENTIFIERS_LEN; i++) {
            printf("compilerFormat=[%s]\n", compilerFormatIdentifiers[i]);
        }
        exit(-1);
    }
    getLineInfo(&build_info->includeFolders, "include=[", file, offset);
    getLineInfo(&build_info->libFolders, "libFolder=[", file, offset);
    getLineInfo(&build_info->libs, "libs=[", file, offset);
    getLineInfo(&build_info->args, "args=[", file, offset);
}

// This assumes null terminated strings
bool strFindReplace(char** src, char* find, char* replace) {
    int srcLen = strlen(*src);
    int findLen = strlen(find);
    int replaceLen = strlen(replace);

    for (int i = 0; i < srcLen - findLen; i++) {
        int count = 0;
        for (int j = 0; j < findLen; j++) {
            if (i + j < srcLen) {
                if ((*src)[i + j] == find[j]) {
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
                (*src) = (char*)realloc(
                    (*src), (srcLen - findLen + replaceLen + 1) * sizeof(char));
                if ((*src) == NULL) {
                    printf("Failed to allocate memory in strFindReplace\n");
                    exit(-1);
                }

                char* tempstr2 =
                    (char*)malloc((srcLen - i - findLen + 1) * sizeof(char));
                memcpy(tempstr2, *src + i + findLen, srcLen - i - findLen);

                memcpy(*src + i, replace, replaceLen);
                srcLen = srcLen - findLen + replaceLen;
                memcpy(*src + i + replaceLen, tempstr2,
                       srcLen - i - replaceLen);

                free(tempstr2);
                (*src)[srcLen] = '\0';
                return true;
            } else if (replaceLen < findLen) {
                char* tempstr2 =
                    (char*)malloc((srcLen - i - findLen + 1) * sizeof(char));
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
        printf(
            "Failed to find closing curly brace '}'. Please make sure all "
            "profiles are enclosed by curly braces\n");
        exit(-1);
    }

    int srcFileIndex = getIndexOf(file, identifier, offset);
    if (srcFileIndex == -1) {
        // This will be used specifically to ensure that certain key elements
        // are included, such as specifying files to be compiled
        return -1;
    } else if (srcFileIndex > closedCurlyIndex) {
        return -1;
    }

    int closingBracketIndex = getIndexOf(file, "]", srcFileIndex + identifierLen);
    if (closingBracketIndex == -1 || closingBracketIndex > closedCurlyIndex) {
        printf("Failed to find closing bracket ']' for '%s'. Please make sure data is properly enclosed.\n", identifier);
        exit(-1);
    }

    int srcCount = 0;
    int srcIndices[SRC_INDICES_LEN];
    // i < closingBracketIndex + 1 is so that i will be able to reach the value of the closing bracket to figure out the
    // interval for the final string between ',' and ']' when parsing the build.set file
    for (int i = srcFileIndex + identifierLen; i < closingBracketIndex + 1 && i < closedCurlyIndex; i++) {
        if (file[i] == ',' || file[i] == ']') {
            if (srcCount >= SRC_INDICES_LEN) {
                printf(
                    "Program is unable to handle more than %d elements in one "
                    "compilation.\n",
                    SRC_INDICES_LEN);
                exit(-1);
            }
            srcIndices[srcCount] = i;
            srcCount++;
            // Make sure the loop stops once a newline is reached or an ending
            // curly brace in order to prevent it from going onto lines other
            // the one that defines the src files
            if (file[i] == '\n' || file[i] == '}') {
                break;
            }
        }
    }
    if (srcCount == 1 && file[srcFileIndex + identifierLen] == ']') {
        printf("The identifier '%s' was found, but no files or paths were specified inside the brackets.\n", identifier);
        exit(-1);
    }
    *twoDArray = (char**)malloc((srcCount + 1) * sizeof(char*));
    (*twoDArray)[srcCount] = NULL;
    if (*twoDArray == NULL) {
        printf(
            "Failed to allocate memory for source files 2D array. (location "
            "1)\n");
        exit(-1);
    }
    for (int i = 0; i < srcCount; i++) {
        if (i == 0) {
            int strLen = srcIndices[0] - (srcFileIndex + identifierLen);
            (*twoDArray)[0] = (char*)malloc((strLen + 1) * sizeof(char));
            if ((*twoDArray)[0] == NULL) {
                printf(
                    "Failed to allocate memory for source files 2D array. "
                    "(location 2)\n");
                exit(-1);
            }
            for (int j = srcFileIndex + identifierLen; j < srcIndices[0]; j++) {
                (*twoDArray)[0][j - srcFileIndex - identifierLen] = file[j];
            }
            (*twoDArray)[0][strLen] = '\0';
        } else {
            int strLen = srcIndices[i] - (srcIndices[i - 1] + 1);
            (*twoDArray)[i] = (char*)malloc((strLen + 1) * sizeof(char));
            if ((*twoDArray)[i] == NULL) {
                printf(
                    "Failed to allocate memory for source files 2D array. "
                    "(location 3)\n");
                exit(-1);
            }
            for (int j = srcIndices[i - 1] + 1; j < srcIndices[i]; j++) {
                (*twoDArray)[i][j - srcIndices[i - 1] - 1] = file[j];
            }
            (*twoDArray)[i][strLen] = '\0';
        }
    }

    return 0;
}

int compilerOutputGccClang(BuildInfo* build_info, bool printOnly) {
    int commandCount = 0;
    char* command = (char*)malloc((MAX_COMMAND_LEN + 1) * sizeof(char));
    if (command == NULL) {
        printf("Failed to allocate memeory for command\n");
        exit(-1);
    }

    // Add compiler first
    int compiler_len = strlen(build_info->compilerPath[0]);
    if (compiler_len + 1 < MAX_COMMAND_LEN) {
        memcpy(command, build_info->compilerPath[0], compiler_len);
        commandCount += compiler_len;
        command[commandCount] = ' ';
        commandCount++;
    } else {
        printf("Maximum command length exceeded.\n");
        exit(-1);
    }

    // Next, add the source files
    for (int i = 0; build_info->srcFiles[i] != NULL; i++) {
        int src_len = strlen(build_info->srcFiles[i]);
        if (commandCount + src_len + 1 < MAX_COMMAND_LEN) {
            memcpy(command + commandCount, build_info->srcFiles[i], src_len);
            commandCount += src_len;
            command[commandCount] = ' ';
            commandCount++;
        } else {
            printf("Maximum command length exceeded.\n");
            exit(-1);
        }
    }

    // Next, add include Folders, if they exist
    if (build_info->includeFolders != NULL) {
        char* includeArg = "-I";
        int includeArgLen = strlen(includeArg);

        for (int i = 0; build_info->includeFolders[i] != NULL; i++) {
            int includeFolderLen = strlen(build_info->includeFolders[i]);
            if (commandCount + includeFolderLen + 1 + includeArgLen <
                MAX_COMMAND_LEN) {
                memcpy(command + commandCount, includeArg, includeArgLen);
                commandCount += includeArgLen;

                memcpy(command + commandCount, build_info->includeFolders[i],
                       includeFolderLen);
                commandCount += includeFolderLen;

                command[commandCount] = ' ';
                commandCount++;
            } else {
                printf("Maximum command length exceeded.\n");
                exit(-1);
            }
        }
    }

    // Next, add the lib folders, if they exist
    if (build_info->libFolders != NULL) {
        char* libFolderArg = "-L";
        int libFolderArgLen = strlen(libFolderArg);

        for (int i = 0; build_info->libFolders[i] != NULL; i++) {
            int libFolderLen = strlen(build_info->libFolders[i]);
            if (commandCount + libFolderLen + 1 + libFolderArgLen <
                MAX_COMMAND_LEN) {
                memcpy(command + commandCount, libFolderArg, libFolderArgLen);
                commandCount += libFolderArgLen;

                memcpy(command + commandCount, build_info->libFolders[i],
                       libFolderLen);
                commandCount += libFolderLen;

                command[commandCount] = ' ';
                commandCount++;
            } else {
                printf("Maximum command length exceeded.\n");
                exit(-1);
            }
        }
    }

    // Next, add the libraries, if they exist
    if (build_info->libs != NULL) {
        char* libArg = "-l:";
        int libArgLen = strlen(libArg);

        for (int i = 0; build_info->libs[i] != NULL; i++) {
            int libLen = strlen(build_info->libs[i]);
            if (commandCount + libLen + 1 + libArgLen < MAX_COMMAND_LEN) {
                memcpy(command + commandCount, libArg, libArgLen);
                commandCount += libArgLen;

                memcpy(command + commandCount, build_info->libs[i], libLen);
                commandCount += libLen;

                command[commandCount] = ' ';
                commandCount++;
            } else {
                printf("Maximum command length exceeded.\n");
                exit(-1);
            }
        }
    }

    // Next, add the arguments, if they exist
    if (build_info->args != NULL) {
        for (int i = 0; build_info->args[i] != NULL; i++) {
            int argsLen = strlen(build_info->args[i]);
            if (commandCount + argsLen + 1 < MAX_COMMAND_LEN) {
                memcpy(command + commandCount, build_info->args[i], argsLen);
                commandCount += argsLen;
                command[commandCount] = ' ';
                commandCount++;
            } else {
                printf("Maximum command length exceeded.\n");
                exit(-1);
            }
        }
    }

    // Finally, add the output file
    char* outputArg = "-o ";
    int outputArgLen = strlen(outputArg);
    int outLen = strlen(build_info->out[0]);
    if (commandCount + outputArgLen + outLen + 1 < MAX_COMMAND_LEN) {
        memcpy(command + commandCount, outputArg, outputArgLen);
        commandCount += outputArgLen;
        memcpy(command + commandCount, build_info->out[0], outLen);
        commandCount += outLen;
        command[commandCount] = '\0';
    }

    printf("%s\n", command);

    if (printOnly == false) {
        system(command);
    }
}

int getCompilerFormatType(char* file, int offset) {
    char* identifier = "compilerFormat=[";
    int identifierLen = strlen(identifier);

    int closedCurlyIndex = getIndexOf(file, "}", offset);
    if (closedCurlyIndex == -1) {
        printf(
            "Failed to find closing curly brace '}'. Please make sure all "
            "profiles are enclosed by curly braces\n");
        exit(-1);
    }

    int srcFileIndex = getIndexOf(file, identifier, offset);
    if (srcFileIndex == -1) {
        // This will be used specifically to ensure that certain key elements
        // are included, such as specifying files to be compiled
        return -1;
    } else if (srcFileIndex > closedCurlyIndex) {
        return -1;
    }

    int closingBracketIndex = getIndexOf(file, "]", srcFileIndex + identifierLen);
    if (closingBracketIndex == -1 || closingBracketIndex > closedCurlyIndex) {
        printf("Failed to find closing bracket ']' for '%s'. Please make sure data is properly enclosed.\n", identifier);
        exit(-1);
    }

    if (closingBracketIndex - (srcFileIndex + identifierLen) == 0) {
        printf("Found '%s', but no format was specified.\nAvailable formats are:\n", identifier);
        for (int i = 0; i < COMPILER_FORMAT_IDENTIFIERS_LEN; i++) {
            printf("compilerFormat=[%s]\n", compilerFormatIdentifiers[i]);
        }
        exit(-1);
    }

    for (int i = 0; i < COMPILER_FORMAT_IDENTIFIERS_LEN; i++) {
        if (strncmp(file + srcFileIndex + identifierLen, compilerFormatIdentifiers[i], closingBracketIndex - (srcFileIndex + identifierLen)) == 0) {
            return i;
        }
    }

    return -1;
}