/*
Duncan Van Keulen
Professor Norman
CS232 - Operating Systems
Homework 2: Copy Program
2/22/2021
*/

// Initial command line argument structure from...
// https://www.geeksforgeeks.org/command-line-arguments-in-c-cpp/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>    // access()
#include <sys/types.h> //path_stat
#include <sys/stat.h>  //path_stat

// Check if a file is regular
// Thanks to Frédéric Hamidi on StackOverflow for this
// https://stackoverflow.com/questions/4553012/checking-if-a-file-is-a-directory-or-just-a-file
int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

int main(int argc, char *argv[])
{
    // Just the program name
    if (argc == 1)
    {
        perror("Please provide a source and destination file");
        return -1;
    }

    // Only source file was passed
    if (argc == 2)
    {
        perror("Please provide a destination file to copy to");
        return -1;
    }
    // 3 arguments were passed ([1], [2], are src & dest)
    if (argc == 3)
    {
        // Check if the file is regular
        if (!is_regular_file(argv[1]))
        {
            perror("Source is not a regular file!");
            return -1;
        }
        // Src file does not exist
        else if (fopen(argv[1], "r") == NULL)
        {
            perror("Src file does not exist");
            return -1;
        }
        // Destination file already exists
        else if (fopen(argv[2], "r") != NULL)
        {
            perror("Destination file already exists");
            return -1;
        }
        // User does not have permission to access/read the src file
        else if (access(argv[1], R_OK) < 0)
        {
            perror("You do not have access to the src file");
            return -1;
        }
        // Good to go, start copying
        else 
        {
            FILE* srcPtr;
            FILE* destPtr;

            srcPtr = fopen(argv[1], "r");
            destPtr = fopen(argv[2], "w");

            // Copy each character into the new file.
            // Code based off of these examples
            // https://www.geeksforgeeks.org/fgetc-fputc-c/
            do
            {
                // Grab one character from the src file
                char c = fgetc(srcPtr);

                // Check for end of file
                if (feof(srcPtr))
                    break;

                // Put the character in the new dest file
                putc(c, destPtr);

            } while(1);

            // Close the files
            if (destPtr != NULL && srcPtr != NULL)
            {
                fclose(srcPtr);
                fclose(destPtr);
            }

            // File copied successfully
            return 0;
        }
    }
    // > 3 arguments provided
    else
    {
        perror("Invalid number of arguments");
        return -1;
    }
    return 0;
}
