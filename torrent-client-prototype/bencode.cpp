#include "bencode.h"

namespace Bencode {

}

Bencode::Type Bencode::parser(const std::string& content, long long& pos, long long& start_hash, long long& finish_hash){
    Type result;
    char c = content[pos];
    switch(c){
        case 'i':{
            std::string temp;
            ++pos;
            while(content[pos] != 'e'){
                temp.push_back(content[pos]);
                ++pos;
            }
            ++pos;
            result.val = std::stoll(temp);
            break;
        }
        case 'l':{
            Vector temp;
            ++pos;
            while(content[pos] != 'e'){
                temp.push_back(parser(content, pos, start_hash, finish_hash));
            }
            ++pos;
            result.val = temp;
            break;
        }
        case 'd':{
            long long ps;
            Map map_temp;
            ++pos;
            while(content[pos] != 'e'){
                std::string temp = std::get<std::string>(parser(content, pos, start_hash, finish_hash).val);
                ps = pos;
                map_temp[temp] = parser(content, pos, start_hash, finish_hash);
                if(temp == "info"){
                    start_hash = ps;
                    finish_hash = pos - 1;
                }
            }
            ++pos;
            result.val = map_temp;
            break;
        }
        default:{
            std::string string_temp;
            std::string num_temp;
            long long num;
            while(content[pos] != ':'){
                num_temp.push_back(content[pos]);
                ++pos;
            }
            num = std::stoll(num_temp);
            ++pos;
            while(num != 0){
                string_temp.push_back(content[pos]);
                ++pos;
                --num;
            }
            result.val = string_temp;
            break;
        }
    }
    
    return result;
}
