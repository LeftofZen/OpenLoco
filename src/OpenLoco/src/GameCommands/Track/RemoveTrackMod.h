#include "GameCommands/GameCommands.h"
#include "Map/Track/TrackModSection.h"

namespace OpenLoco::GameCommands
{
    struct TrackModsRemovalArgs
    {
        static constexpr auto command = GameCommand::removeTrackMod;

        TrackModsRemovalArgs() = default;
        explicit TrackModsRemovalArgs(const registers& regs)
            : pos(regs.ax, regs.cx, regs.di)
            , rotation(regs.bh & 0x3)
            , trackId(regs.dl & 0x3F)
            , index(regs.dh & 0x3)
            , type((regs.edi >> 16) & 0xF)
            , trackObjType(regs.ebp & 0xFF)
            , modSection(static_cast<World::Track::ModSection>((regs.ebp >> 16) & 0xFF))
        {
        }

        World::Pos3 pos;
        uint8_t rotation;
        uint8_t trackId;
        uint8_t index;
        uint8_t type;
        uint8_t trackObjType;
        World::Track::ModSection modSection;

        explicit operator registers() const
        {
            registers regs;
            regs.ax = pos.x;
            regs.cx = pos.y;
            regs.bh = rotation;
            regs.dl = trackId;
            regs.dh = index;
            regs.edi = pos.z | (type << 16);
            regs.ebp = trackObjType | (enumValue(modSection) << 16);
            return regs;
        }
    };

    void removeTrackMod(registers& regs);
}
