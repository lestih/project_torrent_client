#include "parsing/torrent_file.h"
#include "parsing/bencode.h"
#include "utils/byte_tools.h"
#include <vector>
#include <openssl/sha.h>
#include <fstream>
#include <variant>
#include <sstream>
#include <iostream>
#include <string_view>

using Map = Bencode::Map;

TorrentFile LoadTorrentFile(const std::string& filename) { 
    TorrentFile torrent_file;
    std::ifstream file(filename, std::ios::binary);
    std::ostringstream buffer;
    buffer << file.rdbuf();

    const std::string content = buffer.str();
    file.close();


    int64_t position = 0;
    int64_t start_hash, finish_hash;  // начальная и конечная позиция информации из поля info
    // получаем bencode словарь с помощью парсера, чтобы потом записать из него все данные в нужные переменные
    Map bencode_dict = std::get<Map>(Bencode::Parser(content, position, start_hash, finish_hash).val);  

    torrent_file.announce = std::get<std::string>(bencode_dict["announce"].val);
    torrent_file.comment = std::get<std::string>(bencode_dict["comment"].val);
    //torrent_file.announce_list = std::get<std::vector<std::string>>(bencode_dict["announce-list"].val);

    Map info = std::get<Map>(bencode_dict["info"].val);

    torrent_file.length = std::get<int64_t>(info["length"].val);
    torrent_file.name = std::get<std::string>(info["name"].val);
    torrent_file.pieceLength = std::get<int64_t>(info["piece length"].val);
    std::string pieces = std::get<std::string>(info["pieces"].val);

    // pieces представляет собой строку, кратную 20
    // каждая подстрока длиной 20 представляет собой SHA1 сегмента с соответствующим индексом
    for (size_t i = 0; i < pieces.size() - 20; i += 20) {
        torrent_file.pieceHashes.push_back(pieces.substr(i, 20));  //сегменты, на которые разделен файл 
    }

    // infohash это SHA1 от bencoded представления значения info
    // !!!именно content от info в bencoded строке
    std::string info_view(&content[start_hash], finish_hash - start_hash + 1);  // считаем infoHash 
    torrent_file.infoHash = CalculateSHA1(info_view);

    return torrent_file;
}


