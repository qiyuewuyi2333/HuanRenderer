//
// Created by 86156 on 5/9/2025.
//

#ifndef FILE_SYSTEM_HPP
#define FILE_SYSTEM_HPP
#include <string_view>
#include <vector>

#include "huan/common_templates/deferred_system.hpp"


namespace huan::runtime
{
class FileSystem : public DeferredSystem<FileSystem>
{
public:
    inline static std::string loadFile(std::string_view filePath)
    {
        return getInstance()->Imp_loadFile(filePath);
    }
    inline static std::vector<uint32_t> loadBinaryFile(std::string_view filePath)
    {
        return getInstance()->Imp_loadBinaryFileImp(filePath);
    }
    

private:
    std::string Imp_loadFile(std::string_view filePath);
    std::vector<uint32_t> Imp_loadBinaryFileImp(std::string_view filePath);
};

}
#endif //FILE_SYSTEM_HPP
