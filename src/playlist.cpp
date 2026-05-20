#include "playlist.h"
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

void Playlist::addFile(const std::string& path)
{
    Entry e;
    e.path = path;
    e.displayName = fs::path(path).filename().string();
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
