#include "Audio/Audio.h"
#include "Construction.h"
#include "GameCommands/GameCommands.h"
#include "GameCommands/Road/CreateRoadMod.h"
#include "GameCommands/Road/RemoveRoadMod.h"
#include "GameCommands/Track/CreateTrackMod.h"
#include "GameCommands/Track/RemoveTrackMod.h"
#include "Graphics/ImageIds.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/SoftwareDrawingEngine.h"
#include "Graphics/TextRenderer.h"
#include "Input.h"
#include "Localisation/FormatArguments.hpp"
#include "Localisation/StringIds.h"
#include "Map/RoadElement.h"
#include "Map/Track/TrackModSection.h"
#include "Map/TrackElement.h"
#include "Objects/ObjectManager.h"
#include "Objects/RoadExtraObject.h"
#include "Objects/RoadObject.h"
#include "Objects/TrackExtraObject.h"
#include "Objects/TrackObject.h"
#include "SceneManager.h"
#include "Ui/Dropdown.h"
#include "Ui/ToolManager.h"
#include "Ui/ViewportInteraction.h"
#include "Ui/Widget.h"
#include "Ui/Widgets/CheckboxWidget.h"
#include "Ui/Widgets/DropdownWidget.h"
#include "Ui/Widgets/Wt3Widget.h"
#include "World/CompanyManager.h"

using namespace OpenLoco::Interop;
using namespace OpenLoco::World;
using namespace OpenLoco::World::TileManager;

namespace OpenLoco::Ui::Windows::Construction::Overhead
{
    static constexpr auto widgets = makeWidgets(
        Common::makeCommonWidgets(138, 192, StringIds::stringid_2),
        Widgets::Checkbox({ 3, 45 }, { 132, 12 }, WindowColour::secondary, StringIds::empty, StringIds::tooltip_select_track_mod),
        Widgets::Checkbox({ 3, 57 }, { 132, 12 }, WindowColour::secondary, StringIds::empty, StringIds::tooltip_select_track_mod),
        Widgets::Checkbox({ 3, 69 }, { 132, 12 }, WindowColour::secondary, StringIds::empty, StringIds::tooltip_select_track_mod),
        Widgets::Checkbox({ 3, 81 }, { 132, 12 }, WindowColour::secondary, StringIds::empty, StringIds::tooltip_select_track_mod),
        Widgets::Wt3Widget({ 35, 110 }, { 66, 66 }, WindowColour::secondary),
        Widgets::dropdownWidgets({ 3, 95 }, { 132, 12 }, WindowColour::secondary, Widget::kContentNull, StringIds::tooltip_select_track_to_upgrade));

    std::span<const Widget> getWidgets()
    {
        return widgets;
    }

    WindowEventList events;

    // 0x0049EBD1
    static void onMouseUp(Window& self, WidgetIndex_t widgetIndex, [[maybe_unused]] const WidgetId id)
    {
        switch (widgetIndex)
        {
            case Common::widx::close_button:
                WindowManager::close(&self);
                break;

            case Common::widx::tab_construction:
            case Common::widx::tab_overhead:
            case Common::widx::tab_signal:
            case Common::widx::tab_station:
                Common::switchTab(&self, widgetIndex);
                break;

            case widx::checkbox_1:
            case widx::checkbox_2:
            case widx::checkbox_3:
            case widx::checkbox_4:
            {
                auto checkboxIndex = widgetIndex - widx::checkbox_1;

                _cState->lastSelectedMods = _cState->lastSelectedMods ^ (1 << checkboxIndex);

                // TODO: & ~(1 << 7) added to prevent crashing when selecting/deselecting overhead wires for trams
                Scenario::getConstruction().trackMods[_cState->trackType & ~(1 << 7)] = _cState->lastSelectedMods;

                self.invalidate();
                break;
            }
        }
    }

    // 0x0049EBFC
    static void onMouseDown(Window& self, WidgetIndex_t widgetIndex, [[maybe_unused]] const WidgetId id)
    {
        switch (widgetIndex)
        {
            case widx::track_dropdown:
            {
                uint8_t modCount = 3;

                auto widget = self.widgets[widx::track];
                auto xPos = widget.left + self.x;
                auto yPos = widget.top + self.y;
                auto width = widget.width() + 2;
                auto height = widget.height();

                Dropdown::show(xPos, yPos, width, height, self.getColour(WindowColour::secondary), modCount, (1 << 7));

                Dropdown::add(0, StringIds::single_section);
                Dropdown::add(1, StringIds::block_section);
                Dropdown::add(2, StringIds::all_connected_track);

                Dropdown::setHighlightedItem(enumValue(_cState->lastSelectedTrackModSection));
                break;
            }

            case widx::image:
            {
                ToolManager::toolCancel();
                ToolManager::toolSet(&self, widgetIndex, CursorId::crosshair);
                break;
            }
        }
    }

