#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <filesystem>
#include <sstream>
#include <cstdlib>
#include <sys/time.h>
#include <cstdio>
#include "benches/dedup.h"
#include "benches/io-lat-write.h"
#include "benches/dedup.h"

using namespace std;
namespace fs = std::filesystem;

const size_t BLOCK_SIZE = 4096;  // Define block size for I/O latency tests
void worker_thread(int iterations, const string& program) {
    for (int i = 0; i < iterations; i++) {
        if (program == "io-lat-write") {
            io_lat_write("benchmark_output.dat");
        } else if (program == "dedup") {
            long arr_size = 10000;  // Default size
            size_t unique_count = dedup(arr_size);
            //printf("Iteration %d: Unique elements = %zu\n", i + 1, unique_count);
        }
    }
}


int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <program> <num_threads> <iterations>" << endl;
        cerr << "  <program>: 'dedup' or 'io-lat-write'" << endl;
        cerr << "  <num_threads>: number of threads" << endl;
        cerr << "  <iterations>: number of iterations per thread" << endl;
        return 1;
    }

    string program = argv[1];
    int num_threads = stoi(argv[2]);
    int iterations = stoi(argv[3]);

    if (program != "dedup" && program != "io-lat-write") {
        cerr << "Invalid algorithm. Use 'dedup' or 'io-lat-write'." << endl;
        return 1;
    }

    vector<thread> threads;
    auto start = chrono::high_resolution_clock::now();

    for (int i = 0; i < num_threads; i++) {
        threads.emplace_back(worker_thread, iterations, program);
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::seconds>(end - start);
    auto duration_precise = chrono::duration_cast<chrono::duration<double>>(end - start);

    cout << "CPU load completed with " << num_threads << " threads and "
         << iterations << " iterations per thread. Execution time: "
         << duration_precise.count() << " seconds." << endl;

    return 0;
}
