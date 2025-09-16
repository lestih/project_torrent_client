#include "byte_tools.h"
#include <openssl/sha.h>
#include <vector>

int BytesToInt(std::string_view bytes) {  // перевод big endian в int
    // int32_t value;
    // memcpy(&value, bytes.data(), 4);
    // return ntohl(value);
    return ((static_cast<unsigned char>(bytes[0]) << 24) | (static_cast<unsigned char>(bytes[1]) << 16) | 
    (static_cast<unsigned char>(bytes[2]) << 8) | static_cast<unsigned char>(bytes[3]));
}

std::string IntToBytes(int num) {  // перевод int в big endian
    std::string ans;
    ans.resize(4);
    for (int i = 3;i >= 0;--i) {
        ans[i] = static_cast<char>(num & 0xFF);
        num >>= 8;
    }
    return ans;
}

std::string CalculateSHA1(const std::string& msg) {  // подсчет хэша
    unsigned char hash[20];
    SHA1((const unsigned char*)msg.c_str(), msg.size(), hash);
    std::string hash_ans;
    for (int i = 0;i<20;++i) {
        hash_ans.push_back(hash[i]);
    }
    return hash_ans;
}

std::string HexEncode(const std::string& input) {  // перевод сырых двоичных данных в шестнадцатеричный вид
    static const char hex_digits[] = "0123456789abcdef";
    std::string output;
    output.reserve(input.size() * 2);
    
    for (unsigned char byte : input) {
        output.push_back(hex_digits[byte >> 4]);
        output.push_back(hex_digits[byte & 0x0F]);
    }
    
    return output;
}