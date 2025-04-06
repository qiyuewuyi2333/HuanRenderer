//
// Created by 86156 on 4/5/2025.
//

#ifndef FILE_LOAD_HPP
#define FILE_LOAD_HPP
#include <string>
#include <vector>

#include "huan/common.hpp"

namespace huan
{
    namespace utils
    {
        HUAN_API extern std::vector<uint32_t> loadFile(const std::string& file_path);
    }
}

#endif //FILE_LOAD_HPP
