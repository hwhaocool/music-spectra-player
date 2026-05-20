// 播放列表

#pragma once
#include <string>
#include <vector>

class Playlist {
public:
    struct Entry {
        std::string path;
        std::string displayName;
        float       durationHint = 0.f; // 秒
    };

    void addFile(const std::string& path);
    void addEntry(const Entry& e);
    void remove(int index);
    void clear();

    int  size()                   const { return (int)entries_.size(); }
    const Entry& at(int i)        const { return entries_[i]; }
    Entry&       at(int i)              { return entries_[i]; }
    int  currentIndex()           const { return current_; }
    void setCurrent(int i)              { current_ = i; }

    // 返回下一首的索引，-1 表示没有
    int  next() {
        if (entries_.empty()) return -1;
        current_ = (current_ + 1) % (int)entries_.size();
        return current_;
    }
    int  prev() {
        if (entries_.empty()) return -1;
        current_ = (current_ - 1 + (int)entries_.size()) % (int)entries_.size();
        return current_;
    }

    const std::vector<Entry>& entries() const { return entries_; }

private:
    std::vector<Entry> entries_;
    int                current_ = 0;
};
