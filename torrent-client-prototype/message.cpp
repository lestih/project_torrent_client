#include "byte_tools.h"
#include "message.h"
#include <string>
#include <sstream>

Message Message::Parse(const std::string& messageString){ // изменить
    if(messageString.empty()) return {MessageId::KeepAlive, 0, ""};
    MessageId id = static_cast<MessageId>(messageString[0]);
    std::string payl = messageString.substr(1, messageString.size() - 1);
    return {id, messageString.size(), payl};
}

Message Message::Init(MessageId id, const std::string& payload){
    return {id, 1 + payload.size(), payload};
}

std::string Message::ToString() const{
    std::string data = IntToBytes(messageLength) + std::string(1, static_cast<char>(id)) + payload;
    return data;
}