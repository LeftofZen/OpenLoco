#pragma once

#include "HeightMapRange.h"
#include "ITerrainGenerator.h"
#include "PngImage.h"
#include <OpenLoco/Core/FileSystem.hpp>

namespace OpenLoco::S5
{
    struct Options;
}

namespace OpenLoco::World::MapGenerator
{
    class PngTerrainGenerator : public ITerrainGenerator
    {
    public:
        void loadImage(const fs::path& path);
        void generateHeightMap(const S5::Options& options, HeightMapRange& heightMap);
        std::unique_ptr<PngImage> pngImage;
    };
}