    // 0x0049EC09
    static void onDropdown(Window& self, WidgetIndex_t widgetIndex, [[maybe_unused]] const WidgetId id, int16_t itemIndex)
    {
        if (widgetIndex != widx::track_dropdown)
        {
            return;
        }

        if (itemIndex != -1)
        {
            _cState->lastSelectedTrackModSection = static_cast<Track::ModSection>(itemIndex);
            self.invalidate();
        }
    }

    // 0x0049ECD1
    static void onUpdate(Window& self)
    {
        Common::onUpdate(&self, GhostVisibilityFlags::overhead);
    }

    static std::optional<GameCommands::RoadModsPlacementArgs> getRoadModsPlacementArgsFromCursor(const int16_t x, const int16_t y)
    {
        static loco_global<Viewport*, 0x01135F52> _1135F52;

        auto [interaction, viewport] = ViewportInteraction::getMapCoordinatesFromPos(x, y, ~(ViewportInteraction::InteractionItemFlags::roadAndTram));
        _1135F52 = viewport;

        if (interaction.type != ViewportInteraction::InteractionItem::road)
        {
            return std::nullopt;
        }

        auto* elRoad = reinterpret_cast<World::TileElement*>(interaction.object)->as<RoadElement>();
        if (elRoad == nullptr)
        {
            return std::nullopt;
        }

        GameCommands::RoadModsPlacementArgs args;
        args.type = _cState->lastSelectedMods;
        args.pos = World::Pos3(interaction.pos.x, interaction.pos.y, elRoad->baseHeight());
        args.rotation = elRoad->rotation();
        args.roadId = elRoad->roadId();
        args.index = elRoad->sequenceIndex();
        args.roadObjType = elRoad->roadObjectId();
        args.modSection = _cState->lastSelectedTrackModSection;
        return { args };
    }

    static std::optional<GameCommands::TrackModsPlacementArgs> getTrackModsPlacementArgsFromCursor(const int16_t x, const int16_t y)
    {
        static loco_global<Viewport*, 0x01135F52> _1135F52;

        auto [interaction, viewport] = ViewportInteraction::getMapCoordinatesFromPos(x, y, ~(ViewportInteraction::InteractionItemFlags::track));
        _1135F52 = viewport;

        if (interaction.type != ViewportInteraction::InteractionItem::track)
        {
            return std::nullopt;
        }

        auto* elTrack = reinterpret_cast<World::TileElement*>(interaction.object)->as<TrackElement>();
        if (elTrack == nullptr)
        {
            return std::nullopt;
        }

        GameCommands::TrackModsPlacementArgs args;
        args.type = _cState->lastSelectedMods;
        args.pos = World::Pos3(interaction.pos.x, interaction.pos.y, elTrack->baseHeight());
        args.rotation = elTrack->rotation();
        args.trackId = elTrack->trackId();
        args.index = elTrack->sequenceIndex();
        args.trackObjType = elTrack->trackObjectId();
        args.modSection = _cState->lastSelectedTrackModSection;
        return { args };
    }

