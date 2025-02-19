#include <iostream>
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>

#include <iterator>
using namespace std;

/**
 * Executes a command in parallel multiple times.
 */
void executeParallelCommands(int numProcesses, const vector<string>& args) {
    if (numProcesses <= 0) {
        cerr << "Error: Number of processes must be greater than zero." << endl;
        return;
    }
    
    vector<pid_t> pids;
    
    for (int i = 0; i < numProcesses; ++i) {
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("Error: Failed to fork process");
            return;
        } 
        
        if (pid == 0) { // Child process
            vector<char*> c_args;
            for (const auto& arg : args) {
                c_args.push_back(const_cast<char*>(arg.c_str()));
            }
            c_args.push_back(nullptr);

            execvp(c_args[0], c_args.data());

            // If execvp fails, print error and exit child process
            perror("Error: Command execution failed");
            exit(EXIT_FAILURE);
        } 
        
        // Parent process
        pids.push_back(pid);
    }

    // Wait for all child processes to complete
    for (pid_t pid : pids) {
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("Error: Failed to wait for child process");
            return;
        }

        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            cerr << "Process " << pid << " exited with status: " << WEXITSTATUS(status) << endl;
        }
    }
}

/**
 * Main function: Parses input and runs parallel execution.
 */
int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <num_processes> <command> [args...]" << endl;
        return EXIT_FAILURE;
    }

    int numProcesses = atoi(argv[1]);
    if (numProcesses <= 0) {
        cerr << "Error: Invalid number of processes. Must be greater than zero." << endl;
        return EXIT_FAILURE;
    }

    vector<string> commandArgs(argv + 2, argv + argc);
    std::copy(commandArgs.begin(), commandArgs.end(), std::ostream_iterator<std::string>(std::cout, " "));
    std::cout << std::endl; 

    executeParallelCommands(numProcesses, commandArgs);

    return EXIT_SUCCESS;
}
