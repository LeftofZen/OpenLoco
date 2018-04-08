#include "argswrapper.hpp"
#include "stringmgr.h"
#include "../config.h"
#include "../interop/interop.hpp"
#include "../townmgr.h"

#include <cassert>
#include <cstring>
#include <cstdio>
#include <stdexcept>

using namespace openloco::interop;

namespace openloco::stringmgr
{
    const uint16_t NUM_USER_STRINGS   = 2048;
    const uint8_t  USER_STRING_SIZE   = 32;
    const uint16_t USER_STRINGS_START = 0x8000;
    const uint16_t USER_STRINGS_END   = USER_STRINGS_START + NUM_USER_STRINGS;

    const uint16_t NUM_TOWN_NAMES     = 345;
    const uint16_t TOWN_NAMES_START   = 0x9EE7;
    const uint16_t TOWN_NAMES_END     = TOWN_NAMES_START + NUM_TOWN_NAMES;

    static loco_global<char * [0xFFFF], 0x005183FC> _strings;
    static loco_global<char [NUM_USER_STRINGS][USER_STRING_SIZE], 0x0095885C> _userStrings;

    const char* get_string(string_id id)
    {
        printf("Fetching string %d\n", id);
        char* str = _strings[id];
        printf("Found at %p\n", str);

        return str;
    }

    // TODO: decltype(value)
    static char* format_comma(uint32_t value, char* buffer)
    {
        registers regs;
        regs.eax = value;
        regs.edi = (uint32_t) buffer;

        call(0x00495F35, regs);
        return (char*) regs.edi;
    }

    // TODO: decltype(value)
    static char* format_int(uint32_t value, char* buffer)
    {
        registers regs;
        regs.eax = value;
        regs.edi = (uint32_t) buffer;

        call(0x495E2A, regs);
        return (char*) regs.edi;
    }

    // TODO: decltype(value)
    static char* formatNumeric_4(uint16_t value, char* buffer)
    {
        registers regs;
        regs.eax = (uint32_t) value;
        regs.edi = (uint32_t) buffer;

        call(0x4963FC, regs);
        return (char*) regs.edi;
    }

    // TODO: decltype(value)
    static char* format_comma2dp32(uint32_t value, char* buffer)
    {
        registers regs;
        regs.eax = value;
        regs.edi = (uint32_t) buffer;

        call(0x4962F1, regs);
        return (char*) regs.edi;
    }

    static char* formatDayMonthYearFull(uint32_t value, char* buffer)
    {
        registers regs;
        regs.eax = (uint32_t) value;
        regs.edi = (uint32_t) buffer;

        call(0x495CB5, regs);
        return (char*) regs.edi;
    }

    static char* formatMonthYearFull(uint32_t value, char* buffer)
    {
        registers regs;
        regs.eax = (uint32_t) value;
        regs.edi = (uint32_t) buffer;

        call(0x495D77, regs);
        return (char*) regs.edi;
    }

    static char* formatMonthYearAbbrev_0(uint32_t value, char* buffer)
    {
        registers regs;
        regs.eax = (uint32_t) value;
        regs.edi = (uint32_t) buffer;

        call(0x495DC7, regs);
        return (char*) regs.edi;
    }

    static char* format_string(char* buffer, string_id id, argswrapper &args);

