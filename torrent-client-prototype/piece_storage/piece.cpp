#include "utils/byte_tools.h"
#include "piece_storage/piece.h"
#include <iostream>
#include <algorithm>

namespace {
constexpr size_t BLOCK_SIZE = 1 << 14;
}

Piece::Piece(size_t index, size_t length, std::string hash) 
: index_(index), 
  length_(length), 
  hash_(hash) {
    blocks_.resize((length_ + BLOCK_SIZE - 1) / BLOCK_SIZE);  // делим размер части на блоки
    size_t offset = 0;
    for (auto& block : blocks_) {
        block.offset = offset;
        block.piece = index_;
        if (offset + BLOCK_SIZE >= length_) { 
            block.length = length_ - offset;  // если размер последнего блока меньше BLOCK_SIZE, считаем вручную
        } else {
            block.length = BLOCK_SIZE;
        }

        offset += BLOCK_SIZE;
    }
}

/*
* Совпадает ли хеш скачанных данных с ожидаемым
*/
bool Piece::HashMatches() const {
    return GetDataHash() == GetHash(); 
}

/*
* Дать указатель на отсутствующий (еще не скачанный и не запрошенный) блок
*/
Block* Piece::FirstMissingBlock() {
    for (auto& block: blocks_) {
        if (block.status == Block::Status::Missing) {
            return &block; // ??? обычный указатель
        }
    }

    return nullptr;
}

size_t Piece::GetIndex() const {
    return index_;
}

void Piece::SaveBlock(size_t blockOffset, std::string data) {
    blocks_[blockOffset / BLOCK_SIZE].status = Block::Status::Retrieved;  // считаем индекс по смещению от начала piece
    retrieved_cnt++;
    blocks_[blockOffset / BLOCK_SIZE].data = data;
}

bool Piece::AllBlocksRetrieved() const {
    return retrieved_cnt == blocks_.size();
}

/*
* Получить скачанные данные для части файла
*/
std::string Piece::GetData() const {
    std::string all_blocks_data;
    for (auto& block: blocks_) {
        all_blocks_data += block.data;
    }

    return all_blocks_data;
}

/*
* Посчитать хеш по скачанным данным
*/
std::string Piece::GetDataHash() const {
    return CalculateSHA1(GetData());
}

/*
* Получить хеш для части из .torrent файла
*/
const std::string& Piece::GetHash() const {
    return hash_;
}


/*
* Удалить все скачанные данные и отметить все блоки как Missing
*/
void Piece::Reset(){
    for (auto& block: blocks_) {
        block.data.clear();
        block.status = Block::Status::Missing;
    }
    retrieved_cnt = 0;
}