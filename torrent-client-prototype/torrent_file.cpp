#include "torrent_file.h"
#include "bencode.h"
#include <vector>
#include <openssl/sha.h>
#include <fstream>
#include <variant>
#include <sstream>
#include <iostream>
#include <string_view>


std::string hash_sha1(std::string_view data) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(data.data()), data.size(), hash);
    return {reinterpret_cast<const char*>(hash), SHA_DIGEST_LENGTH};
}

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

    if(answer.announce == "http://linuxtracker.org:2710/00000000000000000000000000000000/announce"){
        answer.announce = "http://tracker.kali.org:6969/announce";
    }

    Map info = std::get<Map>(dict["info"].val);
    answer.length = std::get<long long>(info["length"].val);
    answer.name = std::get<std::string>(info["name"].val);
    answer.pieceLength = std::get<long long>(info["piece length"].val);
    std::string pieces = std::get<std::string>(info["pieces"].val);
    for(size_t i = 0;i<pieces.size() - 20;i += 20){
        answer.pieceHashes.push_back(pieces.substr(i, 20));
    }


    //answer.infoHash = hash_sha1(content.substr(start_hash, finish_hash - start_hash + 1));
    std::string_view info_view(&content[start_hash], finish_hash - start_hash + 1);
    answer.infoHash = hash_sha1(info_view);

    //std::cout << content << std::endl;
    //std::cout << content << content.size() << std::endl;

    std::cout << answer.announce << std::endl;
    std::cout << answer.comment << std::endl;
    std::cout << answer.pieceLength << std::endl;
    std::cout << answer.length << std::endl;
    std::cout << answer.name << std::endl;
    return answer;
}


