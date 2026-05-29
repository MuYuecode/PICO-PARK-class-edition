#include "scenes/LevelOneScene.hpp"
#include "game/BoundaryFactory.hpp"
#include <algorithm>
#include "scenes/LevelSceneHelpers.hpp"
#include "systems/SaveManager.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"

using ip = Util::Input;
using k  = Util::Keycode;

LevelOneScene::LevelOneScene(SceneServices services)
    : Scene(services) {
    m_FloorSprite = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/LevelOneScene/Floor.png");
    m_LeftWallSprite = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/Background_lwall.png");
    m_RightWallSprite = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/Background_rwall.png");
    m_CeilingSprite = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/Ceiling.png");

    m_KeySprite = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/Key.png");

    m_BoxA = std::make_shared<PushableBox>(GA_RESOURCE_DIR "/Image/Level_Cover/LevelOneScene/Box.png", 1);
    m_BoxB = std::make_shared<PushableBox>(GA_RESOURCE_DIR "/Image/Level_Cover/LevelOneScene/Box.png", 1);

    m_TimerText = std::make_shared<GameText>("TIME 00:00.00", 42, Util::Color::FromRGB(255, 140, 0, 255));

    m_FloorSprite->SetZIndex(4.0f);
    m_LeftWallSprite->SetZIndex(4.1f);
    m_RightWallSprite->SetZIndex(4.1f);
    m_CeilingSprite->SetZIndex(4.0f);

    m_BoxA->SetZIndex(15.0f);
    m_BoxB->SetZIndex(15.0f);

    m_KeySprite->SetZIndex(18.0f);

    m_TimerText->SetZIndex(40.0f);
    m_TimerText->SetPosition({450.0f, 320.0f});
}

void LevelOneScene::SetupStaticBoundaries() {
    // Derive the physical floor surface from the visual floor sprite.
    // This keeps collider and art aligned.
    float floorSurfaceY = kRoomFloorY;
    if (m_FloorSprite) {
        floorSurfaceY = m_FloorSprite->GetPosition().y
                      + m_FloorSprite->GetScaledSize().y * 0.5f;
    }

    float ceilingSurfaceY = kRoomTopY;
    if (m_CeilingSprite) {
        ceilingSurfaceY = m_CeilingSprite->GetPosition().y
                        - m_CeilingSprite->GetScaledSize().y * 0.5f;
    }

    BoundaryFactory::AddStaticRoomBoundaries(
        m_World,
        floorSurfaceY,
        ceilingSurfaceY,
        LevelGeometryPreset::kLevelOneRoom);
}


void LevelOneScene::SpawnPlayers(int count) {
    LevelSceneHelpers::SpawnPlayerBindings(m_Players, count, m_Actors, m_Session, m_World);
    ApplyInitialFormation();
}

void LevelOneScene::ApplyInitialFormation() {
    const float playerY = kRoomFloorY + PlayerCat::kHalfHeight;

    for (int i = 0; i < static_cast<int>(m_Players.size()); ++i) {
        auto& pb = m_Players[i];
        if (pb.cat == nullptr) continue;

        const float x = -40.0f + static_cast<float>(i) * 45.0f;
        pb.cat->SetPosition({x, playerY});
    }
}

