#include "network/torrent_tracker.h"
#include "parsing/bencode.h"
#include "utils/byte_tools.h"
//#include <cpr/cpr.h>

const std::vector<Peer> &TorrentTracker::GetPeers() const {
    return peers_;
}
