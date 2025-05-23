#pragma once

#include "Economy/Currency.h"
#include "Types.hpp"
#include <OpenLoco/Core/EnumFlags.hpp>
#include <OpenLoco/Engine/World.hpp>

namespace OpenLoco
{
    enum class AiThinkState : uint8_t
    {
        unk0,
        unk1,
        unk2,
        unk3,
        unk4,
        unk5,
        unk6,
        unk7,
        unk8,
        unk9,
        endCompany,
    };

    enum class AiPlaceVehicleState : uint8_t
    {
        begin,
        resetList,
        place,
        restart,
    };

    enum class AiThoughtType : uint8_t
    {
        unk0,
        unk1,
        unk2,
        unk3,
        unk4,
        unk5,
        unk6,
        unk7,
        unk8,
        unk9,
        unk10,
        unk11,
        unk12,
        unk13,
        unk14,
        unk15,
        unk16,
        unk17,
        unk18,
        unk19,

        null = 0xFF
    };
    constexpr auto kAiThoughtTypeCount = 20U;

    enum class AiThoughtStationFlags : uint8_t
    {
        none = 0U,

        aiAllocated = 1U << 0,
        operational = 1U << 1,
    };
    OPENLOCO_ENABLE_ENUM_OPERATORS(AiThoughtStationFlags);

    enum class AiPurchaseFlags : uint8_t
    {
        none = 0,
        unk0 = 1U << 0,
        unk1 = 1U << 1,
        unk2 = 1U << 2,
        requiresMods = 1U << 3, // can be track or road mods
        unk4 = 1U << 4,
    };
    OPENLOCO_ENABLE_ENUM_OPERATORS(AiPurchaseFlags);

    constexpr auto kMaxAiThoughts = 60U;
    constexpr auto kAiThoughtIdNull = 0xFFU;

#pragma pack(push, 1)
    struct AiThought
    {
        struct Station
        {
            StationId id;                 // 0x0
            AiThoughtStationFlags var_02; // 0x2 flags?
            uint8_t rotation;             // 0x3
            World::Pos2 pos;              // 0x4
            uint8_t baseZ;                // 0x8
            uint8_t var_9;                // 0x9 aiStationIndex
            uint8_t var_A;                // 0xA aiStationIndex
            uint8_t var_B;                // 0xB
            uint8_t var_C;                // 0xC
            uint8_t pad_D[0xE - 0xD];

            constexpr bool hasFlags(AiThoughtStationFlags flags) const { return (var_02 & flags) != AiThoughtStationFlags::none; }
        };
        static_assert(sizeof(Station) == 0xE);
        AiThoughtType type;    // 0x00 0x4A8
        uint8_t destinationA;  // 0x01 0x4A9 either a TownId or IndustryId
        uint8_t destinationB;  // 0x02 0x4AA either a TownId or IndustryId
        uint8_t numStations;   // 0x03 0x4AB size of stations
        uint8_t stationLength; // 0x04 0x4AC station length
        uint8_t pad_05;
        Station stations[4];  // 0x06 0x4AE Will lists stations created that vehicles will route to
        uint8_t trackObjId;   // 0x3E 0x4E6 track or road (with high bit set)
        uint8_t rackRailType; // 0x3F 0x4E7 Is 0xFFU for no rack rail
        uint16_t mods;        // 0x40 0x4E8 track or road
        uint8_t cargoType;    // 0x42 0x4EA
        uint8_t var_43;       // 0x4EB
        uint8_t numVehicles;  // 0x44 0x4EC size of var_66
        uint8_t var_45;       // 0x4ED size of var_46
        uint16_t var_46[16];  // 0x4EF array of uint16_t object id
        EntityId vehicles[8]; // 0x66 0x50E see also numVehicles for current size
        currency32_t var_76;  // 0x51E
        uint8_t pad_7A[0x7C - 0x7A];
        currency32_t var_7C;           // 0x524
        currency32_t var_80;           // 0x528
        currency32_t var_84;           // 0x52C
        uint8_t var_88;                // 0x530
        uint8_t stationObjId;          // 0x89 0x531 Could be either Airport/Dock/TrainStation/RoadStation
        uint8_t signalObjId;           // 0x8A 0x532 Can be 0xFFU for n
        AiPurchaseFlags purchaseFlags; // 0x8B 0x533

        constexpr bool hasPurchaseFlags(AiPurchaseFlags flags) const { return (purchaseFlags & flags) != AiPurchaseFlags::none; }

        // Converts the TownId or IndustryId of destinationA into the center position of the destination.
        World::Pos2 getDestinationPositionA() const;
        // Converts the TownId or IndustryId of destinationB into the center position of the destination.
        World::Pos2 getDestinationPositionB() const;
    };
#pragma pack(pop)
    static_assert(sizeof(AiThought) == 0x8C);

    void aiThink(CompanyId id);

    void setAiObservation(CompanyId id);
    void removeEntityFromThought(AiThought& thought, EntityId id);
}
