#include "piece_storage.h"
#include <iostream>

PieceStorage::PieceStorage(const TorrentFile& tf, const std::filesystem::path& outputDirectory, size_t count): pieces_cnt(count),
 outputDirectory_(outputDirectory), piece_length_(tf.pieceLength){
    for(size_t i = 0;i<pieces_cnt;++i){
        if(i == tf.pieceHashes.size() - 1 && tf.length % tf.pieceLength != 0) 
            remainPieces_.push(std::make_shared<Piece>(Piece(i, tf.length % tf.pieceLength, tf.pieceHashes[i])));
        else remainPieces_.push(std::make_shared<Piece>(Piece(i, tf.pieceLength, tf.pieceHashes[i])));
    }

    std::filesystem::path outputFilePath_ = outputDirectory_ / tf.name;
    outputFile_.open(outputFilePath_, std::ios::binary | std::ios::out);

    outputFile_.seekp(tf.length - 1);
    outputFile_.write("", 1);
    outputFile_.seekp(0);
}

PiecePtr PieceStorage::GetNextPieceToDownload() {
    std::lock_guard<std::mutex> lock_(m_);

    if (remainPieces_.empty()) return nullptr;
    PiecePtr temp = remainPieces_.front();
    remainPieces_.pop();
    return temp;
}

void PieceStorage::PieceProcessed(const PiecePtr& piece) {// выполняется под Lock
    std::lock_guard<std::mutex> lock_(m_);
    this->SavePieceToDisk(piece);
}

bool PieceStorage::QueueIsEmpty() const {
    std::lock_guard<std::mutex> lock_(m_);
    return remainPieces_.empty();
}

size_t PieceStorage::PiecesSavedToDiscCount() const{
    std::lock_guard<std::mutex> lock_(m_);
    return saved_.size();
}

size_t PieceStorage::TotalPiecesCount() const {
    return pieces_cnt;
}

/*
    * Закрыть поток вывода в файл
    */
void PieceStorage::CloseOutputFile(){
    std::lock_guard<std::mutex> lock_(m_);
    if (outputFile_.is_open()) {
        outputFile_.close();
    }
}

/*
    * Отдает список номеров частей файла, которые были сохранены на диск
    */
const std::vector<size_t>& PieceStorage::GetPiecesSavedToDiscIndices() const{
    std::lock_guard<std::mutex> lock_(m_);
    return saved_;
}

/*
    * Сколько частей файла в данный момент скачивается
    */
size_t PieceStorage::PiecesInProgressCount() const{
    std::lock_guard<std::mutex> lock_(m_);
    return TotalPiecesCount() - remainPieces_.size() - saved_.size();
}

void PieceStorage::SavePieceToDisk(const PiecePtr& piece){
    outputFile_.seekp(piece_length_ * piece->GetIndex());
    outputFile_.write(piece->GetData().data(), piece->GetData().size());
    saved_.push_back(piece->GetIndex());
}
