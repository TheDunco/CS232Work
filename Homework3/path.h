//
//  path.h
//  CS232 Command Shell
//
//  Created by Duncan Van Keulen on 1/28/2021
//

#include <iostream>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <vector>
#include <sstream>
#include <sys/types.h>
#include <dirent.h>
using namespace std;


class Path
{
public:
    Path();
    int find(const string& program) const;
    string getDirectory(int i) const { return path[i]; }
private:
    vector<string> path;
};