    static char* format_string_part(char* buffer, const char* sourceStr, argswrapper &args)
    {
        while (true)
        {
            uint8_t ch = *sourceStr;
            sourceStr++;
            if (ch == 0)
            {
                *buffer = '\0';
                return buffer;
            }
            else if (ch <= 4)
            {
                *buffer = ch;
                buffer++;

                ch = *sourceStr;
                sourceStr++;

                *buffer = ch;
                buffer++;
            }
            else if (ch <= 16)
            {
                *buffer = ch;
                buffer++;
            }
            else if (ch <= 0x1F)
            {
                if (ch > 22)
                {
                    *buffer = ch;
                    buffer++;

                    ch = *sourceStr;
                    sourceStr++;

                    *buffer = ch;
                    buffer++;

                    ch = *sourceStr;
                    sourceStr++;

                    *buffer = ch;
                    buffer++;
                }

                ch = *sourceStr;
                sourceStr++;

                *buffer = ch;
                buffer++;

                ch = *sourceStr;
                sourceStr++;

                *buffer = ch;
                buffer++;
            }
            else if (ch < '}') // 0x7B
            {
                *buffer = ch;
                buffer++;
            }
            else if (ch < 0x90)
            {
                switch (ch)
                {
                    case 123 + 0:
                    {
                        uint32_t value = args.pop32();
                        buffer = format_comma(value, buffer);
                        break;
                    }

                    case 123 + 1:
                    {
                        uint32_t value = args.pop32();
                        buffer = format_int(value, buffer);
                        break;
                    }

                    case 123 + 2:
                    {
                        uint16_t value = args.pop16();
                        buffer = formatNumeric_4(value, buffer);
                        break;
                    }

                    case 123 + 3:
                    {
                        uint32_t value = args.pop32();
                        buffer = format_comma2dp32(value, buffer);
                        break;
                    }

                    case 123 + 4:
                    {
                        uint16_t value = args.pop16();
                        buffer = format_comma(value, buffer);
                        break;
                    }

                    case 123 + 5:
                    {
                        uint16_t value = args.pop16();
                        buffer = format_int((uint32_t) value, buffer);
                        break;
                    }

                    case 123 + 6:
                    {
                        args.pop32();
                        // !!! TODO: implement and call sub_495B66
                        printf("Unimplemented format string: 6\n");
                        break;
                    }

                    case 123 + 7:
                    {
                        args.pop32();
                        args.pop16();
                        // !!! TODO: implement and call sub_495B5B
                        printf("Unimplemented format string: 7\n");
                        break;
                    }

                    case 123 + 8:
                    {
                        string_id id = args.pop16();
                        const char* sourceStr_ = sourceStr;
                        buffer = format_string(buffer, id, args);
                        sourceStr = sourceStr_;
                        break;
                    }

                    case 123 + 9:
                    {
                        string_id id = *(string_id*) sourceStr;
                        sourceStr += 2;
                        const char* sourceStr_ = sourceStr;
                        buffer = format_string(buffer, id, args);
                        sourceStr = sourceStr_;
                        break;
                    }

                    case 123 + 10:
                    {
                        const char* sourceStr_ = sourceStr;
                        sourceStr = (char*) args.pop32();

                        do
                        {
                            *buffer = *sourceStr;
                            buffer++;
                            sourceStr++;
                        }
                        while (*sourceStr != '\0');

                        buffer--;
                        sourceStr = sourceStr_;
                        break;
                    }

                    case 123 + 11:
                    {
                        char modifier = *sourceStr;
                        uint8_t value = args.pop8();
                        sourceStr++;

                        switch (modifier)
                        {
                            case 0:
                                buffer = formatDayMonthYearFull(value, buffer);
                                break;

                            case 4:
                                buffer = formatMonthYearFull(value, buffer);
                                break;

                            case 8:
                                buffer = formatMonthYearAbbrev_0(value, buffer);
                                break;

                            default:
                               throw std::out_of_range("format_string: unexpected modifier: " + std::to_string((uint8_t) modifier));
                        }

                        break;
                    }

                    case 123 + 12:
                    {
                        // velocity
                        auto measurement_format = config::get().measurement_format;

                        uint32_t value = args.pop16();

                        const char* unit;
                        if (measurement_format == config::measurement_formats::FORMAT_IMPERIAL)
                        {
                            // !!! TODO: Move to string id
                            unit = "mph";
                        }
                        else
                        {
                            // !!! TODO: Move to string id
                            unit = "kmh";
                            value = (value * 1648) >> 10;
                        }

                        buffer = format_comma(value, buffer);

                        do
                        {
                            *buffer = *unit;
                            buffer++;
                            unit++;
                        }
                        while (*unit != '\0');

                        buffer--;

                        break;
                    }

                    case 123 + 13:
                        // pop16
                        args.pop16();
                        break;

                    case 123 + 14:
                        // push16
                        args.push16();
                        break;

                    case 123 + 15:
                        // timeMS
                        args.pop16();
                        // !!! TODO: implement timeMS
                        printf("Unimplemented format string: 15\n");
                        break;

                    case 123 + 16:
                        // timeHM
                        args.pop16();
                        // !!! TODO: implement timeHM
                        printf("Unimplemented format string: 16\n");
                        break;

                    case 123 + 17:
                    {
                        // distance
                        uint32_t value = args.pop16();
                        auto measurement_format = config::get().measurement_format;

                        const char* unit;
                        if (measurement_format == config::measurement_formats::FORMAT_IMPERIAL)
                        {
                            // !!! TODO: Move to string id
                            unit = "ft";
                        }
                        else
                        {
                            // !!! TODO: Move to string id
                            unit = "m";
                            value = (value * 840) >> 8;
                        }

                        buffer = format_comma(value, buffer);

                        do
                        {
                            *buffer = *unit;
                            buffer++;
                            unit++;
                        }
                        while (*unit != '\0');

                        buffer--;

                        break;
                    }

                    case 123 + 18:
                    {
                        // height
                        uint32_t value = args.pop16();

                        bool show_height_as_units = config::get().flags & config::flags::SHOW_HEIGHT_AS_UNITS;
                        uint8_t measurement_format = config::get().measurement_format;
                        const char* unit;

                        if (show_height_as_units)
                        {
                            // !!! TODO: move to string id
                            unit = " units";
                        }
                        else if (measurement_format == config::measurement_formats::FORMAT_IMPERIAL)
                        {
                            // !!! TODO: Move to string id
                            unit = "ft";
                            value *= 10;
                        }
                        else
                        {
                            // !!! TODO: Move to string id
                            unit = "m";
                            value *= 5;
                        }

                        buffer = format_comma(value, buffer);

                        do
                        {
                            *buffer = *unit;
                            buffer++;
                            unit++;
                        }
                        while (*unit != '\0');

                        buffer--;

                        break;
                    }

                    case 123 + 19:
                    {
                        // power
                        uint32_t value = args.pop16();
                        auto measurement_format = config::get().measurement_format;

                        const char* unit;
                        if (measurement_format == config::measurement_formats::FORMAT_IMPERIAL)
                        {
                            // !!! TODO: Move to string id
                            unit = "hp";
                        }
                        else
                        {
                            // !!! TODO: Move to string id
                            unit = "kW";
                            value = (value * 764) >> 10;
                        }

                        buffer = format_comma(value, buffer);

                        do
                        {
                            *buffer = *unit;
                            buffer++;
                            unit++;
                        }
                        while (*unit != '\0');

                        buffer--;

                        break;
                    }

                    case 123 + 20:
                    {
                        // sprite
                        *buffer = 23;
                        uint32_t value = args.pop32();
                        *(buffer + 1) = value;
                        buffer += 5;
                        break;
                    }
                }
            }
            else
            {
                *buffer = ch;
                buffer++;
            }
        }

        return buffer;
    }

