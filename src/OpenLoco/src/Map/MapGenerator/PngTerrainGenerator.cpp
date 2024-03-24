#include "PngTerrainGenerator.h"
#include "Logging.h"
#include "MapGenerator.h"
#include "PngImage.h"
#include "S5/S5.h"
#include <OpenLoco/Engine/World.hpp>
#include <OpenLoco/Platform/Platform.h>
#include <png.h>

using namespace OpenLoco::World;
using namespace OpenLoco::Diagnostics;

namespace OpenLoco::World::MapGenerator
{
    void PngTerrainGenerator::loadImage(const fs::path& path)
    {
        if (!fs::is_regular_file(path))
        {
            Logging::error("Can't find terrain file ({})", path);
            return;
        }

        pngImage = PngOps::loadPng(path.string());
        if (pngImage == nullptr)
        {
            Logging::error("Can't load terrain file ({})", path);
            return;
        }
    }

    void PngTerrainGenerator::generateHeightMap(const S5::Options& options, HeightMapRange& heightMap)
    {
        if (pngImage == nullptr)
        {
            Logging::error("png image was null");
            return;
        }

        const int maxHeightmapLevels = 64 - options.minLandHeight;
        const float scalingFactor = maxHeightmapLevels / 255.f;

        for (auto y = 0; y < World::kMapRows; y++)
        {
            for (auto x = 0; x < World::kMapColumns; x++)
            {
                if (y >= pngImage->height || x >= pngImage->width)
                {
                    heightMap[{ x, y }] = options.minLandHeight;
                }
                else
                {
                    png_byte red, green, blue, alpha;
                    pngImage->getPixel(x, y, red, green, blue, alpha);

                    auto imgHeight = std::max({ red, green, blue });
                    heightMap[{ x, y }] = options.minLandHeight + (imgHeight * scalingFactor);
                }
            }
        }
    }
}
