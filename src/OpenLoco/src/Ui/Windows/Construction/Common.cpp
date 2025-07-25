#include "Construction.h"
#include "Date.h"
#include "GameCommands/GameCommands.h"
#include "GameState.h"
#include "Graphics/Colour.h"
#include "Graphics/ImageIds.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/SoftwareDrawingEngine.h"
#include "Input.h"
#include "Map/MapSelection.h"
#include "Map/RoadElement.h"
#include "Map/TileElement.h"
#include "Map/Track/TrackData.h"
#include "Map/TrackElement.h"
#include "Objects/AirportObject.h"
#include "Objects/BridgeObject.h"
#include "Objects/DockObject.h"
#include "Objects/InterfaceSkinObject.h"
#include "Objects/ObjectManager.h"
#include "Objects/ObjectUtils.h"
#include "Objects/RoadExtraObject.h"
#include "Objects/RoadObject.h"
#include "Objects/RoadStationObject.h"
#include "Objects/TrackExtraObject.h"
#include "Objects/TrackObject.h"
#include "Objects/TrainSignalObject.h"
#include "Objects/TrainStationObject.h"
#include "SceneManager.h"
#include "Ui/ToolManager.h"
#include "Ui/Widget.h"
#include "World/CompanyManager.h"
#include "World/StationManager.h"

using namespace OpenLoco::Interop;
using namespace OpenLoco::World;
using namespace OpenLoco::World::TileManager;

namespace OpenLoco::Ui::Windows::Construction
{
    static Window* nonTrackWindow()
    {
        auto window = WindowManager::find(WindowType::construction);

        if (window != nullptr)
        {
            Common::setDisabledWidgets(window);
        }

        window = WindowManager::find(WindowType::construction);

        if (window != nullptr)
        {
            window->callOnMouseUp(Common::widx::tab_station, window->widgets[Common::widx::tab_station].id);
        }
        return window;
    }

    static Window* trackWindow()
    {
        auto window = WindowManager::find(WindowType::construction);

        if (window != nullptr)
        {
            Common::setDisabledWidgets(window);
        }

        Construction::activateSelectedConstructionWidgets();
        window = WindowManager::find(WindowType::construction);

        if (window != nullptr)
        {
            window->callOnMouseUp(Construction::widx::rotate_90, window->widgets[Construction::widx::rotate_90].id);
        }

        return window;
    }

    static Window* createTrackConstructionWindow()
    {
        Common::createConstructionWindow();

        const auto signalList = getAvailableCompatibleSignals(_cState->trackType);
        Common::copyToLegacyList(signalList, _cState->signalList);

        auto lastSignal = Scenario::getConstruction().signals[_cState->trackType];

        if (lastSignal == 0xFF)
        {
            lastSignal = _cState->signalList[0];
        }

        _cState->lastSelectedSignal = lastSignal;

        const auto stationList = getAvailableCompatibleStations(_cState->trackType, TransportMode::rail);
        Common::copyToLegacyList(stationList, _cState->stationList);

        auto lastStation = Scenario::getConstruction().trainStations[_cState->trackType];

        if (lastStation == 0xFF)
        {
            lastStation = _cState->stationList[0];
        }

        _cState->lastSelectedStationType = lastStation;

        const auto bridgeList = getAvailableCompatibleBridges(_cState->trackType, TransportMode::rail);
        Common::copyToLegacyList(bridgeList, _cState->bridgeList);

        auto lastBridge = Scenario::getConstruction().bridges[_cState->trackType];

        if (lastBridge == 0xFF)
        {
            lastBridge = _cState->bridgeList[0];
        }

        _cState->lastSelectedBridge = lastBridge;

        const auto modList = getAvailableCompatibleMods(_cState->trackType, TransportMode::rail, GameCommands::getUpdatingCompanyId());
        std::copy(modList.begin(), modList.end(), std::begin(_cState->modList));

        auto lastMod = Scenario::getConstruction().trackMods[_cState->trackType];

        if (lastMod == 0xFF)
        {
            lastMod = 0;
        }

        _cState->lastSelectedMods = lastMod;
        _cState->byte_113603A = 0;

        return trackWindow();
    }

    static Window* createRoadConstructionWindow()
    {
        Common::createConstructionWindow();

        _cState->lastSelectedSignal = 0xFF;

        const auto stationList = getAvailableCompatibleStations(_cState->trackType, TransportMode::road);
        Common::copyToLegacyList(stationList, _cState->stationList);

        auto lastStation = Scenario::getConstruction().roadStations[(_cState->trackType & ~(1ULL << 7))];

        if (lastStation == 0xFF)
        {
            lastStation = _cState->stationList[0];
        }

        _cState->lastSelectedStationType = lastStation;

        const auto bridgeList = getAvailableCompatibleBridges(_cState->trackType, TransportMode::road);
        Common::copyToLegacyList(bridgeList, _cState->bridgeList);

        auto lastBridge = Scenario::getConstruction().bridges[(_cState->trackType & ~(1ULL << 7))];

        if (lastBridge == 0xFF)
        {
            lastBridge = _cState->bridgeList[0];
        }

        _cState->lastSelectedBridge = lastBridge;

        const auto modList = getAvailableCompatibleMods(_cState->trackType, TransportMode::road, GameCommands::getUpdatingCompanyId());
        std::copy(modList.begin(), modList.end(), std::begin(_cState->modList));

        auto lastMod = Scenario::getConstruction().roadMods[(_cState->trackType & ~(1ULL << 7))];

        if (lastMod == 0xff)
        {
            lastMod = 0;
        }

        _cState->lastSelectedMods = lastMod;
        _cState->byte_113603A = 0;

        return trackWindow();
    }

