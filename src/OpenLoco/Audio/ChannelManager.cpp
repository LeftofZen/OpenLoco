#include "ChannelManager.h"
#include "VehicleChannel.h"
#include <algorithm>
#include <functional>
#include <iterator>

namespace OpenLoco::Audio
{
    static Channel* makeNewChannel(OpenAL::SourceManager& sourceManager, ChannelId channelId)
    {
        // if (channelId == ChannelId::vehicle)
        //     return new VehicleChannel(sourceManager.allocate());
        // else
        return new Channel(sourceManager.allocate());
    }

    ChannelManager::ChannelManager(OpenAL::SourceManager& sourceManager)
    {
        _sourceManager = sourceManager;

        virtualChannels.reserve(kVirtualChannelDefinitions.size());

        for (const auto& channelDef : kVirtualChannelDefinitions)
        {
            auto result = virtualChannels.emplace(channelDef.first, VirtualChannel(1.f));

            if (result.second)
            {
                auto& virtualChannel = result.first->second;

                // reserve max (instead of initial) to avoid resizing when allocating new channels
                virtualChannel.channels.reserve(channelDef.second.maxPhysicalChannels);

                for (size_t i = 0; i < channelDef.second.initialPhysicalChannels; ++i)
                {
                    virtualChannel.channels.push_back(makeNewChannel(_sourceManager, channelDef.first));
                }
            }
        }

        // stopChannels(ChannelId::vehicle);
    }

    Channel* ChannelManager::getFreeChannel(ChannelId channelId)
    {
        auto& channels = virtualChannels.at(channelId).channels;

        auto freeChannel = std::find_if(
            std::begin(channels),
            std::end(channels),
            [&](auto& channel) {
                return !channel->isPlaying();
            });

        Channel* channel = nullptr;

        if (freeChannel == std::end(channels))
        {
            if (channels.size() < kVirtualChannelDefinitions.at(channelId).maxPhysicalChannels)
            {
                channel = makeNewChannel(_sourceManager, channelId);
                channels.push_back(channel);
            }
            // else - no free channels and we've hit the channel limit - sound just won't play
        }
        else
        {
            channel = *freeChannel;
        }
        return channel;
    }

    bool ChannelManager::play(ChannelId channelId, PlaySoundParams&& soundParams)
    {
        auto channel = getFreeChannel(channelId);

        if (channel != nullptr && channel->load(soundParams.sample))
        {
            channel->setVolume(soundParams.volume);
            channel->setFrequency(soundParams.freq);
            channel->setPan(soundParams.pan);
            return channel->play(soundParams.loop);
        }

        return false;
    }

    bool ChannelManager::play(ChannelId channelId, PlaySoundParams&& soundParams, EntityId entityId)
    {
        // auto vehicleChannel = *static_cast<VehicleChannel*>(getFreeChannel(ChannelId::vehicle));
        // vehicleChannel.begin(entityId);
        return true;
    }

    VirtualChannel& ChannelManager::getVirtualChannel(ChannelId channelId)
    {
        return virtualChannels.at(channelId);
    }

    Channel& ChannelManager::getFirstChannel(ChannelId channelId)
    {
        return *(virtualChannels.at(channelId).channels.front());
    }

    void ChannelManager::disposeChannels()
    {
        virtualChannels.clear();
    }

    void ChannelManager::stopChannels(ChannelId channelId)
    {
        for (auto& channel : virtualChannels.at(channelId).channels)
        {
            channel->stop();
        }
    }

    void ChannelManager::cleanupNonPlayingChannels(ChannelId channelId)
    {
        for (auto& channel : virtualChannels.at(channelId).channels)
        {
            if (!channel->isPlaying())
            {

                channel->stop(); // This forces deallocation of buffer
            }
        }
    }
    void ChannelManager::updateVehicleChannels()
    {
        /*for (auto& channel : virtualChannels.at(ChannelId::vehicle).channels)
        {
            auto vehicleChannel = *static_cast<VehicleChannel*>(channel);
            vehicleChannel.update();
        }*/
    }

    void ChannelManager::stopVehicleNoise(EntityId id)
    {
        // for (auto& channel : virtualChannels.at(ChannelId::vehicle).channels)
        //{
        /*    auto vehicleChannel = *static_cast<VehicleChannel*>(channel);
            if (vehicleChannel.getId() == id)
                vehicleChannel.stop();
        }*/
    }

    VirtualChannel::VirtualChannel(float gain)
        : gain(gain)
    {
    }
}