void LevelOneScene::OnEnter() {
    const int playerCount = std::clamp(m_Session.GetSelectedPlayerCount(), 2, 8);
    m_Session.SetSelectedPlayerCount(playerCount);

    m_BoxA->SetRequiredPushers(playerCount - 1);
    m_BoxB->SetRequiredPushers(playerCount);

    m_PlayerEntered.assign(static_cast<size_t>(playerCount), false);
    m_EnteredCount = 0;
    m_ClearDone = false;

    m_World.Clear();
    m_ElapsedSec = 0.0f;
    m_KeyCarrierIdx = -1;
    m_DoorOpened = false;

    if (m_Actors.Header()) m_Actors.Header()->SetVisible(false);
    if (m_Actors.Floor())  m_Actors.Floor()->SetVisible(false);

    if (m_Actors.Door()) {
        m_Actors.Door()->SetVisible(true);
        m_Actors.Door()->SetImage(GA_RESOURCE_DIR "/Image/Background/door_close.png");
        m_Actors.Door()->SetPosition(m_DoorPos);
    }

    for (auto& cat : m_Actors.StartupCats()) {
        if (cat) cat->SetVisible(false);
    }

    // Place floor so its top edge is exactly kRoomFloorY.
    float floorSurfaceY = kRoomFloorY;
    if (m_FloorSprite) {
        const float floorHalfH = m_FloorSprite->GetScaledSize().y * 0.5f;
        m_FloorSprite->SetPosition({0.0f, kRoomFloorY - floorHalfH});
        floorSurfaceY = m_FloorSprite->GetPosition().y + floorHalfH;
    }

    // Place ceiling so its bottom edge is kRoomTopY.
    if (m_CeilingSprite) {
        const float ceilHalfH = m_CeilingSprite->GetScaledSize().y * 0.5f;
        m_CeilingSprite->SetPosition({0.0f, kRoomTopY + ceilHalfH});
    }

    m_LeftWallSprite->SetPosition({kRoomLeftX, 0.0f});
    m_RightWallSprite->SetPosition({kRoomRightX, 0.0f});
    // Reset key visual state when re-entering level after a clear/retry.
    m_KeySprite->SetVisible(true);
    m_KeySprite->SetPosition(m_KeyInitialPos);

    // Spawn boxes using real physics half height, no hardcoded magic number.
    const float boxAY = floorSurfaceY + m_BoxA->GetHalfSize().y;
    const float boxBY = floorSurfaceY + m_BoxB->GetHalfSize().y;
    m_BoxA->SetPosition({-120.0f, boxAY});
    m_BoxB->SetPosition({ 120.0f, boxBY});
    m_BoxA->SetPushQuery(&m_World);
    m_BoxB->SetPushQuery(&m_World);

    m_Actors.Root().AddChild(m_FloorSprite);
    m_Actors.Root().AddChild(m_LeftWallSprite);
    m_Actors.Root().AddChild(m_RightWallSprite);
    m_Actors.Root().AddChild(m_CeilingSprite);
    m_Actors.Root().AddChild(m_KeySprite);

    m_Actors.Root().AddChild(m_BoxA);
    m_Actors.Root().AddChild(m_BoxB);

    if (m_BoxA->GetTextObject()) {
        m_BoxA->GetTextObject()->SetZIndex(16.0f);
        m_Actors.Root().AddChild(m_BoxA->GetTextObject());
    }
    if (m_BoxB->GetTextObject()) {
        m_BoxB->GetTextObject()->SetZIndex(16.0f);
        m_Actors.Root().AddChild(m_BoxB->GetTextObject());
    }

    m_Actors.Root().AddChild(m_TimerText);
    UpdateTimerText();

    SpawnPlayers(playerCount);

    // Ensure player spawn also lands on the same physical floor surface.
    for (int i = 0; i < static_cast<int>(m_Players.size()); ++i) {
        auto& pb = m_Players[i];
        if (pb.cat == nullptr) continue;
        const float x = -40.0f + static_cast<float>(i) * 45.0f;
        const float y = floorSurfaceY + PlayerCat::kHalfHeight;
        pb.cat->SetPosition({x, y});
    }

    m_World.Register(m_BoxA);
    m_World.Register(m_BoxB);
    SetupStaticBoundaries();
    SetupHackMenu();
    m_HackMenu.AddToRoot(m_Actors.Root());

    LOG_INFO("LevelOneScene::OnEnter players={}", playerCount);
}


void LevelOneScene::OnExit() {
    m_HackMenu.RemoveFromRoot(m_Actors.Root());

    for (auto& pb : m_Players) {
        if (pb.cat) m_Actors.Root().RemoveChild(pb.cat);
    }
    m_Players.clear();

    // Ensure paused overlay state does not persist frozen flags into next enter.
    m_World.UnfreezeAll();
    m_World.Clear();

    m_Actors.Root().RemoveChild(m_FloorSprite);
    m_Actors.Root().RemoveChild(m_LeftWallSprite);
    m_Actors.Root().RemoveChild(m_RightWallSprite);
    m_Actors.Root().RemoveChild(m_CeilingSprite);
    m_Actors.Root().RemoveChild(m_KeySprite);
    m_Actors.Root().RemoveChild(m_BoxA);
    m_Actors.Root().RemoveChild(m_BoxB);
    m_Actors.Root().RemoveChild(m_TimerText);

    if (m_BoxA && m_BoxA->GetTextObject()) m_Actors.Root().RemoveChild(m_BoxA->GetTextObject());
    if (m_BoxB && m_BoxB->GetTextObject()) m_Actors.Root().RemoveChild(m_BoxB->GetTextObject());

    m_Actors.Header()->SetVisible(false);
    m_Actors.Floor()->SetVisible(false);
    if (m_Actors.Door()) {
        m_Actors.Door()->SetVisible(false);
        m_Actors.Door()->SetImage(GA_RESOURCE_DIR "/Image/Background/door_close.png");
    }

    for (auto& cat : m_Actors.StartupCats()) {
        if (cat) cat->SetVisible(true);
    }
}

void LevelOneScene::PauseGameplay() {
    m_World.FreezeAll();
}

void LevelOneScene::ResumeGameplay() {
    m_World.UnfreezeAll();
}

void LevelOneScene::HandlePlayerInput() const {
    LevelSceneHelpers::HandlePlayerInput(m_Players, m_Audio);
}

void LevelOneScene::UpdateTimerText() const {
    m_TimerText->SetText("TIME " + SaveManager::FormatTime(m_ElapsedSec));
}

