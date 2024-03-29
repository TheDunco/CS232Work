//
//  shell.cpp
//  CS232 Command Shell
//
//  Created by Duncan Van Keulen on 1/28/2021
//

#include "shell.h"

Shell::Shell() 
{
    Prompt prompt;
}

void Shell::run() 
{
    while(true) 
    {
        // display the prompt
        cout << prompt.get() << flush;
        // get the line the user enters and send it to the CommandLine
        CommandLine commline = CommandLine(cin);

        // Loop back if there are no command line arguments
        if(commline.getArgCount() <= 0) {
            continue;
        }
        // grab the command which should be the first string of the vector
        string argZero = commline.getArgVector(0);

        // "switch" based on the command and execute command if in shell. Otherwise fork and run from path
        if (argZero == "cd")
            cd(commline.getArgVector(1)); // change directories to the directory if it exists
        
        else if (argZero == "pwd")
            cout << prompt.getCwd() << "\n" << flush; // print cwd on cout

        else if (argZero == "exit")
            _exit(EXIT_SUCCESS); // exit gracefully

        else // we don't have an in-shell command for the program
            ForkChild(commline, argZero);
    }
}

void Shell::ForkChild(CommandLine& commline, string& argZero) {
    // based on Kerrek SB/Alan Haggai Alavi's answer to this stackoverflow question
    // https://stackoverflow.com/questions/19099663/how-to-correctly-use-fork-exec-wait
    pid_t parent = getpid();
    pid_t pid = fork();
    // failed to fork
    if (pid == -1)
    {
        cerr << "Child process failed to fork!";
        _exit(EXIT_FAILURE);
    }
    // we are the parent
    else if (pid > 0)
    {
        int status;
        // wait for child if ampersand
        if (commline.noAmpersand()) {
            waitpid(pid, &status, 0);
        }
    }
    // we are the child
    else
    {
        cout << flush;
        try // if there is an error, an invalid/nonexistent command was provided
        {
            RunFromPath(commline, argZero);
        }
        catch (...)
        {
            cerr << "Invalid command\n" << flush;
        }
    }
}

void Shell::RunFromPath(CommandLine& commline, string& argZero) {
    Path path;
            
    // Make way for the command text if there was an ampersand
    if (!commline.noAmpersand())
    {
        cout << "\n" << flush;
    }

    // get the path of the command we're trying to run
    string pathname = path.getDirectory(path.find(argZero)) + "/" + argZero;

    // execute the command located at that path
    execve(pathname.c_str(), commline.getArgVector(), NULL);

    _exit(EXIT_FAILURE); // exec never returns, so if we get here it's an error
}

// change directories
void Shell::cd(const char* dir) {
    if (chdir(dir) >= 0) {
        // we've successfully changed directories
        prompt.set(); // update prompt
    }
    else // chdir gave an error 
    {
        cerr << "No such directory \"" << dir << "\"\n" << flush;
    }
}

Shell::~Shell()
{
    delete &prompt;
}
