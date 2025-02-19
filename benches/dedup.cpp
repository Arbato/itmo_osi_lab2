#include <iostream>
#include <vector>
#include <unordered_set>
#include <chrono>
#include <cstdlib>

using namespace std;

/**
 * Generates an array of random numbers.
 */
vector<long> generateRandomArray(long size) {
    vector<long> arr(size);
    for (long i = 0; i < size; ++i) {
        arr[i] = rand() % size;  // Random numbers in range [0, size)
    }
    return arr;
}

/**
 * Deduplicates an array and returns the number of unique elements.
 * Also measures execution time.
 */
size_t deduplicate(vector<long>& arr) {
    auto start = chrono::high_resolution_clock::now();

    unordered_set<long> unique_elements(arr.begin(), arr.end());  // Deduplicate

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    return unique_elements.size();
}

/**
 * Dedup Benchmark Function
 */
size_t dedup(long arr_size) {
    vector<long> arr = generateRandomArray(arr_size);
    size_t unique_count = deduplicate(arr);
    return unique_count;
}