void LevelOneScene::SetupHackMenu() {
    m_HackMenu.SetItems({
        {"TP KEY", false, false, {}, [this]() { HackTeleportToKey(); }},
        {"TP DOOR", false, false, {}, [this]() { HackTeleportToDoor(); }},
        {"GET KEY", false, false, {}, [this]() { HackGrantKey(); }},
        {"OPEN DOOR", false, false, {}, [this]() { HackOpenDoor(); }},
        {"BOX NEED 1", false, false, {}, [this]() { HackSetBoxesOnePusher(); }},
        {"BOX ASIDE", false, false, {}, [this]() { HackMoveBoxesAside(); }},
    });
}

void LevelOneScene::TeleportPlayersTo(const glm::vec2& pos) const {
    LevelSceneHelpers::TeleportActivePlayersTo(m_Players, pos);
}

void LevelOneScene::HackTeleportToKey() const {
    if (m_KeyCarrierIdx >= 0) return;
    if (m_KeySprite == nullptr) return;
    TeleportPlayersTo(m_KeySprite->GetPosition() + glm::vec2{0.0f, PlayerCat::kHalfHeight});
}

void LevelOneScene::HackTeleportToDoor() const {
    if (m_Actors.Door() == nullptr) return;
    TeleportPlayersTo(m_Actors.Door()->GetPosition());
}

void LevelOneScene::HackGrantKey() {
    if (m_Players.empty() || m_Players[0].cat == nullptr) return;
    m_KeyCarrierIdx = 0;
    UpdateKeyFollow();
}

void LevelOneScene::HackOpenDoor() {
    if (m_Actors.Door() == nullptr || m_DoorOpened) return;
    m_DoorOpened = true;
    m_Actors.Door()->SetImage(GA_RESOURCE_DIR "/Image/Background/door_open.png");
    if (m_KeySprite != nullptr) {
        m_KeySprite->SetVisible(false);
    }
    m_Audio.PlaySe(SoundEffect::Door);
}

void LevelOneScene::HackSetBoxesOnePusher() const {
    if (m_BoxA != nullptr) m_BoxA->SetRequiredPushers(1);
    if (m_BoxB != nullptr) m_BoxB->SetRequiredPushers(1);
}

void LevelOneScene::HackMoveBoxesAside() const {
    if (m_BoxA != nullptr) {
        m_BoxA->SetPosition({kRoomLeftX + 120.0f, kRoomFloorY + m_BoxA->GetHalfSize().y});
    }
    if (m_BoxB != nullptr) {
        m_BoxB->SetPosition({kRoomRightX - 120.0f, kRoomFloorY + m_BoxB->GetHalfSize().y});
    }
}

void LevelOneScene::TryPickKey() {
    if (m_KeyCarrierIdx >= 0) return;

    for (int i = 0; i < static_cast<int>(m_Players.size()); ++i) {
        if (m_Players[i].cat == nullptr) continue;
        if (LevelSceneHelpers::AabbOverlap(
                m_Players[i].cat->GetPosition(), m_Players[i].cat->GetHalfSize(),
                m_KeySprite->GetPosition(), kKeyHalf)) {
            m_KeyCarrierIdx = i;
            return;
        }
    }
}

void LevelOneScene::UpdateKeyFollow() const {
    if (m_KeyCarrierIdx < 0 || m_KeyCarrierIdx >= static_cast<int>(m_Players.size())) return;
    if (m_DoorOpened) return;
    const auto& carrier = m_Players[m_KeyCarrierIdx].cat;
    if (carrier == nullptr) return;

    m_KeySprite->SetPosition(carrier->GetPosition() + glm::vec2{-28.0f, 28.0f});
}

void LevelOneScene::TryOpenDoorAndClear() {
    LevelSceneHelpers::TryOpenDoorWithKeyCarrier(
        m_Players, m_KeyCarrierIdx, m_Actors.Door(), m_KeySprite, m_DoorOpened, m_Audio);
}

void LevelOneScene::UpdateDoorEntryAndClear() {
    if (!m_DoorOpened || m_Actors.Door() == nullptr) return;
    LevelSceneHelpers::UpdateDoorEntryAndClear(
        m_Players,
        m_PlayerEntered,
        m_EnteredCount,
        m_ClearDone,
        m_Actors.Door(),
        m_Session.GetSelectedPlayerCount(),
        kLevelIndex,
        m_ElapsedSec,
        m_Audio);
}

void LevelOneScene::Update() {
    if (ip::IsKeyDown(k::ESCAPE)) {
        RequestSceneOp({SceneOpType::PushOverlay, SceneId::LevelExit});
        return;
    }

    HandlePlayerInput();
    m_World.Update();

    m_ElapsedSec += Util::Time::GetDeltaTimeMs() / 1000.0f;
    UpdateTimerText();

    TryPickKey();
    UpdateKeyFollow();
    TryOpenDoorAndClear();
    UpdateDoorEntryAndClear();
    m_HackMenu.Update();
    if (m_ClearDone) {
        RequestSceneOp({SceneOpType::ClearToAndGoTo, SceneId::LevelSelect});
        return;
    }
}