    // 0x0049FF0
    void removeTrackModsGhost()
    {
        if ((_ghostVisibilityFlags & GhostVisibilityFlags::overhead) != GhostVisibilityFlags::none)
        {
            if (_cState->modGhostTrackObjId & (1 << 7))
            {
                GameCommands::RoadModsRemovalArgs args;
                args.pos = _cState->modGhostPos;
                args.rotation = _cState->modGhostRotation;
                args.roadId = _cState->modGhostTrackId;
                args.index = _cState->modGhostTileIndex;
                args.roadObjType = _cState->modGhostTrackObjId & ~(1 << 7);
                args.type = _cState->lastSelectedMods;
                args.modSection = _cState->lastSelectedTrackModSection;
                GameCommands::doCommand(args, GameCommands::Flags::apply | GameCommands::Flags::noErrorWindow | GameCommands::Flags::noPayment | GameCommands::Flags::ghost);
            }
            else
            {
                GameCommands::TrackModsRemovalArgs args;
                args.pos = _cState->modGhostPos;
                args.rotation = _cState->modGhostRotation;
                args.trackId = _cState->modGhostTrackId;
                args.index = _cState->modGhostTileIndex;
                args.trackObjType = _cState->modGhostTrackObjId & ~(1 << 7);
                args.type = _cState->lastSelectedMods;
                args.modSection = _cState->lastSelectedTrackModSection;
                GameCommands::doCommand(args, GameCommands::Flags::apply | GameCommands::Flags::noErrorWindow | GameCommands::Flags::noPayment | GameCommands::Flags::ghost);
            }
            _ghostVisibilityFlags = _ghostVisibilityFlags & ~GhostVisibilityFlags::overhead;
        }
    }

    static uint32_t placeRoadModGhost(const GameCommands::RoadModsPlacementArgs& args)
    {
        auto res = GameCommands::doCommand(args, GameCommands::Flags::apply | GameCommands::Flags::flag_1 | GameCommands::Flags::noErrorWindow | GameCommands::Flags::noPayment | GameCommands::Flags::ghost);
        if (res != GameCommands::FAILURE)
        {
            _ghostVisibilityFlags = _ghostVisibilityFlags | GhostVisibilityFlags::overhead;
            _cState->modGhostPos = args.pos;
            _cState->modGhostRotation = args.rotation;
            _cState->modGhostTrackId = args.roadId;
            _cState->modGhostTileIndex = args.index;
            _cState->modGhostTrackObjId = args.roadObjType | (1 << 7); // This looks wrong!
        }
        return res;
    }

    static uint32_t placeTrackModGhost(const GameCommands::TrackModsPlacementArgs& args)
    {
        auto res = GameCommands::doCommand(args, GameCommands::Flags::apply | GameCommands::Flags::flag_1 | GameCommands::Flags::noErrorWindow | GameCommands::Flags::noPayment | GameCommands::Flags::ghost);
        if (res != GameCommands::FAILURE)
        {
            _ghostVisibilityFlags = _ghostVisibilityFlags | GhostVisibilityFlags::overhead;
            _cState->modGhostPos = args.pos;
            _cState->modGhostRotation = args.rotation;
            _cState->modGhostTrackId = args.trackId;
            _cState->modGhostTileIndex = args.index;
            _cState->modGhostTrackObjId = args.trackObjType;
        }
        return res;
    }

    // 0x0049EC15
    static void onToolUpdate(Window& self, const WidgetIndex_t widgetIndex, [[maybe_unused]] const WidgetId id, const int16_t x, const int16_t y)
    {
        if (widgetIndex != widx::image)
        {
            return;
        }

        if (_cState->trackType & (1 << 7))
        {
            auto placementArgs = getRoadModsPlacementArgsFromCursor(x, y);
            if (!placementArgs || ((placementArgs->roadObjType | (1 << 7)) != _cState->trackType))
            {
                removeConstructionGhosts();
                if (_cState->modCost != 0x80000000)
                {
                    _cState->modCost = 0x80000000;
                    self.invalidate();
                }
                return;
            }

            if ((_ghostVisibilityFlags & GhostVisibilityFlags::overhead) != GhostVisibilityFlags::none)
            {
                if (_cState->modGhostPos == placementArgs->pos
                    && _cState->modGhostRotation == placementArgs->rotation
                    && _cState->modGhostTrackId == placementArgs->roadId
                    && _cState->modGhostTileIndex == placementArgs->index
                    && _cState->modGhostTrackObjId == placementArgs->roadObjType)
                {
                    return;
                }
            }

            removeConstructionGhosts();

            auto cost = placeRoadModGhost(*placementArgs);
            if (cost != _cState->modCost)
            {
                _cState->modCost = cost;
                self.invalidate();
            }
        }
        else
        {
            auto placementArgs = getTrackModsPlacementArgsFromCursor(x, y);
            if (!placementArgs || (placementArgs->trackObjType != _cState->trackType))
            {
                removeConstructionGhosts();
                if (_cState->modCost != 0x80000000)
                {
                    _cState->modCost = 0x80000000;
                    self.invalidate();
                }
                return;
            }

            if ((_ghostVisibilityFlags & GhostVisibilityFlags::overhead) != GhostVisibilityFlags::none)
            {
                if (_cState->modGhostPos == placementArgs->pos
                    && _cState->modGhostRotation == placementArgs->rotation
                    && _cState->modGhostTrackId == placementArgs->trackId
                    && _cState->modGhostTileIndex == placementArgs->index
                    && _cState->modGhostTrackObjId == placementArgs->trackObjType)
                {
                    return;
                }
            }

            removeConstructionGhosts();

            auto cost = placeTrackModGhost(*placementArgs);
            if (cost != _cState->modCost)
            {
                _cState->modCost = cost;
                self.invalidate();
            }
        }
    }

