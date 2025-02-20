#ifndef BLOCK_CACHE_HPP
#define BLOCK_CACHE_HPP

#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <list>
#include <unordered_map>
#include <vector>

// Hash function for (fd, offset) pairs
struct pair_hash {
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2>& p) const {
        return std::hash<T1>()(p.first) ^ std::hash<T2>()(p.second);
    }
};

class BlockCache {
public:
    explicit BlockCache(size_t cache_size);
    ~BlockCache();

    int openFile(const std::string& path);
    int closeFile(int fd);
    ssize_t read(int fd, void* buf, size_t count);
    ssize_t write(int fd, const void* buf, size_t count);
    int fsync(int fd);

private:
    struct CacheBlock {
        int fd;
        off_t offset;
        std::vector<char> data;
        bool dirty;
    };

    size_t cache_size_;
    std::list<CacheBlock> cache_list_;
    std::unordered_map<std::pair<int, off_t>, std::list<CacheBlock>::iterator, pair_hash> cache_map_;

    void evictBlock();
    ssize_t loadBlock(int fd, off_t offset);
};

#endif // BLOCK_CACHE_HPP
