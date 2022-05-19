#include "./StringManager.h"
#include "./Interop/Interop.hpp"
#include <array>

using namespace OpenLoco::Interop;

namespace OpenLoco::StringManager
{
    static loco_global<string_id, 0x0112C826> _commonFormatArgs;

    // 0x0112C826
    template<>
    string_id getCommonFormatArgs<string_id>()
    {
        return *_commonFormatArgs;
    }

    template<>
    std::array<uint16_t, 10> getCommonFormatArgs<std::array<uint16_t, 10>>()
    {
        auto ptr = reinterpret_cast<uint16_t*>(&*_commonFormatArgs);
        return reinterpret_cast<std::array<uint16_t, 10>&>(ptr);
    }
    template<>
    std::array<char, 16> getCommonFormatArgs<std::array<char, 16>>()
    {
        auto ptr = reinterpret_cast<char*>(&*_commonFormatArgs);
        return reinterpret_cast<std::array<char, 16>&>(ptr);
    }
    template<>
    std::array<std::byte, 20> getCommonFormatArgs<std::array<std::byte, 20>>()
    {
        auto ptr = reinterpret_cast<std::byte*>(&*_commonFormatArgs);
        return reinterpret_cast<std::array<std::byte, 20>&>(ptr);
    }

    template<>
    char* getCommonFormatArgs<char*>()
    {
        auto ptr = reinterpret_cast<char*>(&*_commonFormatArgs);
        return ptr;
    }
    template<>
    uint8_t* getCommonFormatArgs<uint8_t*>()
    {
        auto ptr = reinterpret_cast<uint8_t*>(&*_commonFormatArgs);
        return ptr;
    }
    template<>
    int32_t getCommonFormatArgs<int32_t>()
    {
        return *reinterpret_cast<int32_t*>(&*_commonFormatArgs);
    }

    /*template<>
    void setCommonFormatArgs<string_id>(string_id str)
    {
        *_commonFormatArgs = str;
    }*/
    /* template<>
     void setCommonFormatArgs<uint16_t[10]>(uint16_t[10] str)
     {
         *_commonFormatArgs = str;
     }*/
}
