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

    // ── 播放列表文件管理 ──
    void init(const std::string& exeDir);
    const std::string& currentName() const { return currentPlaylistName_; }
    std::vector<std::string> listPlaylistNames() const;
    void switchToPlaylist(const std::string& name);
    void createPlaylist(const std::string& name);
    void saveToFile() const;
    void appendSongToFile(const std::string& songPath);

private:
    void loadFromFile(const std::string& name);

    std::vector<Entry> entries_;
    int                current_ = 0;

    std::string playlistDir_;
    std::string currentPlaylistName_;
};