    // 0x0049EC20
    static void onToolUp([[maybe_unused]] Window& self, const WidgetIndex_t widgetIndex, [[maybe_unused]] const WidgetId id, const int16_t x, const int16_t y)
    {
        if (widgetIndex != widx::image)
        {
            return;
        }

        removeConstructionGhosts();

        if (_cState->trackType & (1 << 7))
        {
            auto args = getRoadModsPlacementArgsFromCursor(x, y);
            if (!args)
            {
                return;
            }

            if ((args->roadObjType | (1 << 7)) != _cState->trackType)
            {
                Error::open(StringIds::error_cant_build_this_here, StringIds::wrong_type_of_track_road);
                return;
            }
            GameCommands::setErrorTitle(StringIds::error_cant_build_this_here);
            auto res = GameCommands::doCommand(*args, GameCommands::Flags::apply);
            if (res == GameCommands::FAILURE || res == 0)
            {
                return;
            }
            Audio::playSound(Audio::SoundId::construct, GameCommands::getPosition());
        }
        else
        {
            auto args = getTrackModsPlacementArgsFromCursor(x, y);
            if (!args)
            {
                return;
            }

            if (args->trackObjType != _cState->trackType)
            {
                Error::open(StringIds::error_cant_build_this_here, StringIds::wrong_type_of_track_road);
                return;
            }
            GameCommands::setErrorTitle(StringIds::error_cant_build_this_here);
            auto res = GameCommands::doCommand(*args, GameCommands::Flags::apply);
            if (res == GameCommands::FAILURE || res == 0)
            {
                return;
            }
            Audio::playSound(Audio::SoundId::construct, GameCommands::getPosition());
        }
    }

    static void setCheckbox(Window* self, WidgetIndex_t checkboxIndex, StringId name)
    {
        auto widgetIndex = checkboxIndex + widx::checkbox_1;
        self->widgets[widgetIndex].hidden = false;
        self->widgets[widgetIndex].text = name;

        if (_cState->lastSelectedMods & (1 << checkboxIndex))
        {
            self->activatedWidgets |= (1ULL << widgetIndex);
        }
    }

    // 0x0049E7D3
    static void prepareDraw(Window& self)
    {
        Common::prepareDraw(&self);

        self.activatedWidgets &= ~(1 << widx::checkbox_1 | 1 << widx::checkbox_2 | 1 << widx::checkbox_3 | 1 << widx::checkbox_4);

        self.widgets[widx::checkbox_1].hidden = true;
        self.widgets[widx::checkbox_2].hidden = true;
        self.widgets[widx::checkbox_3].hidden = true;
        self.widgets[widx::checkbox_4].hidden = true;

        if (_cState->trackType & (1 << 7))
        {
            auto trackType = _cState->trackType & ~(1 << 7);
            auto roadObj = ObjectManager::get<RoadObject>(trackType);

            auto args = FormatArguments(self.widgets[Common::widx::caption].textArgs);
            args.push(roadObj->name);

            for (auto i = 0; i < 2; i++)
            {
                if (_cState->modList[i] != 0xFF)
                {
                    auto extraName = ObjectManager::get<RoadExtraObject>(_cState->modList[i])->name;
                    setCheckbox(&self, i, extraName);
                }
            }
        }
        else
        {
            auto trackObj = ObjectManager::get<TrackObject>(_cState->trackType);

            auto args = FormatArguments(self.widgets[Common::widx::caption].textArgs);
            args.push(trackObj->name);

            for (auto i = 0; i < 4; i++)
            {
                if (_cState->modList[i] != 0xFF)
                {
                    auto extraName = ObjectManager::get<TrackExtraObject>(_cState->modList[i])->name;
                    setCheckbox(&self, i, extraName);
                }
            }
        }

        // self->activatedWidgets = activatedWidgets;

        self.widgets[widx::image].hidden = true;
        self.widgets[widx::track].hidden = true;
        self.widgets[widx::track_dropdown].hidden = true;

        self.widgets[widx::image].tooltip = StringIds::null;

        if (_cState->lastSelectedMods & 0xF)
        {
            self.widgets[widx::image].hidden = false;
            self.widgets[widx::track].hidden = false;
            self.widgets[widx::track_dropdown].hidden = false;

            self.widgets[widx::image].tooltip = StringIds::upgrade_track_with_mods;

            if (SceneManager::isNetworkHost())
            {
                if (ToolManager::getToolWindowType() == WindowType::construction)
                {
                    self.widgets[widx::image].tooltip = StringIds::click_track_to_upgrade;
                }
            }
        }

        static StringId modString[] = {
            StringIds::single_section,
            StringIds::block_section,
            StringIds::all_connected_track,
        };

        self.widgets[widx::track].text = modString[enumValue(_cState->lastSelectedTrackModSection)];

        Common::repositionTabs(&self);
    }