    static Window* createDockConstructionWindow()
    {
        Common::createConstructionWindow();

        _cState->lastSelectedSignal = 0xFF;

        _cState->modList[0] = 0xFF;
        _cState->modList[1] = 0xFF;
        _cState->modList[2] = 0xFF;
        _cState->modList[3] = 0xFF;

        _cState->lastSelectedMods = 0;
        _cState->lastSelectedBridge = 0xFF;

        const auto stationList = getAvailableDocks();
        Common::copyToLegacyList(stationList, _cState->stationList);

        if (getGameState().lastShipPort == 0xFF)
        {
            _cState->lastSelectedStationType = _cState->stationList[0];
        }
        else
        {
            _cState->lastSelectedStationType = getGameState().lastShipPort;
        }

        return nonTrackWindow();
    }

    static Window* createAirportConstructionWindow()
    {
        Common::createConstructionWindow();

        _cState->lastSelectedSignal = 0xFF;
        _cState->modList[0] = 0xFF;
        _cState->modList[1] = 0xFF;
        _cState->modList[2] = 0xFF;
        _cState->modList[3] = 0xFF;
        _cState->lastSelectedMods = 0;
        _cState->lastSelectedBridge = 0xFF;

        const auto stationList = getAvailableAirports();
        Common::copyToLegacyList(stationList, _cState->stationList);

        if (getGameState().lastAirport == 0xFF)
        {
            _cState->lastSelectedStationType = _cState->stationList[0];
        }
        else
        {
            _cState->lastSelectedStationType = getGameState().lastAirport;
        }

        return nonTrackWindow();
    }

    // 0x004A0EAD
    Window* openAtTrack(const Window& main, TrackElement* track, const Pos2 pos)
    {
        auto* viewport = main.viewports[0];
        _cState->backupTileElement = *reinterpret_cast<TileElement*>(track);
        auto* copyElement = (_cState->backupTileElement).as<TrackElement>();
        if (copyElement == nullptr)
        {
            return nullptr;
        }

        if (copyElement->owner() != CompanyManager::getControllingId())
        {
            return nullptr;
        }

        removeConstructionGhosts();
        auto* wnd = WindowManager::find(WindowType::construction);
        if (wnd == nullptr)
        {
            WindowManager::closeConstructionWindows();
            Common::createConstructionWindow();
        }
        else
        {
            Common::resetWindow(*wnd, Common::widx::tab_construction);
        }

        _cState->trackType = copyElement->trackObjectId();
        _cState->byte_1136063 = 0;
        Common::setTrackOptions(_cState->trackType);

        _cState->constructionHover = 0;
        _cState->byte_113607E = 1;
        _cState->trackCost = 0x80000000;
        _cState->byte_1136076 = 0;
        _cState->lastSelectedTrackModSection = Track::ModSection::single;

        Common::setNextAndPreviousTrackTile(*copyElement, pos);

        const bool isCloserToNext = Common::isPointCloserToNextOrPreviousTile(Input::getDragLastLocation(), *viewport);

        const auto chosenLoc = isCloserToNext ? _cState->nextTile : _cState->previousTile;
        const auto chosenRotation = isCloserToNext ? _cState->nextTileRotation : _cState->previousTileRotation;
        _cState->x = chosenLoc.x;
        _cState->y = chosenLoc.y;
        _cState->constructionZ = chosenLoc.z;
        _cState->constructionRotation = chosenRotation;
        _cState->lastSelectedTrackPiece = 0;
        _cState->lastSelectedTrackGradient = 0;

        const auto signalList = getAvailableCompatibleSignals(_cState->trackType);
        Common::copyToLegacyList(signalList, _cState->signalList);
        auto lastSignal = Scenario::getConstruction().signals[_cState->trackType];

        if (lastSignal == 0xFF)
        {
            lastSignal = _cState->signalList[0];
        }

        _cState->lastSelectedSignal = lastSignal;

        const auto stationList = getAvailableCompatibleStations(_cState->trackType, TransportMode::rail);
        Common::copyToLegacyList(stationList, _cState->stationList);

        auto lastStation = Scenario::getConstruction().trainStations[_cState->trackType];

        if (lastStation == 0xFF)
        {
            lastStation = _cState->stationList[0];
        }

        _cState->lastSelectedStationType = lastStation;

        const auto bridgeList = getAvailableCompatibleBridges(_cState->trackType, TransportMode::rail);
        Common::copyToLegacyList(bridgeList, _cState->bridgeList);

        auto lastBridge = Scenario::getConstruction().bridges[_cState->trackType];

        if (lastBridge == 0xFF)
        {
            lastBridge = _cState->bridgeList[0];
        }

        _cState->lastSelectedBridge = lastBridge;

        if (copyElement->hasBridge())
        {
            _cState->lastSelectedBridge = copyElement->bridge();
        }
        const auto modList = getAvailableCompatibleMods(_cState->trackType, TransportMode::rail, GameCommands::getUpdatingCompanyId());
        std::copy(modList.begin(), modList.end(), std::begin(_cState->modList));

        _cState->lastSelectedMods = copyElement->mods();
        _cState->byte_113603A = 0;
        auto* window = WindowManager::find(WindowType::construction);

        if (window != nullptr)
        {
            Common::setDisabledWidgets(window);
        }

        return window;
    }

