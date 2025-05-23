#pragma once

#include <OpenLoco/Engine/World.hpp>
#include <array>
#include <span>

namespace OpenLoco::World
{
    struct StationElement;
}
namespace OpenLoco::Paint
{
    struct PaintSession;

    void paintStation(PaintSession& session, const World::StationElement& elStation);

    void paintStationCargo(PaintSession& session, const World::StationElement& elStation, const uint8_t flags, const uint32_t cargoTypes, std::span<const std::array<World::Pos3, 2>> cargoOffsets, const int16_t offsetZ, const World::Pos3& boundBoxOffset, const World::Pos3& boundBoxSize);
}
