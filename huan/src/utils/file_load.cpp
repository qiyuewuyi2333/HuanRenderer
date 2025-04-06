//
// Created by 86156 on 4/5/2025.
//

#include "huan/utils/file_load.hpp"

#include <fstream>
#include <string>

#include "huan/log/Log.hpp"

namespace huan {
    namespace utils {
        std::vector<uint32_t> loadFile(const std::string& file_path)
        {
            std::ifstream file(file_path, std::ios::ate | std::ios::binary);
            if (!file.is_open())
            {
                HUAN_CLIENT_BREAK("Failed to open file: %s", file_path.c_str());
                return {};
            }

            size_t file_size = (size_t)file.tellg();
            
            std::vector<uint32_t> buffer(file_size / sizeof(uint32_t));
            
            file.seekg(0);

            file.read((char*)buffer.data(), file_size);
            file.close();

            HUAN_CORE_INFO("Loaded file: {}", file_path.c_str())
            return std::move(buffer);
        }
    }
}
