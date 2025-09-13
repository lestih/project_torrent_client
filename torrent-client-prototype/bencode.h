#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <variant>
#include <list>
#include <map>
#include <sstream>
#include <cstdint>

namespace Bencode {
        struct Type;
    
        using Vector = std::vector<Type>;
        using Map = std::map<std::string, Type>;
    
        struct Type {
            std::variant<int64_t, std::string, Vector, Map> val;  // используем variant для реализации полиморфизма 
        };
    
        Type Parser(const std::string& content, int64_t& pos, int64_t& start_hash, int64_t& finish_hash);
    }
