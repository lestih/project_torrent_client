#include "bencode.h"

namespace Bencode {

// Парсер bencode формата
Type Parser(const std::string& content, int64_t& position, int64_t& start_hash, int64_t& finish_hash){
    Type result;
    char current_char = content[position];
    switch (current_char) {
        case 'i': {  // int format: i<content>e
            ++position; 
            std::string int_string;
            while (content[position] != 'e') {
                int_string.push_back(content[position]);
                ++position;
            }

            result.val = std::stoll(int_string);
            ++position;
            break; 
        }
        case 'l': {  // list format: l<content>e
            Vector list;
            ++position;
            while (content[position] != 'e') {
                list.push_back(Parser(content, position, start_hash, finish_hash));
            }

            result.val = list;
            ++position;
            break;
        }
        case 'd': {  // dict format: d<key><value>e
            ++position;
            Map map;
            int64_t info_position;  // позиция начала данных info
            while (content[position] != 'e') {
                std::string key = std::get<std::string>(Parser(content, position, start_hash, finish_hash).val);
                info_position = position;

                map[key] = Parser(content, position, start_hash, finish_hash);
                if (key == "info") {  // если "info", то сохраняем позиции начала и конца
                    start_hash = info_position;
                    finish_hash = position - 1;
                }
            }

            result.val = map;
            ++position;
            break;
        }
        default: {  // string format: <length>:<content>
            std::string string;
            std::string size_string;
            int64_t size;

            while (content[position] != ':') {
                size_string.push_back(content[position]);
                ++position;
            }

            size = std::stoll(size_string);
            ++position;
            while (size != 0) {
                string.push_back(content[position]);
                ++position;
                --size;
            }
            result.val = string;
            break;
        }
    }
    
    return result;
}

}
