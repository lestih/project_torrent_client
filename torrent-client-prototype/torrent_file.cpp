#include "torrent_file.h"
#include "bencode.h"
#include "byte_tools.h"
#include <vector>
#include <openssl/sha.h>
#include <fstream>
#include <variant>
#include <sstream>
#include <iostream>
#include <string_view>

using Map = Bencode::Map;

TorrentFile LoadTorrentFile(const std::string& filename) { 
    TorrentFile answer;
    std::ifstream file(filename, std::ios::binary);
    std::ostringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();


    long long pos = 0;
    long long start_hash, finish_hash;
    Map dict = std::get<Map>(Bencode::parser(content, pos, start_hash, finish_hash).val);

    answer.announce = std::get<std::string>(dict["announce"].val);
    answer.comment = std::get<std::string>(dict["comment"].val);
    answer.announce_list = std::get<std::vector<std::string>>(dict["announce-list"].val);

    Map info = std::get<Map>(dict["info"].val);

    answer.length = std::get<long long>(info["length"].val);
    answer.name = std::get<std::string>(info["name"].val);
    answer.pieceLength = std::get<long long>(info["piece length"].val);
    std::string pieces = std::get<std::string>(info["pieces"].val);

    for(size_t i = 0;i<pieces.size() - 20;i += 20){
        answer.pieceHashes.push_back(pieces.substr(i, 20));
    }

    std::string info_view(&content[start_hash], finish_hash - start_hash + 1);
    answer.infoHash = CalculateSHA1(info_view);

    return answer;
}


