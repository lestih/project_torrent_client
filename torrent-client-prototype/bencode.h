#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <variant>
#include <list>
#include <map>
#include <sstream>

namespace Bencode {
        struct Type;
    
        using Vector = std::vector<Type>;
        using Map = std::map<std::string, Type>;
    
        struct Type {
            std::variant<long long, std::string, Vector, Map> val;
        };
    
        Type parser(const std::string& content, long long& pos, long long& start_hash, long long& finish_hash);
    }
