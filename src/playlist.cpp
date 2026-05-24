#include "playlist.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include "unicode_utils.h"

namespace fs = std::filesystem;

void Playlist::addFile(const std::string& path)
{
    Entry e;
    e.path = path;
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

// ── 播放列表文件管理 ──

void Playlist::init(const std::string& exeDir)
{
    playlistDir_ = exeDir + "/playlist";
    auto dirPath = utf8Path(playlistDir_);
    fs::create_directories(dirPath);

    auto names = listPlaylistNames();
    if (names.empty()) {
        createPlaylist("default");
        names = listPlaylistNames();
    }
    if (!names.empty())
        switchToPlaylist(names[0]);
}

std::vector<std::string> Playlist::listPlaylistNames() const
{
    std::vector<std::string> names;
    auto dirPath = utf8Path(playlistDir_);
    std::error_code ec;
    for (auto& entry : fs::directory_iterator(dirPath, ec)) {
        if (entry.path().extension() == ".playlist") {
            names.push_back(entry.path().stem().string());
        }
    }
    return names;
}

void Playlist::switchToPlaylist(const std::string& name)
{
    if (name == currentPlaylistName_) return;

    if (!currentPlaylistName_.empty())
        saveToFile();

    clear();
    currentPlaylistName_ = name;
    loadFromFile(name);
}

void Playlist::createPlaylist(const std::string& name)
{
    std::string filePath = playlistDir_ + "/" + name + ".playlist";
    auto fp = utf8Path(filePath);
    if (fs::exists(fp)) return;
    std::ofstream ofs(fp);
}

void Playlist::saveToFile() const
{
    if (currentPlaylistName_.empty()) return;
    std::string filePath = playlistDir_ + "/" + currentPlaylistName_ + ".playlist";
    auto fp = utf8Path(filePath);
    std::ofstream ofs(fp, std::ios::trunc);
    for (auto& entry : entries_)
        ofs << entry.path << "\n";
}

void Playlist::appendSongToFile(const std::string& songPath)
{
    if (currentPlaylistName_.empty()) return;
    std::string filePath = playlistDir_ + "/" + currentPlaylistName_ + ".playlist";
    auto fp = utf8Path(filePath);
    std::ofstream ofs(fp, std::ios::app);
    ofs << songPath << "\n";
}

void Playlist::loadFromFile(const std::string& name)
{
    std::string filePath = playlistDir_ + "/" + name + ".playlist";
    auto fp = utf8Path(filePath);
    std::ifstream ifs(fp);
    std::string line;
    while (std::getline(ifs, line)) {
        if (!line.empty())
            addFile(line);
    }
}
