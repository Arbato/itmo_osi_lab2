#include "cache.h"

// Constructor
BlockCache::BlockCache(size_t cache_size) : cache_size_(cache_size) {}

// Destructor - flush dirty pages
BlockCache::~BlockCache() {
    for (auto& block : cache_list_) {
        if (block.dirty) {
            lseek(block.fd, block.offset, SEEK_SET);
            write(block.fd, block.data.data(), block.data.size());
        }
    }
}

// Open file
int BlockCache::openFile(const std::string& path) {
    int fd = open(path.c_str(), O_RDWR | O_DIRECT);
    if (fd < 0) {
        perror("open");
        return -1;
    }
    return fd;
}

// Close file
int BlockCache::closeFile(int fd) {
    for (auto it = cache_list_.begin(); it != cache_list_.end();) {
        if (it->fd == fd) {
            cache_map_.erase({fd, it->offset});
            it = cache_list_.erase(it);
        } else {
            ++it;
        }
    }
    return close(fd);
}

// Read from file with caching
ssize_t BlockCache::read(int fd, void* buf, size_t count) {
    off_t offset = lseek(fd, 0, SEEK_CUR);
    if (offset < 0) return -1;

    auto it = cache_map_.find({fd, offset});
    if (it != cache_map_.end()) {
        cache_list_.splice(cache_list_.begin(), cache_list_, it->second);
        memcpy(buf, it->second->data.data(), count);
        return count;
    }

    ssize_t bytes = loadBlock(fd, offset);
    if (bytes > 0) {
        memcpy(buf, cache_list_.begin()->data.data(), count);
    }
    return bytes;
}

// Write to file with caching
ssize_t BlockCache::write(int fd, const void* buf, size_t count) {
    off_t offset = lseek(fd, 0, SEEK_CUR);
    if (offset < 0) return -1;

    auto it = cache_map_.find({fd, offset});
    if (it != cache_map_.end()) {
        cache_list_.splice(cache_list_.begin(), cache_list_, it->second);
        memcpy(it->second->data.data(), buf, count);
        it->second->dirty = true;
        return count;
    }

    evictBlock();
    CacheBlock new_block = {fd, offset, std::vector<char>(count), true};
    memcpy(new_block.data.data(), buf, count);
    cache_list_.push_front(new_block);
    cache_map_[{fd, offset}] = cache_list_.begin();
    return count;
}

// Flush cached data to file
int BlockCache::fsync(int fd) {
    for (auto& block : cache_list_) {
        if (block.fd == fd && block.dirty) {
            lseek(block.fd, block.offset, SEEK_SET);
            write(block.fd, block.data.data(), block.data.size());
            block.dirty = false;
        }
    }
    return ::fsync(fd);
}

// Load a block into cache
ssize_t BlockCache::loadBlock(int fd, off_t offset) {
    evictBlock();
    CacheBlock new_block = {fd, offset, std::vector<char>(4096), false};
    ssize_t bytes = pread(fd, new_block.data.data(), 4096, offset);
    if (bytes > 0) {
        cache_list_.push_front(new_block);
        cache_map_[{fd, offset}] = cache_list_.begin();
    }
    return bytes;
}

// Evict least-recently-used block
void BlockCache::evictBlock() {
    if (cache_list_.size() < cache_size_) return;

    auto last = std::prev(cache_list_.end());
    if (last->dirty) {
        lseek(last->fd, last->offset, SEEK_SET);
        write(last->fd, last->data.data(), last->data.size());
    }
    cache_map_.erase({last->fd, last->offset});
    cache_list_.pop_back();
}
