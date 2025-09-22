#pragma once

#include <string>
#include <vector>

// метаинформация торрент файла
struct TorrentFile {
    std::string announce;
    std::vector<std::string> announce_list;
    std::string comment;
    std::vector<std::string> pieceHashes;
    size_t pieceLength;
    size_t length;
    std::string name;
    std::string infoHash;
};

TorrentFile LoadTorrentFile(const std::string& filename);