    // 0x0049EA3E
    static void draw(Window& self, Gfx::DrawingContext& drawingCtx)
    {
        self.draw(drawingCtx);
        Common::drawTabs(&self, drawingCtx);
        if (_cState->lastSelectedMods & 0xF)
        {
            auto xPos = self.x + self.widgets[widx::image].left + 1;
            auto yPos = self.y + self.widgets[widx::image].top + 1;
            auto width = self.widgets[widx::image].width();
            auto height = self.widgets[widx::image].height();

            const auto& rt = drawingCtx.currentRenderTarget();
            auto clipped = Gfx::clipRenderTarget(rt, Ui::Rect(xPos, yPos, width, height));
            if (clipped)
            {
                coord_t x = 0x2010;
                coord_t y = 0x2010;

                auto rotCoord = Math::Vector::rotate(Pos2{ x, y }, WindowManager::getCurrentRotation());
                Ui::Point screenPos = { static_cast<int16_t>(rotCoord.y - rotCoord.x), static_cast<int16_t>(((rotCoord.x + rotCoord.y) >> 1) - 460) };

                screenPos.x -= (self.widgets[widx::image].width() / 2);
                screenPos.y -= ((self.widgets[widx::image].width() / 2) + 16);
                clipped->x += screenPos.x;
                clipped->y += screenPos.y;

                drawingCtx.pushRenderTarget(*clipped);

                const auto previewPos = World::Pos3(256 * World::kTileSize, 256 * World::kTileSize, 116 * World::kSmallZStep);

                _byte_522095 = _byte_522095 | (1 << 0);

                if (_cState->trackType & (1 << 7))
                {
                    uint8_t trackType = _cState->trackType & ~(1 << 7);
                    Construction::drawRoad(previewPos, _cState->lastSelectedMods, trackType, 0, WindowManager::getCurrentRotation(), drawingCtx);
                }
                else
                {
                    Construction::drawTrack(previewPos, _cState->lastSelectedMods, _cState->trackType, 0, WindowManager::getCurrentRotation(), drawingCtx);
                }
                _byte_522095 = _byte_522095 & ~(1 << 0);

                drawingCtx.popRenderTarget();
            }
        }

        if (_cState->modCost != 0x80000000 && _cState->modCost != 0)
        {
            FormatArguments args{};
            args.push<uint32_t>(_cState->modCost);

            auto tr = Gfx::TextRenderer(drawingCtx);

            auto point = Point(self.x + 69, self.widgets[widx::image].bottom + self.y + 4);
            tr.drawStringCentred(point, Colour::black, StringIds::build_cost, args);
        }
    }

    void tabReset(Window* self)
    {
        self->callOnMouseDown(Overhead::widx::image, self->widgets[Overhead::widx::image].id);
    }

    static constexpr WindowEventList kEvents = {
        .onClose = Common::onClose,
        .onMouseUp = onMouseUp,
        .onMouseDown = onMouseDown,
        .onDropdown = onDropdown,
        .onUpdate = onUpdate,
        .onToolUpdate = onToolUpdate,
        .toolUp = onToolUp,
        .prepareDraw = prepareDraw,
        .draw = draw,
    };

    const WindowEventList& getEvents()
    {
        return kEvents;
    }
}