    // 0x004A147F
    Window* openAtRoad(const Window& main, RoadElement* road, const Pos2 pos)
    {
        auto* viewport = main.viewports[0];
        _cState->backupTileElement = *reinterpret_cast<TileElement*>(road);
        auto* copyElement = (_cState->backupTileElement).as<RoadElement>();
        if (copyElement == nullptr)
        {
            return nullptr;
        }

        removeConstructionGhosts();
        auto* wnd = WindowManager::find(WindowType::construction);
        if (wnd == nullptr)
        {
            WindowManager::closeConstructionWindows();
            Common::createConstructionWindow();
        }
        else
        {
            Common::resetWindow(*wnd, Common::widx::tab_construction);
        }

        _cState->trackType = copyElement->roadObjectId() | (1 << 7);
        _cState->byte_1136063 = 0;
        Common::setTrackOptions(_cState->trackType);

        _cState->constructionHover = 0;
        _cState->byte_113607E = 1;
        _cState->trackCost = 0x80000000;
        _cState->byte_1136076 = 0;
        _cState->lastSelectedTrackModSection = Track::ModSection::single;

        Common::setNextAndPreviousRoadTile(*copyElement, pos);

        const bool isCloserToNext = Common::isPointCloserToNextOrPreviousTile(Input::getDragLastLocation(), *viewport);

        const auto chosenLoc = isCloserToNext ? _cState->nextTile : _cState->previousTile;
        const auto chosenRotation = isCloserToNext ? _cState->nextTileRotation : _cState->previousTileRotation;
        _cState->x = chosenLoc.x;
        _cState->y = chosenLoc.y;
        _cState->constructionZ = chosenLoc.z;
        _cState->constructionRotation = chosenRotation;
        _cState->lastSelectedTrackPiece = 0;
        _cState->lastSelectedTrackGradient = 0;
        _cState->lastSelectedSignal = 0xFF;

        const auto stationList = getAvailableCompatibleStations(_cState->trackType, TransportMode::road);
        Common::copyToLegacyList(stationList, _cState->stationList);

        auto lastStation = Scenario::getConstruction().roadStations[(_cState->trackType & ~(1ULL << 7))];

        if (lastStation == 0xFF)
        {
            lastStation = _cState->stationList[0];
        }

        _cState->lastSelectedStationType = lastStation;

        const auto bridgeList = getAvailableCompatibleBridges(_cState->trackType, TransportMode::road);
        Common::copyToLegacyList(bridgeList, _cState->bridgeList);

        auto lastBridge = Scenario::getConstruction().bridges[(_cState->trackType & ~(1ULL << 7))];

        if (lastBridge == 0xFF)
        {
            lastBridge = _cState->bridgeList[0];
        }

        _cState->lastSelectedBridge = lastBridge;
        if (copyElement->hasBridge())
        {
            _cState->lastSelectedBridge = copyElement->bridge();
        }

        const auto modList = getAvailableCompatibleMods(_cState->trackType, TransportMode::road, GameCommands::getUpdatingCompanyId());
        std::copy(modList.begin(), modList.end(), std::begin(_cState->modList));

        _cState->lastSelectedMods = 0;
        auto* roadObj = ObjectManager::get<RoadObject>(_cState->trackType & ~(1ULL << 7));
        if (!roadObj->hasFlags(RoadObjectFlags::unk_03))
        {
            _cState->lastSelectedMods = copyElement->mods();
        }
        _cState->byte_113603A = 0;

        auto* window = WindowManager::find(WindowType::construction);

        if (window != nullptr)
        {
            Common::setDisabledWidgets(window);
        }

        return window;
    }

    // 0x004A3B0D
    Window* openWithFlags(const uint32_t flags)
    {
        auto mainWindow = WindowManager::getMainWindow();
        if (mainWindow)
        {
            auto viewport = mainWindow->viewports[0];
            _cState->viewportFlags = viewport->flags;
        }

        auto window = WindowManager::find(WindowType::construction);

        if (window != nullptr)
        {
            if (flags & (1 << 7))
            {
                auto trackType = flags & ~(1 << 7);
                auto roadObj = ObjectManager::get<RoadObject>(trackType);

                if (roadObj->hasFlags(RoadObjectFlags::unk_03))
                {
                    if (_cState->trackType & (1 << 7))
                    {
                        trackType = _cState->trackType & ~(1 << 7);
                        roadObj = ObjectManager::get<RoadObject>(trackType);

                        if (roadObj->hasFlags(RoadObjectFlags::unk_03))
                        {
                            _cState->trackType = static_cast<uint8_t>(flags);

                            Common::sub_4A3A50();

                            _cState->lastSelectedTrackPiece = 0;
                            _cState->lastSelectedTrackGradient = 0;

                            return window;
                        }
                    }
                }
            }
        }

        WindowManager::closeConstructionWindows();
        Common::sub_4CD454();

        mainWindow = WindowManager::getMainWindow();

        if (mainWindow)
        {
            auto viewport = mainWindow->viewports[0];
            viewport->flags = _cState->viewportFlags;
        }

        _cState->trackType = static_cast<uint8_t>(flags);
        _cState->byte_1136063 = flags >> 24;
        _cState->x = 0x1800;
        _cState->y = 0x1800;
        _cState->constructionZ = 0x100;
        _cState->constructionRotation = 0;
        _cState->constructionHover = 0;
        _cState->byte_113607E = 1;
        _cState->trackCost = 0x80000000;
        _cState->byte_1136076 = 0;
        _cState->lastSelectedTrackPiece = 0;
        _cState->lastSelectedTrackGradient = 0;
        _cState->lastSelectedTrackModSection = Track::ModSection::single;

        Common::setTrackOptions(flags);

        if (flags & (1 << 31))
        {
            return createAirportConstructionWindow();
        }
        else if (flags & (1 << 30))
        {
            return createDockConstructionWindow();
        }
        else if (flags & (1 << 7))
        {
            return createRoadConstructionWindow();
        }

        return createTrackConstructionWindow();
    }

    // 0x004A6E2B
    // Update available road and rail for player company
    void updateAvailableRoadAndRailOptions()
    {
        if (getGameState().lastRoadOption == 0xFF)
        {
            uint8_t lastRoadOption = getGameState().lastTrackTypeOption;
            if (lastRoadOption == 0xFF)
            {
                const auto availableObjects = companyGetAvailableRoads(CompanyManager::getControllingId());
                if (!availableObjects.empty())
                {
                    lastRoadOption = availableObjects[0];
                }
            }
            else
            {
                lastRoadOption |= 1 << 7;
            }
            getGameState().lastRoadOption = lastRoadOption;
            WindowManager::invalidate(Ui::WindowType::topToolbar, 0);
        }

        if (getGameState().lastRailroadOption == 0xFF)
        {
            const auto availableObjects = companyGetAvailableRailTracks(CompanyManager::getControllingId());
            if (!availableObjects.empty())
            {
                getGameState().lastRailroadOption = availableObjects[0];
            }
            WindowManager::invalidate(Ui::WindowType::topToolbar, 0);
        }
    }

