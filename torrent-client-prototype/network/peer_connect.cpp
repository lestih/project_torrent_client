#include "utils/byte_tools.h"
#include "network/peer_connect.h"
#include "network/message.h"
#include <iostream>
#include <sstream>
#include <utility>
#include <atomic>

using namespace std::chrono_literals;

PeerPiecesAvailability::PeerPiecesAvailability() {}

PeerPiecesAvailability::PeerPiecesAvailability(std::string bitfield) : bitfield_(bitfield) {}

bool PeerPiecesAvailability::IsPieceAvailable(size_t pieceIndex) const {
    return bitfield_[pieceIndex / 8] & (1 << (7 - pieceIndex % 8));
}

void PeerPiecesAvailability::SetPieceAvailability(size_t pieceIndex) {
    bitfield_[pieceIndex / 8] |= (1 << (7 - pieceIndex % 8));
}

size_t PeerPiecesAvailability::Size() const {
    return 8 * bitfield_.size();
}


PeerConnect::PeerConnect(const Peer& peer, const TorrentFile &tf, std::string selfPeerId, PieceStorage& pieceStorage)
: tf_(tf),
  socket_(peer.ip, peer.port, 2000ms, 2000ms), 
  selfPeerId_(selfPeerId), piecesAvailability_(PeerPiecesAvailability(std::string((tf.pieceHashes.size() + 7) / 8, 0))), 
  terminated_(false),
  choked_(true), 
  pieceStorage_(pieceStorage), 
  pendingBlock_(false) {}


void PeerConnect::Run() {
    while (!terminated_) {
        if (EstablishConnection()) {
            std::cout << "Connection established to peer" << std::endl;
            try {
                MainLoop();
            } catch (const std::exception& e) {
                std::cout << "Mainloop error" << std::endl;
                Terminate();
            }
        } else {
            failed_ = true;
            std::cerr << "Cannot establish connection to peer" << std::endl;
            Terminate();
        }
    }
}

// Функция производит handshake.
// Подключиться к пиру по протоколу TCP
// Отправить пиру сообщение handshake
// Проверить правильность ответа пира
void PeerConnect::PerformHandshake() {
    socket_.EstablishConnection();
    const std::string data = std::string(1, 19) + "BitTorrent protocol00000000" + tf_.infoHash + selfPeerId_; // какой peer передаем
    socket_.SendData(data);
    const std::string data_ans = socket_.ReceiveData(68);

    if (data_ans[0] != '\x13' || data_ans.substr(1, 19) != "BitTorrent protocol" || data_ans.substr(28, 20) != tf_.infoHash) {
        throw std::runtime_error("Ошибка размера или протокола");
    }
}

// Провести handshake
// Получить bitfield с информацией о наличии у пира различных частей файла
// Сообщить пиру, что мы готовы получать от него данные (отправить interested)
// Возвращает true, если все три этапа прошли без ошибок
bool PeerConnect::EstablishConnection() {
    try {
        PerformHandshake();
        ReceiveBitfield();
        SendInterested();
        return true;
    } catch (const std::exception& e) {
        failed_ = true;
        std::cerr << "Failed to establish connection with peer " << socket_.GetIp() << ":" <<
            socket_.GetPort() << " -- " << e.what() << std::endl;
        return false;
    }
}

// Функция читает из сокета bitfield с информацией о наличии у пира различных частей файла.
// Полученная информация сохраняется в поле `piecesAvailability_`.
// !!! сообщение bitfield опицаонально, пиры могут сразу слать Unchoke
void PeerConnect::ReceiveBitfield() {
    std::string data = socket_.ReceiveData();

    MessageId id = static_cast<MessageId>(data[0]);

    while (id != MessageId::Unchoke) {
        if (id == MessageId::BitField) {
            std::string bitfield = data.substr(1, data.size() - 1);
            piecesAvailability_ = PeerPiecesAvailability(bitfield);
        }
        if (id == MessageId::Have) {
            std::string have = data.substr(1, data.size() - 1);
            size_t index = BytesToInt(have);
            piecesAvailability_.SetPieceAvailability(index);
        }

        data = socket_.ReceiveData();
        id = static_cast<MessageId>(data[0]);
    }
    choked_ = false;
}

//Функция посылает пиру сообщение типа interested
void PeerConnect::SendInterested() {
    std::string data = IntToBytes(1) + std::string(1, static_cast<char>(MessageId::Interested));
    this->socket_.SendData(data);
}

// Функция отправляет пиру сообщение типа request. Это сообщение обозначает запрос части файла у пира.
// За одно сообщение запрашивается блок размера 2^14 байт
// Какую часть файла надо скачать запрашиваем у pieceStorage_
void PeerConnect::RequestPiece() {
    size_t total_pieces_count = pieceStorage_.TotalPiecesCount();
    while (total_pieces_count--) {
        if (pieceInProgress_ == nullptr) 
            pieceInProgress_ = pieceStorage_.GetNextPieceToDownload();
        if (piecesAvailability_.IsPieceAvailable(pieceInProgress_->GetIndex())) {
            Block* block = pieceInProgress_->FirstMissingBlock();
            Message msg = Message::Init(MessageId::Request, IntToBytes(pieceInProgress_->GetIndex()) + 
            IntToBytes(block->offset) + IntToBytes(block->length));

            std::string data = msg.ToString();
            this->socket_.SendData(data);
            pendingBlock_ = true;
            return;
        }
    }
    return;

}

void PeerConnect::Terminate() {
    std::cerr << "Terminate" << std::endl;
    terminated_ = true;
}

bool PeerConnect::Failed() const {
    return failed_;
}

// Основной цикл общения с пиром. Здесь ждем следующее сообщение от пира и обрабатываем его.
void PeerConnect::MainLoop() {
    while (!terminated_) {
        if (!choked_ && !pendingBlock_) {
            RequestPiece();
        }

        Message msg = Message::Parse(socket_.ReceiveData());
        if(msg.id == MessageId::Choke) {
            choked_ = true;
        } else if (msg.id == MessageId::Unchoke) {
            choked_ = false;
        } else if (msg.id == MessageId::Have) {
            size_t index = BytesToInt(msg.payload);
            piecesAvailability_.SetPieceAvailability(index);
        } else if (msg.id == MessageId::Piece) {
            pendingBlock_ = false;
            // size_t index = BytesToInt(msg.payload.substr(0, 4));  
            size_t begin = BytesToInt(msg.payload.substr(4, 4));
            std::string data = msg.payload.substr(8);

            pieceInProgress_->SaveBlock(begin, data);
            if (pieceInProgress_->AllBlocksRetrieved()) {

                if (pieceInProgress_->HashMatches()) {  // если хэши совпадают
                    pieceStorage_.PieceProcessed(pieceInProgress_);
                    if (pieceStorage_.QueueIsEmpty()) {
                        Terminate();
                    }
                    pieceInProgress_ = pieceStorage_.GetNextPieceToDownload();
                } else{
                    pieceInProgress_->Reset(); // стоит ли делать перевод на след блок
                }
            }
        }
    }
}