    static char* format_string_part(char* buffer, const char* sourceStr, void* args)
    {
        auto wrapped = argswrapper(args);
        return format_string_part(buffer, sourceStr, wrapped);
    }

    // 0x004958C6
    static char* format_string(char* buffer, string_id id, argswrapper &args)
    {
        if (id < USER_STRINGS_START)
        {
            const char* sourceStr = get_string(id);
            if (sourceStr == nullptr || sourceStr == (char*) 0x50)
            {
                printf("Got a nullptr for string id %d -- cowardly refusing\n", id);
                return buffer;
            }

            buffer = format_string_part(buffer, sourceStr, args);
            assert(*buffer == '\0');
            return buffer;
        }
        else if (id < USER_STRINGS_END)
        {
            id -= USER_STRINGS_START;
            args.pop16();
            const char* sourceStr = _userStrings[id];

            // !!! TODO: original code is prone to buffer overflow.
            buffer = strncpy(buffer, sourceStr, USER_STRING_SIZE);
            buffer[USER_STRING_SIZE - 1] = '\0';
            buffer += strlen(sourceStr);

            return buffer;
        }
        else if (id < TOWN_NAMES_END)
        {
            id -= TOWN_NAMES_START;
            uint16_t town_id = args.pop16();
            auto town = townmgr::get(town_id);
            void* town_name = (void*) &town->name;
            return format_string(buffer, id, town_name);
        }
        else if (id == TOWN_NAMES_END)
        {
            auto temp = args;
            uint16_t town_id = args.pop16();
            auto town = townmgr::get(town_id);
            buffer = format_string(buffer, town->name, nullptr);
            args = temp;
            return buffer;
        }
        else
        {
            // throw std::out_of_range("format_string: invalid string id: " + std::to_string((uint32_t) id));
            printf("Invalid string id: %d\n", (uint32_t) id);
            return buffer;
        }
    }

    char* format_string(char* buffer, string_id id, void* args)
    {
        auto wrapped = argswrapper(args);
        return format_string(buffer, id, wrapped);
    }
}
