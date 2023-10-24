#include "ChannelManager.h"
#include "Logging.h"
#include <OpenLoco/Core/Exception.hpp>

using namespace OpenLoco::Diagnostics;

namespace OpenLoco::Audio
{
    VirtualChannel::VirtualChannel(float gain)
        : gain(gain)
    {
    }

    void VirtualChannel::disposeChannels()
    {
        channels.clear();
    }

    void VirtualChannel::stopChannels()
    {
        for (auto& channel : channels)
        {
            channel->stop();
        }
    }

    Channel& VirtualChannel::getFirstChannel()
    {
        return *(channels.front());
    }

    static std::unique_ptr<Channel> makeNewChannel(OpenAL::SourceManager& sourceManager, ChannelId channelId)
    {
        if (channelId == ChannelId::vehicle)
            // return new VehicleChannel(sourceManager.allocate());
            throw Exception::NotImplemented();
        else
            return std::make_unique<Channel>(sourceManager.allocate());
    }

    void ChannelManager::initialise(OpenAL::SourceManager& _sourceManager)
    {
        Logging::verbose("ChannelManager::initialise()");

        this->sourceManager = &_sourceManager;

        virtualChannels.reserve(kVirtualChannelDefinitions.size());

        for (const auto& channelDef : kVirtualChannelDefinitions)
        {
            auto result = virtualChannels.emplace(channelDef.first, VirtualChannel(1.f));

            if (!result.second)
            {
                continue;
            }

            auto& virtualChannel = result.first->second;

            // reserve max (instead of initial) to avoid resizing when allocating new channels
            virtualChannel.channels.reserve(channelDef.second.maxPhysicalChannels);

            for (size_t i = 0; i < channelDef.second.initialPhysicalChannels; ++i)
            {
                virtualChannel.channels.push_back(makeNewChannel(*sourceManager, channelDef.first));
            }
        }
    }

    void ChannelManager::disposeChannels()
    {
        Logging::verbose("ChannelManager::disposeChannels()");
        for (auto& virtualChannel : virtualChannels)
        {
            virtualChannel.second.disposeChannels();
        }
    }

    VirtualChannel& ChannelManager::getVirtualChannel(ChannelId channelId)
    {
        return virtualChannels[channelId];
    }

    // previously 0x00401A05
    void ChannelManager::stopChannel(ChannelId channelId)
    {
        Logging::verbose("ChannelManager::stopChannel({})", static_cast<int>(channelId));
        virtualChannels[channelId].stopChannels();
    }

    void ChannelManager::setVolume(ChannelId channelId, float volume)
    {
        for (auto& channel : virtualChannels[channelId].channels)
        {
            channel->setVolume(volume);
        }
    }

}