    // 0x004A6E9B
    // Update available airports and docks for player company
    void updateAvailableAirportAndDockOptions()
    {
        if (getGameState().lastAirport != 0xFF)
        {
            const auto* airportObj = ObjectManager::get<AirportObject>(getGameState().lastAirport);
            if (getGameState().currentYear > airportObj->obsoleteYear)
            {
                getGameState().lastAirport = 0xFF;
            }
        }

        if (getGameState().lastAirport == 0xFF)
        {
            const auto availableObjects = getAvailableAirports();
            if (!availableObjects.empty())
            {
                getGameState().lastAirport = availableObjects[0];
                bool found = false;
                for (size_t vehicleObjectIndex = 0; vehicleObjectIndex < ObjectManager::getMaxObjects(ObjectType::vehicle); ++vehicleObjectIndex)
                {
                    const auto* vehicleObject = ObjectManager::get<VehicleObject>(vehicleObjectIndex);
                    if (vehicleObject == nullptr)
                    {
                        continue;
                    }
                    if (vehicleObject->mode == TransportMode::air)
                    {
                        const Company* company = CompanyManager::get(getGameState().playerCompanies[0]);
                        if (company->unlockedVehicles.get(vehicleObjectIndex))
                        {
                            found = true;
                            break;
                        }
                    }
                }
                if (!found)
                {
                    getGameState().lastAirport = 0xFF;
                }
            }
        }

        if (getGameState().lastShipPort == 0xFF)
        {
            const auto availableObjects = getAvailableDocks();
            if (!availableObjects.empty())
            {
                getGameState().lastShipPort = availableObjects[0];
                bool found = false;
                for (size_t vehicleObjectIndex = 0; vehicleObjectIndex < ObjectManager::getMaxObjects(ObjectType::vehicle); ++vehicleObjectIndex)
                {
                    const auto* vehicleObject = ObjectManager::get<VehicleObject>(vehicleObjectIndex);
                    if (vehicleObject == nullptr)
                    {
                        continue;
                    }
                    if (vehicleObject->mode == TransportMode::water)
                    {
                        const Company* company = CompanyManager::get(getGameState().playerCompanies[0]);
                        if (company->unlockedVehicles.get(vehicleObjectIndex))
                        {
                            found = true;
                            break;
                        }
                    }
                }
                if (!found)
                {
                    getGameState().lastShipPort = 0xFF;
                }
            }
        }
        WindowManager::invalidate(Ui::WindowType::topToolbar, 0);
    }

    // 0x004A6FAC
    void sub_4A6FAC()
    {
        auto window = WindowManager::find(WindowType::construction);
        if (window == nullptr)
        {
            return;
        }
        if (window->currentTab == Common::widx::tab_station - Common::widx::tab_construction)
        {
            if (_cState->byte_1136063 & ((1 << 7) | (1 << 6)))
            {
                WindowManager::close(window);
            }
            else
            {
                window->callOnMouseUp(Common::widx::tab_construction, window->widgets[Common::widx::tab_construction].id);
            }
        }
    }

    // 0x004A6A0C
    bool isStationTabOpen()
    {
        auto* window = WindowManager::find(WindowType::construction);
        if (window == nullptr)
        {
            return false;
        }

        return window->currentTab == Common::widx::tab_station - Common::widx::tab_construction;
    }

    // 0x004A69EE
    bool isOverheadTabOpen()
    {
        auto* window = WindowManager::find(WindowType::construction);
        if (window == nullptr)
        {
            return false;
        }

        return window->currentTab == Common::widx::tab_overhead - Common::widx::tab_construction;
    }

    // 0x004A6A2A
    bool isSignalTabOpen()
    {
        auto* window = WindowManager::find(WindowType::construction);
        if (window == nullptr)
        {
            return false;
        }

        return window->currentTab == Common::widx::tab_signal - Common::widx::tab_construction;
    }

    // 0x0049FEC7
    void removeConstructionGhosts()
    {
        if ((_ghostVisibilityFlags & GhostVisibilityFlags::constructArrow) != GhostVisibilityFlags::none)
        {
            World::TileManager::mapInvalidateTileFull(World::Pos2(_cState->x, _cState->y));
            World::resetMapSelectionFlag(World::MapSelectionFlags::enableConstructionArrow);
            _ghostVisibilityFlags = _ghostVisibilityFlags & ~GhostVisibilityFlags::constructArrow;
        }
        Construction::removeTrackGhosts();
        Signal::removeSignalGhost();
        Station::removeStationGhost();
        Overhead::removeTrackModsGhost();
    }

    uint16_t getLastSelectedMods()
    {
        return _cState->lastSelectedMods;
    }

    Track::ModSection getLastSelectedTrackModSection()
    {
        if (WindowManager::find(WindowType::construction) != nullptr)
        {
            return _cState->lastSelectedTrackModSection;
        }
        else
        {
            return Track::ModSection::single;
        }
    }

    namespace Common
    {
        struct TabInformation
        {
            std::span<const Widget> widgets;
            const widx widgetIndex;
            const WindowEventList& events;
            void (*tabReset)(Window&);
        };

        // clang-format off
        static TabInformation tabInformationByTabOffset[] = {
            { Construction::getWidgets(), widx::tab_construction, Construction::getEvents(), &Construction::tabReset },
            { Station::getWidgets(),      widx::tab_station,      Station::getEvents(),      &Station::tabReset },
            { Signal::getWidgets(),       widx::tab_signal,       Signal::getEvents(),       &Signal::tabReset },
            { Overhead::getWidgets(),     widx::tab_overhead,     Overhead::getEvents(),     &Overhead::tabReset },
        };
        // clang-format on

        void prepareDraw(Window* self)
        {
            // Activate the current tab
            self->activatedWidgets &= ~((1ULL << tab_construction) | (1ULL << tab_overhead) | (1ULL << tab_signal) | (1ULL << tab_station));
            self->activatedWidgets |= (1ULL << Common::tabInformationByTabOffset[self->currentTab].widgetIndex);
        }

