//
//  path.cpp
//  CS232 Command Shell
//
//  Created by Duncan Van Keulen on 1/28/2021
//

#include "path.h"

Path::Path() 
{
    // get the PATH
    char* pathC = getenv("PATH");
    // convert it to a string
    string pathStr = string(pathC);

    // split the PATH variable into individual paths in the path vector
    // based on roach's answer in this stackoverflow question...
    // https://stackoverflow.com/questions/5607589/right-way-to-split-an-stdstring-into-a-vectorstring
    string line;
    std::stringstream ss(pathStr);
    // read in the lines split at the colons
    while(getline(ss, line, ':')) 
    {
        // push them to the path vector
        path.push_back(line);
    }

}

int Path::find(const string& program) const
{
    int i = 0;
    // loop through all of the strings in the path vector
    for (string dir : path)
    {
        // open the current directory
        DIR* directory = opendir(dir.c_str());
        // for every entry in that directory, see if it matches the program name
        while(dirent* entry = readdir(directory))
        {
            if (entry->d_name == program) 
            {
                return i;
            }
        }
        i++;
        // close the directory to avoid memory leaks
        closedir(directory);
    }
    return -1;
}
