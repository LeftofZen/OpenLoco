#pragma once
#include "Channel.h"
#include "OpenAL.h"
#include <cstdint>
#include <memory>
#include <unordered_map>

namespace OpenLoco::Audio
{

    enum class ChannelId
    {
        music,
        unk_1,
        ambient,
        title_deprecated,
        vehicle, // * 10
        soundFX, // * 64
    };
    constexpr int32_t kNumReservedChannels = 4 + 10;

    struct VirtualChannelAttributes
    {
        size_t initialPhysicalChannels;
        size_t maxPhysicalChannels;
    };

    // constexpr?
    static const std::unordered_map<ChannelId, VirtualChannelAttributes> kVirtualChannelDefinitions = {
        { ChannelId::music, VirtualChannelAttributes{ 1, 1 } },
        //{ ChannelId::unk_1, VirtualChannelAttributes{ 0, 0 } },
        { ChannelId::ambient, VirtualChannelAttributes{ 1, 1 } },
        //{ ChannelId::title_deprecated, VirtualChannelAttributes{ 0, 0 } },
        //{ ChannelId::vehicle, VirtualChannelAttributes{ 10, 64 } },
        { ChannelId::soundFX, VirtualChannelAttributes{ 16, 64 } },
    };

    // aka a 'mixer' channel
    class VirtualChannel
    {
    public:
        float gain;
        std::vector<std::unique_ptr<Channel>> channels;

        VirtualChannel(float gain = 1.f);

        void disposeChannels();
        void stopChannels();
        Channel& getFirstChannel();
    };

    struct PlaySoundParams
    {
        uint32_t sample{};
        int32_t volume{};
        int32_t pan{};
        int32_t freq{};
        bool loop{};
    };

    class ChannelManager
    {
        std::unordered_map<ChannelId, VirtualChannel> virtualChannels;
        OpenAL::SourceManager* sourceManager;

    public:
        ChannelManager(){};
        ChannelManager(const ChannelManager&) = delete;
        ChannelManager& operator=(const ChannelManager&) = delete;

        void initialise(OpenAL::SourceManager&);
        void disposeChannels();

        VirtualChannel& getVirtualChannel(ChannelId);
        void stopChannel(ChannelId);
        void setVolume(ChannelId, float volume);
    };

}
