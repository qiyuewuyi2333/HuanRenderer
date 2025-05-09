//
// Created by 86156 on 5/9/2025.
//
#include <cassert>
#include <fstream>
#include <huan/utils/file_system.hpp>

#include "huan/common.hpp"
#include "huan/log/Log.hpp"

namespace huan::engine
{

std::string FileSystem::Imp_loadFile(std::string_view filePath)
{
    std::ifstream file(filePath.data(), std::ios::ate);
    if (!file.is_open())
    {
        HUAN_CLIENT_BREAK("Failed to open file: %s", filePath);
    }
    size_t fileSize = (size_t)file.tellg();
    std::string result;
    result.resize(fileSize);
    
    file.seekg(0);
    file.read(result.data(), fileSize);
    file.close();
    
    HUAN_CORE_INFO("Loaded file: {}", filePath)
    return result;
}

std::vector<uint32_t> FileSystem::Imp_loadBinaryFileImp(std::string_view filePath)
{
    std::ifstream file(filePath.data(), std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        HUAN_CLIENT_BREAK("Failed to open file: %s", filePath);
    }

    size_t file_size = (size_t)file.tellg();
            
    std::vector<uint32_t> buffer(file_size / sizeof(uint32_t));
            
    file.seekg(0);

    file.read((char*)buffer.data(), file_size);
    file.close();

    HUAN_CORE_INFO("Loaded file: {}", filePath)
    return buffer;
}
}