        // 0x004A0EF4
        void resetWindow(Window& self, WidgetIndex_t tabWidgetIndex)
        {
            self.currentTab = tabWidgetIndex - widx::tab_construction;

            const auto& tabInfo = tabInformationByTabOffset[tabWidgetIndex - widx::tab_construction];

            self.eventHandlers = &tabInfo.events;
            self.activatedWidgets = 0;
            self.setWidgets(tabInfo.widgets);

            setDisabledWidgets(&self);

            self.width = self.widgets[widx::frame].right + 1;
            self.height = self.widgets[widx::frame].bottom + 1;
        }

        void setNextAndPreviousTrackTile(const TrackElement& elTrack, const World::Pos2& pos)
        {
            const auto& piece = TrackData::getTrackPiece(elTrack.trackId())[elTrack.sequenceIndex()];
            const auto firstTileOffset = Math::Vector::rotate(World::Pos2(piece.x, piece.y), elTrack.rotation());
            const auto firstTile = World::Pos3(pos.x, pos.y, elTrack.baseHeight()) - World::Pos3(firstTileOffset.x, firstTileOffset.y, piece.z);

            // Get coordinates of the next tile after the end of the track piece
            const auto trackAndDirection = (elTrack.trackId() << 3) | elTrack.rotation();
            const auto& trackSize = TrackData::getUnkTrack(trackAndDirection);
            const auto nextTile = firstTile + trackSize.pos;
            _cState->nextTile = nextTile;
            _cState->nextTileRotation = trackSize.rotationEnd;

            // Get coordinates of the previous tile before the start of the track piece
            const auto unk = World::kReverseRotation[trackSize.rotationBegin];
            auto previousTile = firstTile;
            _cState->previousTileRotation = unk;
            if (unk < 12)
            {
                previousTile += World::Pos3{ World::kRotationOffset[unk], 0 };
            }
            _cState->previousTile = previousTile;
        }

        void setNextAndPreviousRoadTile(const RoadElement& elRoad, const World::Pos2& pos)
        {
            const auto& piece = TrackData::getRoadPiece(elRoad.roadId())[elRoad.sequenceIndex()];
            const auto firstTileOffset = Math::Vector::rotate(World::Pos2(piece.x, piece.y), elRoad.rotation());
            const auto firstTile = World::Pos3(pos.x, pos.y, elRoad.baseHeight()) - World::Pos3(firstTileOffset.x, firstTileOffset.y, piece.z);

            // Get coordinates of the next tile after the end of the track piece
            const auto trackAndDirection = (elRoad.roadId() << 3) | elRoad.rotation();
            const auto& trackSize = TrackData::getUnkRoad(trackAndDirection);
            const auto nextTile = firstTile + trackSize.pos;
            _cState->nextTile = nextTile;
            _cState->nextTileRotation = trackSize.rotationEnd;

            // Get coordinates of the previous tile before the start of the track piece
            const auto unk = World::kReverseRotation[trackSize.rotationBegin];
            auto previousTile = firstTile;
            _cState->previousTileRotation = unk;
            if (unk < 12)
            {
                previousTile += World::Pos3{ World::kRotationOffset[unk], 0 };
            }
            _cState->previousTile = previousTile;
        }

        // True for next, false for previous
        bool isPointCloserToNextOrPreviousTile(const Point& point, const Viewport& viewport)
        {
            const auto vpPosNext = gameToScreen(_cState->nextTile + World::Pos3(16, 16, 0), viewport.getRotation());
            const auto uiPosNext = viewport.viewportToScreen(vpPosNext);
            const auto distanceToNext = Math::Vector::manhattanDistance2D(uiPosNext, point);

            const auto vpPosPrevious = gameToScreen(_cState->previousTile + World::Pos3(16, 16, 0), viewport.getRotation());
            const auto uiPosPrevious = viewport.viewportToScreen(vpPosPrevious);
            const auto distanceToPrevious = Math::Vector::manhattanDistance2D(uiPosPrevious, point);

            return distanceToNext < distanceToPrevious;
        }

        // 0x0049D93A
        void switchTab(Window& self, WidgetIndex_t widgetIndex)
        {
            if (self.currentTab == widgetIndex - widx::tab_construction)
            {
                return;
            }

            if (widgetIndex == widx::tab_station)
            {
                Ui::Windows::Station::showStationCatchment(StationId::null);
            }

            if (widgetIndex == widx::tab_construction)
            {
                Construction::activateSelectedConstructionWidgets();
            }

            removeConstructionGhosts();
            World::mapInvalidateMapSelectionTiles();
            World::resetMapSelectionFlag(World::MapSelectionFlags::enableConstruct);
            _cState->trackCost = 0x80000000;
            _cState->signalCost = 0x80000000;
            _cState->stationCost = 0x80000000;
            _cState->modCost = 0x80000000;
            _cState->byte_1136076 = 0;

            if (ToolManager::isToolActive(self.type, self.number))
            {
                ToolManager::toolCancel();
            }

            self.currentTab = widgetIndex - widx::tab_construction;
            self.frameNo = 0;
            self.flags &= ~(WindowFlags::flag_16);

            auto tabInfo = tabInformationByTabOffset[widgetIndex - widx::tab_construction];

            self.eventHandlers = &tabInfo.events;
            self.activatedWidgets = 0;
            self.setWidgets(tabInfo.widgets);
            self.holdableWidgets = 0;

            setDisabledWidgets(&self);

            self.invalidate();

            self.width = self.widgets[widx::frame].right + 1;
            self.height = self.widgets[widx::frame].bottom + 1;

            self.callOnResize();
            self.callPrepareDraw();
            self.initScrollWidgets();
            self.invalidate();

            tabInfo.tabReset(self);

            self.moveInsideScreenEdges();
        }

