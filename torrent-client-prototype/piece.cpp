#include "byte_tools.h"
#include "piece.h"
#include <iostream>
#include <algorithm>

namespace {
constexpr size_t BLOCK_SIZE = 1 << 14;
}

Piece::Piece(size_t index, size_t length, std::string hash): index_(index), length_(length), hash_(hash){
    blocks_.resize((length_ + BLOCK_SIZE - 1) / BLOCK_SIZE);

    size_t offs = 0;
    for(auto& block: blocks_){
        block.offset = offs;
        block.piece = index_;
        if(offs + BLOCK_SIZE >= length_) block.length = length_ - offs; //
        else block.length = BLOCK_SIZE;
        offs += BLOCK_SIZE;
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
Block* Piece::FirstMissingBlock(){
    for(auto& block: blocks_){
        if(block.status == Block::Status::Missing) return &block; // ??? обычный указатель
    }
    return nullptr;
}

size_t Piece::GetIndex() const{
    return index_;
}

void Piece::SaveBlock(size_t blockOffset, std::string data){
    blocks_[blockOffset / BLOCK_SIZE].status = Block::Status::Retrieved;
    retrieved_cnt++;
    blocks_[blockOffset / BLOCK_SIZE].data = data;
}

bool Piece::AllBlocksRetrieved() const{
    return retrieved_cnt == blocks_.size();
}

/*
* Получить скачанные данные для части файла
*/
std::string Piece::GetData() const{
    std::string all_data;
    for(auto& block: blocks_){
        all_data += block.data;
    }
    return all_data;
}

/*
* Посчитать хеш по скачанным данным
*/
std::string Piece::GetDataHash() const{
    return CalculateSHA1(GetData());
}

/*
* Получить хеш для части из .torrent файла
*/
const std::string& Piece::GetHash() const{
    return hash_;
}


/*
* Удалить все скачанные данные и отметить все блоки как Missing
*/
void Piece::Reset(){
    for(auto& block: blocks_){
        block.data.clear();
        block.status = Block::Status::Missing;
        retrieved_cnt = 0;
    }
}