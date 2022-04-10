#include "./Misc.h"
#include "../GameState.h"

namespace OpenLoco
{
    // 0x00525E28
    uint32_t& gameStateFlags()
    {
        return getGameState().flags;
    }

    // 0x0052622E
    uint16_t& gameStateVar416()
    {
        return getGameState().var_416;
    }
}