        // 0x0049EFEF
        static void drawRoadTabs(Window& self, Gfx::DrawingContext& drawingCtx)
        {
            auto company = CompanyManager::getPlayerCompany();
            auto companyColour = company->mainColours.primary;
            auto roadObj = ObjectManager::get<RoadObject>(_cState->trackType & ~(1 << 7));
            // Construction Tab
            {
                auto imageId = roadObj->image;
                if (self.currentTab == widx::tab_construction - widx::tab_construction)
                {
                    imageId += (self.frameNo / 4) % 32;
                }

                Widget::drawTab(self, drawingCtx, Gfx::recolour(imageId, companyColour), widx::tab_construction);
            }
            // Station Tab
            {
                Widget::drawTab(self, drawingCtx, ImageIds::null, widx::tab_station);
                if (!self.isDisabled(widx::tab_station))
                {
                    auto x = self.widgets[widx::tab_station].left + 1;
                    auto y = self.widgets[widx::tab_station].top + 1;
                    auto width = 29;
                    auto height = 25;
                    if (self.currentTab == widx::tab_station - widx::tab_construction)
                    {
                        height++;
                    }

                    const auto& rt = drawingCtx.currentRenderTarget();
                    auto clipped = Gfx::clipRenderTarget(rt, Ui::Rect(x, y, width, height));
                    if (clipped)
                    {
                        clipped->zoomLevel = 1;
                        clipped->width <<= 1;
                        clipped->height <<= 1;
                        clipped->x <<= 1;
                        clipped->y <<= 1;

                        drawingCtx.pushRenderTarget(*clipped);

                        auto roadStationObj = ObjectManager::get<RoadStationObject>(_cState->lastSelectedStationType);
                        auto imageId = Gfx::recolour(roadStationObj->image, companyColour);
                        drawingCtx.drawImage(-4, -10, imageId);
                        auto colour = Colours::getTranslucent(companyColour);
                        if (!roadStationObj->hasFlags(RoadStationFlags::recolourable))
                        {
                            colour = ExtColour::unk2E;
                        }
                        imageId = Gfx::recolourTranslucent(roadStationObj->image, colour) + 1;
                        drawingCtx.drawImage(-4, -10, imageId);

                        drawingCtx.popRenderTarget();
                    }

                    Widget::drawTab(self, drawingCtx, Widget::kContentUnk, widx::tab_station);
                }
            }
            // Overhead tab
            {
                Widget::drawTab(self, drawingCtx, ImageIds::null, widx::tab_overhead);
                if (!self.isDisabled(widx::tab_overhead))
                {
                    auto x = self.widgets[widx::tab_overhead].left + 2;
                    auto y = self.widgets[widx::tab_overhead].top + 2;

                    for (auto i = 0; i < 2; i++)
                    {
                        if (_cState->modList[i] != 0xFF)
                        {
                            auto roadExtraObj = ObjectManager::get<RoadExtraObject>(_cState->modList[i]);
                            auto imageId = roadExtraObj->var_0E;
                            if (self.currentTab == widx::tab_overhead - widx::tab_construction)
                            {
                                imageId += (self.frameNo / 2) % 8;
                            }
                            drawingCtx.drawImage(x, y, imageId);
                        }
                    }

                    Widget::drawTab(self, drawingCtx, Widget::kContentUnk, widx::tab_overhead);
                }
            }
        }

        std::array<uint32_t, 16> kTrackPreviewImages = {
            TrackObj::ImageIds::kUiPreviewImage0,
            TrackObj::ImageIds::kUiPreviewImage1,
            TrackObj::ImageIds::kUiPreviewImage2,
            TrackObj::ImageIds::kUiPreviewImage3,
            TrackObj::ImageIds::kUiPreviewImage4,
            TrackObj::ImageIds::kUiPreviewImage5,
            TrackObj::ImageIds::kUiPreviewImage6,
            TrackObj::ImageIds::kUiPreviewImage7,
            TrackObj::ImageIds::kUiPreviewImage8,
            TrackObj::ImageIds::kUiPreviewImage9,
            TrackObj::ImageIds::kUiPreviewImage10,
            TrackObj::ImageIds::kUiPreviewImage11,
            TrackObj::ImageIds::kUiPreviewImage12,
            TrackObj::ImageIds::kUiPreviewImage13,
            TrackObj::ImageIds::kUiPreviewImage14,
            TrackObj::ImageIds::kUiPreviewImage15,
        };

