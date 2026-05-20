#include "playlist.h"
#include <algorithm>
#include <filesystem>
#include "unicode_utils.h"

namespace fs = std::filesystem;

void Playlist::addFile(const std::string& path)
{
    Entry e;
    e.path = path;
     // 修复：纯字符串提取文件名，不经过 fs::path 窄字符构造
    e.displayName = utf8DisplayName(path);
    entries_.push_back(std::move(e));
}

void Playlist::addEntry(const Entry& e) { entries_.push_back(e); }

void Playlist::remove(int index)
{
    if (index >= 0 && index < (int)entries_.size()) {
        entries_.erase(entries_.begin() + index);
        if (current_ >= (int)entries_.size()) current_ = std::max(0, (int)entries_.size() - 1);
    }
}

void Playlist::clear()
{
    entries_.clear();
    current_ = 0;
}
