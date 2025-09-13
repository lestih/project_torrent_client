#pragma once

#include <vector>
#include <openssl/sha.h>
#include <fstream>
#include <variant>
#include <list>
#include <map>
#include <sstream>
//#include <cpr/cpr.h>

#include <arpa/inet.h>
#include <bitset>
#include <iostream>
#include <cstring>

#include <string>
#include "torrent_file.h"
#include "peer.h"

class TorrentTracker {
public:
    explicit TorrentTracker(const std::string& url) : url_(url) {};

    void UpdatePeers(const TorrentFile& tf, std::string peerId, int port){
        // cpr::Response result;
        // size_t announce_index = 0;

        // do {
        //     result = cpr::Get(
        //         cpr::Url{url_},
        //         cpr::Parameters {
        //                 {"info_hash", tf.infoHash},
        //                 {"peer_id", peerId},
        //                 {"port", std::to_string(port)},
        //                 {"uploaded", std::to_string(0)},
        //                 {"downloaded", std::to_string(0)},
        //                 {"left", std::to_string(tf.length)},
        //                 {"compact",    std::to_string(1)}
        //         },
        //         cpr::Timeout{20000}
        //     );

        //     if ((result.status_code == 0 || result.status_code >= 400) && 
        //      (announce_index < tf.announce_list.size())) {
        //         url_ = tf.announce_list[announce_index++];
        //     }
        // } while ((result.status_code == 0 || result.status_code >= 400) && 
        //      (announce_index < tf.announce_list.size()));

        // if(result.status_code == 0){
        //     throw std::runtime_error("url error");
        // }

        // if (result.status_code != 200) {
        //     throw std::runtime_error("Tracker request failed");
        // }

        // const std::string& response_text = result.text;
        const std::string& response_text = "";


        int peers_position = response_text.find("peers");  // находим позицию peers и переводим указатель на список пиров
        if (peers_position == std::string::npos) {
            throw std::runtime_error("Invalid tracker response: peers not found");
        }

        peers_position += 5;

        std::string num_peers_string;
        while (response_text[peers_position] != ':') {
            num_peers_string.push_back(response_text[peers_position]);
            ++peers_position;
        }

        ++peers_position;
        int size_string_peers = stoll(num_peers_string);  // считаем размер строки, содержащей пиры

        size_t peer_size = 6;
        
        std::vector<Peer> peers;
        for (int index = peers_position; index + peer_size <= size_string_peers + peers_position; index += peer_size) {
            Peer peer;
            
            // Извлекаем ip адресс
            uint32_t ip_adress;
            std::memcpy(&ip_adress, &response_text[index], 4);

            char ip_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &ip_adress, ip_str, INET_ADDRSTRLEN);
            peer.ip = ip_str;

            // извлекаем порт
            uint16_t port_num;
            std::memcpy(&port_num, &response_text[index + 4], 2);
            peer.port = ntohs(port_num);

            peers.push_back(peer);
        }
        peers_ = peers;
    }

    const std::vector<Peer>& GetPeers() const;

private:
    std::string url_;
    std::vector<Peer> peers_;
};