        // 0x0049ED40
        static void drawTrackTabs(Window& self, Gfx::DrawingContext& drawingCtx)
        {
            auto company = CompanyManager::getPlayerCompany();
            auto companyColour = company->mainColours.primary;
            auto trackObj = ObjectManager::get<TrackObject>(_cState->trackType);
            // Construction Tab
            {
                auto imageId = trackObj->image;
                if (self.currentTab == widx::tab_construction - widx::tab_construction)
                {
                    imageId += kTrackPreviewImages[(self.frameNo / 4) % kTrackPreviewImages.size()];
                }

                Widget::drawTab(self, drawingCtx, Gfx::recolour(imageId, companyColour), widx::tab_construction);
            }
            // Station Tab
            {
                if (_cState->byte_1136063 & (1 << 7))
                {
                    auto imageId = ObjectManager::get<InterfaceSkinObject>()->img + InterfaceSkin::ImageIds::toolbar_menu_airport;

                    Widget::drawTab(self, drawingCtx, imageId, widx::tab_station);
                }
                else
                {
                    if (_cState->byte_1136063 & (1 << 6))
                    {
                        auto imageId = ObjectManager::get<InterfaceSkinObject>()->img + InterfaceSkin::ImageIds::toolbar_menu_ship_port;

                        Widget::drawTab(self, drawingCtx, imageId, widx::tab_station);
                    }
                    else
                    {
                        Widget::drawTab(self, drawingCtx, ImageIds::null, widx::tab_station);
                        if (!self.isDisabled(widx::tab_station))
                        {
                            auto x = self.widgets[widx::tab_station].left + 1;
                            auto y = self.widgets[widx::tab_station].top + 1;
                            auto width = 29;
                            auto height = 25;
                            if (self.currentTab == widx::tab_station - widx::tab_construction)
                            {
                                height++;
                            }

                            const auto& rt = drawingCtx.currentRenderTarget();
                            auto clipped = Gfx::clipRenderTarget(rt, Ui::Rect(x, y, width, height));
                            if (clipped)
                            {
                                clipped->zoomLevel = 1;
                                clipped->width *= 2;
                                clipped->height *= 2;
                                clipped->x *= 2;
                                clipped->y *= 2;

                                drawingCtx.pushRenderTarget(*clipped);

                                auto trainStationObj = ObjectManager::get<TrainStationObject>(_cState->lastSelectedStationType);
                                auto imageId = Gfx::recolour(trainStationObj->image + TrainStation::ImageIds::preview_image, companyColour);
                                drawingCtx.drawImage(-4, -9, imageId);

                                auto colour = Colours::getTranslucent(companyColour);
                                if (!trainStationObj->hasFlags(TrainStationFlags::recolourable))
                                {
                                    colour = ExtColour::unk2E;
                                }
                                imageId = Gfx::recolourTranslucent(trainStationObj->image + TrainStation::ImageIds::preview_image_windows, colour);
                                drawingCtx.drawImage(-4, -9, imageId);

                                drawingCtx.popRenderTarget();
                            }

                            Widget::drawTab(self, drawingCtx, Widget::kContentUnk, widx::tab_station);
                        }
                    }
                }
            }
            // Signal Tab
            {
                Widget::drawTab(self, drawingCtx, ImageIds::null, widx::tab_signal);
                if (!self.isDisabled(widx::tab_signal))
                {
                    auto x = self.widgets[widx::tab_signal].left + 1;
                    auto y = self.widgets[widx::tab_signal].top + 1;
                    auto width = 29;
                    auto height = 25;
                    if (self.currentTab == widx::tab_station - widx::tab_construction)
                    {
                        height++;
                    }

                    const auto& rt = drawingCtx.currentRenderTarget();
                    auto clipped = Gfx::clipRenderTarget(rt, Ui::Rect(x, y, width, height));
                    if (clipped)
                    {
                        drawingCtx.pushRenderTarget(*clipped);

                        auto trainSignalObject = ObjectManager::get<TrainSignalObject>(_cState->lastSelectedSignal);
                        auto imageId = trainSignalObject->image;
                        if (self.currentTab == widx::tab_signal - widx::tab_construction)
                        {
                            auto frames = signalFrames[(((trainSignalObject->numFrames + 2) / 3) - 2)];
                            auto frameCount = std::size(frames) - 1;
                            frameCount &= (self.frameNo >> trainSignalObject->animationSpeed);
                            auto frameIndex = frames[frameCount];
                            frameIndex <<= 3;
                            imageId += frameIndex;
                        }
                        drawingCtx.drawImage(15, 31, imageId);

                        drawingCtx.popRenderTarget();
                    }

                    Widget::drawTab(self, drawingCtx, Widget::kContentUnk, widx::tab_signal);
                }
            }
            // Overhead Tab
            {
                Widget::drawTab(self, drawingCtx, ImageIds::null, widx::tab_overhead);
                if (!self.isDisabled(widx::tab_overhead))
                {
                    auto x = self.widgets[widx::tab_overhead].left + 2;
                    auto y = self.widgets[widx::tab_overhead].top + 2;

                    for (auto i = 0; i < 4; i++)
                    {
                        if (_cState->modList[i] != 0xFF)
                        {
                            auto trackExtraObj = ObjectManager::get<TrackExtraObject>(_cState->modList[i]);
                            auto imageId = trackExtraObj->var_0E;
                            if (self.currentTab == widx::tab_overhead - widx::tab_construction)
                            {
                                imageId += (self.frameNo / 2) % 8;
                            }
                            drawingCtx.drawImage(x, y, imageId);
                        }
                    }

                    Widget::drawTab(self, drawingCtx, Widget::kContentUnk, widx::tab_overhead);
                }
            }
        }

        // 0x0049ED33
        void drawTabs(Window& self, Gfx::DrawingContext& drawingCtx)
        {
            if (_cState->trackType & (1 << 7))
            {
                drawRoadTabs(self, drawingCtx);
            }
            else
            {
                drawTrackTabs(self, drawingCtx);
            }
        }

        // 0x004A09DE
        void repositionTabs(Window* self)
        {
            int16_t xPos = self->widgets[widx::tab_construction].left;
            const int16_t tabWidth = self->widgets[widx::tab_construction].right - xPos;

            for (uint8_t i = widx::tab_construction; i <= widx::tab_overhead; i++)
            {
                if (self->isDisabled(i))
                {
                    self->widgets[i].hidden = true;
                    continue;
                }

                self->widgets[i].hidden = false;
                self->widgets[i].left = xPos;
                self->widgets[i].right = xPos + tabWidth;
                xPos = self->widgets[i].right + 1;
            }
        }

        // 0x0049DD14
        void onClose([[maybe_unused]] Window& self)
        {
            removeConstructionGhosts();
            WindowManager::viewportSetVisibility(WindowManager::ViewportVisibility::reset);
            World::mapInvalidateMapSelectionTiles();
            World::resetMapSelectionFlag(World::MapSelectionFlags::enableConstruct);
            World::resetMapSelectionFlag(World::MapSelectionFlags::enableConstructionArrow);
            Windows::Main::hideDirectionArrows();
            Windows::Main::hideGridlines();
        }

        // 0x0049E437, 0x0049E76F, 0x0049ECD1
        void onUpdate(Window* self, GhostVisibilityFlags flag)
        {
            self->frameNo++;
            self->callPrepareDraw();
            WindowManager::invalidateWidget(WindowType::construction, self->number, self->currentTab + Common::widx::tab_construction);

            if (ToolManager::isToolActive(WindowType::construction, self->number))
            {
                return;
            }

            if ((_ghostVisibilityFlags & flag) == GhostVisibilityFlags::none)
            {
                return;
            }

            removeConstructionGhosts();
        }

        // 0x004CD454
        void sub_4CD454()
        {
            if (SceneManager::isNetworkHost())
            {
                auto window = WindowManager::find(ToolManager::getToolWindowType(), ToolManager::getToolWindowNumber());
                if (window != nullptr)
                {
                    ToolManager::toolCancel();
                }
            }
        }

