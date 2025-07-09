#pragma once

#include <vector>
#include <openssl/sha.h>
#include <fstream>
#include <variant>
#include <list>
#include <map>
#include <sstream>
#include <cpr/cpr.h>

#include <arpa/inet.h>
#include <bitset>
#include <iostream>
#include <cstring>

#include <string>
#include "torrent_file.h"
#include "peer.h"

class TorrentTracker {
public:
    TorrentTracker(const std::string& url): url_(url){};

    void UpdatePeers(const TorrentFile& tf, std::string peerId, int port){
        cpr::Response res;
        size_t announce_index = 0;
        do {
            res = cpr::Get(
                cpr::Url{url_},
                cpr::Parameters {
                        {"info_hash", tf.infoHash},
                        {"peer_id", peerId},
                        {"port", std::to_string(port)},
                        {"uploaded", std::to_string(0)},
                        {"downloaded", std::to_string(0)},
                        {"left", std::to_string(tf.length)},
                        {"compact",    std::to_string(1)}
                },
                cpr::Timeout{20000}
            );
            if(res.status_code == 0){
                url_ = tf.announce_list[announce_index++];
            }
        } while(res.status_code == 0 && announce_index < f.announce_list.size());

        if(res.status_code == 0){
            throw std::runtime_error("url error");
        }

        if (res.status_code != 200) {
            throw std::runtime_error("Tracker request failed");
        }

        const std::string buff = res.text;


        int pos_peers = buff.find("peers");
        std::cerr << pos_peers << std::endl;
        pos_peers += 5;
        std::string num_peers_string;
        while(buff[pos_peers] != ':'){
            num_peers_string.push_back(buff[pos_peers]);
            ++pos_peers;
        }
        ++pos_peers;
        int num_peers = stoll(num_peers_string);

        size_t peer_size = 6;
        
        std::vector<Peer> peers;
        for(int i = pos_peers;i + peer_size <= num_peers + pos_peers;i += peer_size){
            Peer pr;
            std::string ans;
            for(size_t j = 0;j<4;++j){
                ans += std::to_string(static_cast<int>(static_cast<unsigned char>(buff[i + j])));
                if(j < 3) ans += '.';
            }

            uint32_t IP;
            std::memcpy(&IP, &buff[i], 4);

            char IP_ans[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &IP, IP_ans, INET_ADDRSTRLEN);
            pr.ip = IP_ans;

            uint16_t port;
            std::memcpy(&port, &buff[i + 4], 2);
            pr.port = ntohs(port);
            peers.push_back(pr);
        }
        peers_ = peers;
    }

    const std::vector<Peer>& GetPeers() const;

private:
    std::string url_;
    std::vector<Peer> peers_;
};
