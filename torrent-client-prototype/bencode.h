#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <variant>
#include <list>
#include <map>
#include <sstream>

namespace Bencode {

    /*
     * В это пространство имен рекомендуется вынести функции для работы с данными в формате bencode.
     * Этот формат используется в .torrent файлах и в протоколе общения с трекером
     */
        struct Type;
    
        using Vector = std::vector<Type>;
        using Map = std::map<std::string, Type>;
    
        struct Type {
            std::variant<long long, std::string, Vector, Map> val;
        };
    
        Type parser(const std::string& content, long long& pos, long long& start_hash, long long& finish_hash);
    }
