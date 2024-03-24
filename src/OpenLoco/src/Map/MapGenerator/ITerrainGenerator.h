#pragma once

namespace OpenLoco::S5
{
    struct Options;
}
namespace OpenLoco::World::MapGenerator
{
    class HeightMapRange;
}

class ITerrainGenerator
{
public:
    virtual void generateHeightMap(const OpenLoco::S5::Options& options, OpenLoco::World::MapGenerator::HeightMapRange& heightMap) = 0;
    // virtual void generateTerrain(const OpenLoco::S5::Options& options, OpenLoco::World::MapGenerator::HeightMapRange& heightMap) = 0;
};
