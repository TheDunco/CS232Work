//
//  shell.h
//  CS232 Command Shell
//
//  Created by Duncan Van Keulen on 1/28/2021
//

#include <iostream>
#include <string>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;
#include "prompt.h"
#include "commandline.h"
#include "path.h"

class Shell
{
public:
    Shell();
    void run();
    ~Shell();
private:
    Prompt prompt;
    void cd(const char* str);
    void ForkChild(CommandLine& commline, string& argZero);
    void RunFromPath(CommandLine& commline, string& argZero);
};
