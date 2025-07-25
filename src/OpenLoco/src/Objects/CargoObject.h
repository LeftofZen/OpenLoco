#pragma once

#include "Object.h"
#include "Types.hpp"
#include <OpenLoco/Core/EnumFlags.hpp>
#include <span>

namespace OpenLoco
{
    namespace ObjectManager
    {
        struct DependentObjects;
    }

    enum class CargoObjectFlags : uint8_t
    {
        none = 0U,
        unk0 = 1U << 0,
        refit = 1U << 1,
        delivering = 1U << 2,
    };
    OPENLOCO_ENABLE_ENUM_OPERATORS(CargoObjectFlags);

    // TODO: Eventually perhaps make this a separate object type
    enum class CargoCategory : uint16_t
    {
        grain = 1,
        coal,
        ironOre,
        liquids,
        paper,
        steel,
        timber,
        goods,
        foods,
        livestock = 11,
        passengers,
        mail,

        // Note: Mods may (and do) use other numbers not covered by this list
        null = 0xFFFFU
    };

#pragma pack(push, 1)
    struct CargoObject
    {
        static constexpr auto kObjectType = ObjectType::cargo;

        StringId name;                 // 0x0
        uint16_t unitWeight;           // 0x2
        uint16_t cargoTransferTime;    // 0x4
        StringId unitsAndCargoName;    // 0x6
        StringId unitNameSingular;     // 0x8
        StringId unitNamePlural;       // 0xA
        uint32_t unitInlineSprite;     // 0xC
        CargoCategory cargoCategory;   // 0x10
        CargoObjectFlags flags;        // 0x12
        uint8_t numPlatformVariations; // 0x13
        uint8_t var_14;
        uint8_t premiumDays;       // 0x15
        uint8_t maxNonPremiumDays; // 0x16
        uint16_t nonPremiumRate;   // 0x17
        uint16_t penaltyRate;      // 0x19
        uint16_t paymentFactor;    // 0x1B
        uint8_t paymentIndex;      // 0x1D
        uint8_t unitSize;          // 0x1E

        bool validate() const;
        void load(const LoadedObjectHandle& handle, std::span<const std::byte> data, ObjectManager::DependentObjects*);
        void unload();

        constexpr bool hasFlags(CargoObjectFlags flagsToTest) const
        {
            return (flags & flagsToTest) != CargoObjectFlags::none;
        }
    };
#pragma pack(pop)

    static_assert(sizeof(CargoObject) == 0x1F);
    namespace Cargo::ImageIds
    {
        constexpr uint32_t kInlineSprite = 0;
        // There are numPlatformVariations sprites after this one
        constexpr uint32_t kStationPlatformBegin = 1;
    }
}
