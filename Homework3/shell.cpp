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

        // Check if there are actually arguments.
        if(commline.getArgCount() <= 0) {
            continue;
        }
        // grab the command which should be the first string of the vector
        string argZero = commline.getArgVector(0);

        // "switch" based on the command and execute command if in shell. OTherwise fork and run from path
        if (argZero == "cd")
        {
            // change directories to the directory if it exists
            cd(commline.getArgVector(1));
        } 
        
        else if (argZero == "pwd")
        {
            cout << prompt.getCwd() << "\n" << flush;
        }

        else if (argZero == "exit")
        {
            // exit gracefully
            _exit(EXIT_SUCCESS);
        }

        else // we don't have an in-shell command for the program
        {

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
                // wait for child if ampersand...
                // this seems backwards to me but it works like this
                // to be clear, we only wait for the child if there WAS and ampersand.
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
                    _exit(EXIT_FAILURE); // exec never returns
                }
                catch (...)
                {
                    cerr << "Invalid command\n" << flush;
                }
            }
        }
    }
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
