#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>
#include <chrono>
#include <sys/resource.h>

using namespace std;

/**
 * Parses a command string into a vector of arguments.
 */
vector<string> parseCommand(const string& input) {
    istringstream iss(input);
    vector<string> tokens;
    string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

/**
 * Prints execution time statistics.
 */
void printExecutionTime(chrono::high_resolution_clock::time_point start,
                        chrono::high_resolution_clock::time_point end,
                        const struct rusage& usage) {
    auto realTime = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    auto userTime = usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec / 1000;
    auto sysTime = usage.ru_stime.tv_sec * 1000 + usage.ru_stime.tv_usec / 1000;

    cout << "Time: real = " << realTime << "ms, user = " << userTime << "ms, sys = " << sysTime << "ms" << endl;
}

/**
 * Executes a command with optional time measurement.
 */
int executeCommand(const vector<string>& args, bool measureTime) {
    if (args.empty()) return -1;

    auto start = chrono::high_resolution_clock::now();
    pid_t pid = fork();

    if (pid < 0) {
        perror("Error: Failed to fork process");
        return -1;
    } else if (pid == 0) {
        vector<char*> c_args;
        for (const auto& arg : args) {
            c_args.push_back(const_cast<char*>(arg.c_str()));
        }
        c_args.push_back(nullptr);

        execvp(c_args[0], c_args.data());

        // If execvp fails, print error and exit child process
        perror("Error: Command execution failed");
        exit(EXIT_FAILURE);
    } else {
        int status;
        struct rusage usage;

        if (wait4(pid, &status, 0, &usage) == -1) {
            perror("Error: Failed to wait for child process");
            return -1;
        }

        auto end = chrono::high_resolution_clock::now();

        if (measureTime) {
            printExecutionTime(start, end, usage);
        }

        return WEXITSTATUS(status);
    }
}

/**
 * Runs the interactive shell loop.
 */
void runShellLoop() {
    while (true) {
        cout << ">>> ";
        string command;
        if (!getline(cin, command)) {
            cout << endl;  // Handle EOF (Ctrl+D)
            break;
        }

        vector<string> args = parseCommand(command);
        if (args.empty()) continue;

        if (args[0] == "exit") break;

        bool measureTime = false;
        if (args[0] == "time" && args.size() > 1) {
            measureTime = true;
            args.erase(args.begin());
        }

        if (args[0] == "cd") {
            if (args.size() < 2) {
                cerr << "Usage: cd <directory>" << endl;
            } else if (chdir(args[1].c_str()) != 0) {
                perror("Error: Failed to change directory");
            }
        } else {
            int status = executeCommand(args, measureTime);
            if (status != 0) {
                cerr << "Command exited with status: " << status << endl;
            }
        }
    }
}

/**
 * Main function to start the shell.
 */
int main() {
    runShellLoop();
    return 0;
}