        // 0x004A3A06
        void setTrackOptions(const uint8_t trackType)
        {
            auto newTrackType = trackType;
            if (trackType & (1 << 7))
            {
                newTrackType &= ~(1 << 7);
                auto roadObj = ObjectManager::get<RoadObject>(newTrackType);
                if (!roadObj->hasFlags(RoadObjectFlags::unk_01))
                {
                    getGameState().lastRoadOption = trackType;
                }
                else
                {
                    getGameState().lastRailroadOption = trackType;
                }
            }
            else
            {
                auto trackObj = ObjectManager::get<TrackObject>(newTrackType);
                if (!trackObj->hasFlags(TrackObjectFlags::unk_02))
                {
                    getGameState().lastRailroadOption = trackType;
                }
                else
                {
                    getGameState().lastRoadOption = trackType;
                }
            }
            WindowManager::invalidate(WindowType::topToolbar, 0);
        }

        // 0x0049CE33
        void setDisabledWidgets(Window* self)
        {
            auto disabledWidgets = 0;
            if (SceneManager::isEditorMode())
            {
                disabledWidgets |= (1ULL << Common::widx::tab_station);
            }

            if (_cState->byte_1136063 & (1 << 7 | 1 << 6))
            {
                disabledWidgets |= (1ULL << Common::widx::tab_construction);
            }

            if (_cState->lastSelectedSignal == 0xFF)
            {
                disabledWidgets |= (1ULL << Common::widx::tab_signal);
            }

            if (_cState->modList[0] == 0xFF && _cState->modList[1] == 0xFF && _cState->modList[2] == 0xFF && _cState->modList[3] == 0xFF)
            {
                disabledWidgets |= (1ULL << Common::widx::tab_overhead);
            }

            if (_cState->lastSelectedStationType == 0xFF)
            {
                disabledWidgets |= (1ULL << Common::widx::tab_station);
            }

            self->disabledWidgets = disabledWidgets;
        }

        // 0x004A0963
        void createConstructionWindow()
        {
            auto window = WindowManager::createWindow(
                WindowType::construction,
                Construction::kWindowSize,
                WindowFlags::flag_11 | WindowFlags::noAutoClose,
                Construction::getEvents());

            window->setWidgets(Construction::getWidgets());
            window->currentTab = 0;
            window->activatedWidgets = 0;

            setDisabledWidgets(window);

            window->initScrollWidgets();
            window->owner = CompanyManager::getControllingId();

            auto skin = ObjectManager::get<InterfaceSkinObject>();
            window->setColour(WindowColour::secondary, skin->windowConstructionColour);

            WindowManager::sub_4CEE0B(*window);
            Windows::Main::showDirectionArrows();
            Windows::Main::showGridlines();
        }

        // 0x004A3A50
        void sub_4A3A50()
        {
            removeConstructionGhosts();
            setTrackOptions(_cState->trackType);
            const auto stationList = getAvailableCompatibleStations(_cState->trackType, TransportMode::road);
            copyToLegacyList(stationList, _cState->stationList);

            auto lastStation = Scenario::getConstruction().roadStations[(_cState->trackType & ~(1ULL << 7))];
            if (lastStation == 0xFF)
            {
                lastStation = _cState->stationList[0];
            }
            _cState->lastSelectedStationType = lastStation;

            const auto bridgeList = getAvailableCompatibleBridges(_cState->trackType, TransportMode::road);
            copyToLegacyList(bridgeList, _cState->bridgeList);

            auto lastBridge = Scenario::getConstruction().bridges[(_cState->trackType & ~(1ULL << 7))];
            if (lastBridge == 0xFF)
            {
                lastBridge = _cState->bridgeList[0];
            }
            _cState->lastSelectedBridge = lastBridge;

            const auto modList = getAvailableCompatibleMods(_cState->trackType, TransportMode::road, GameCommands::getUpdatingCompanyId());
            std::copy(modList.begin(), modList.end(), std::begin(_cState->modList));

            auto lastMod = Scenario::getConstruction().roadMods[(_cState->trackType & ~(1ULL << 7))];
            if (lastMod == 0xFF)
            {
                lastMod = 0;
            }
            _cState->lastSelectedMods = lastMod;

            auto window = WindowManager::find(WindowType::construction);

            if (window != nullptr)
            {
                setDisabledWidgets(window);
            }
            Construction::activateSelectedConstructionWidgets();
        }

        void previousTab(Window* self)
        {
            WidgetIndex_t prev = self->prevAvailableWidgetInRange(widx::tab_construction, widx::tab_overhead);
            if (prev != -1)
            {
                self->callOnMouseUp(prev, self->widgets[prev].id);
            }
        }

        void nextTab(Window* self)
        {
            WidgetIndex_t next = self->nextAvailableWidgetInRange(widx::tab_construction, widx::tab_overhead);
            if (next != -1)
            {
                self->callOnMouseUp(next, self->widgets[next].id);
            }
        }
    }

    bool rotate(Window& self)
    {
        switch (self.currentTab)
        {
            case Common::widx::tab_construction - Common::widx::tab_construction:
                if (_cState->constructionHover == 1)
                {
                    self.callOnMouseUp(Construction::widx::rotate_90, self.widgets[Construction::widx::rotate_90].id);
                    removeConstructionGhosts();
                    return true;
                }
                break;

            case Common::widx::tab_station - Common::widx::tab_construction:
                if (!self.widgets[Station::widx::rotate].hidden)
                {
                    self.callOnMouseUp(Station::widx::rotate, self.widgets[Station::widx::rotate].id);
                    return true;
                }
                break;
        }
        return false;
    }

    void registerHooks()
    {
        registerHook(
            0x0049F1B5,
            [](registers& regs) FORCE_ALIGN_ARG_POINTER -> uint8_t {
                registers backup = regs;
                Construction::activateSelectedConstructionWidgets();
                regs = backup;
                return 0;
            });

        // registerHook(
        //     0x0049DC97,
        //     [](registers& regs) FORCE_ALIGN_ARG_POINTER -> uint8_t {
        //         registers backup = regs;
        //         construction::on_tool_down(*((Ui::window*)regs.esi), regs.dx, regs.ax, regs.cx);
        //         regs = backup;
        //         return 0;
        //     });
    }
}
