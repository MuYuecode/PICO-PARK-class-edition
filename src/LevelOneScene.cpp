#include "LevelOneScene.hpp"
#include <algorithm>
#include <cmath>
#include "CatAssets.hpp"
#include "KeyboardConfigScene.hpp"
#include "SaveManager.hpp"
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

bool LevelOneScene::AabbOverlap(const glm::vec2& aPos, const glm::vec2& aHalf,
                                const glm::vec2& bPos, const glm::vec2& bHalf) {
    return std::abs(aPos.x - bPos.x) < (aHalf.x + bHalf.x) &&
           std::abs(aPos.y - bPos.y) < (aHalf.y + bHalf.y);
}

void LevelOneScene::SetupStaticBoundaries() {
    constexpr float wallHalfW  = 32.0f;
    constexpr float wallHalfH  = 360.0f;
    constexpr float floorHalfH = 35.0f;
    constexpr float ceilHalfH  = 35.0f;

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

    // Floor collider: top edge = floorSurfaceY.
    m_World.AddStaticBoundary({0.0f, floorSurfaceY - floorHalfH}, {600.0f, floorHalfH});

    // Ceiling collider: bottom edge = ceilingSurfaceY.
    m_World.AddStaticBoundary({0.0f, ceilingSurfaceY + ceilHalfH}, {600.0f, ceilHalfH});

    // Side walls.
    m_World.AddStaticBoundary({kRoomLeftX - wallHalfW,  0.0f}, {wallHalfW, wallHalfH});
    m_World.AddStaticBoundary({kRoomRightX + wallHalfW, 0.0f}, {wallHalfW, wallHalfH});
}


void LevelOneScene::SpawnPlayers(int count) {
    m_Players.clear();
    m_Players.reserve(static_cast<size_t>(count));

    for (int i = 0; i < count; ++i) {
        const std::string color = m_Actors.CatColorOrder()[static_cast<size_t>(i)];
        auto cat = std::make_shared<PlayerCat>(
            CatAssets::BuildFullAnimPaths(color),
            k::UNKNOWN, k::UNKNOWN, k::UNKNOWN);

        PlayerBinding pb;
        pb.cat = cat;
        pb.key = m_Session.GetAppliedKeyConfigs()[static_cast<size_t>(i)];

        if (i == 0 && pb.key.left == k::UNKNOWN && pb.key.right == k::UNKNOWN) {
            pb.key = KeyboardConfigScene::k_Default1P;
        } else if (i == 1 && pb.key.left == k::UNKNOWN && pb.key.right == k::UNKNOWN) {
            pb.key = KeyboardConfigScene::k_Default2P;
        }

        cat->SetZIndex(20.0f + static_cast<float>(i) * 0.01f);
        m_Players.push_back(pb);
        m_Actors.Root().AddChild(cat);
    }

    ApplyInitialFormation();

    for (auto& pb : m_Players) {
        m_World.Register(pb.cat);
    }
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

    if (m_Actors.TestBox()) {
        m_Actors.TestBox()->SetVisible(false);
        if (m_Actors.TestBox()->GetTextObject()) {
            m_Actors.TestBox()->GetTextObject()->SetVisible(false);
        }
    }

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
    m_KeySprite->SetPosition(m_KeyInitialPos);

    // Spawn boxes using real physics half height, no hardcoded magic number.
    const float boxAY = floorSurfaceY + m_BoxA->GetHalfSize().y;
    const float boxBY = floorSurfaceY + m_BoxB->GetHalfSize().y;
    m_BoxA->SetPosition({-120.0f, boxAY});
    m_BoxB->SetPosition({ 120.0f, boxBY});
    m_BoxA->SetWorld(&m_World);
    m_BoxB->SetWorld(&m_World);

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

    LOG_INFO("LevelOneScene::OnEnter players={}", playerCount);
}


void LevelOneScene::OnExit() {
    for (auto& pb : m_Players) {
        if (pb.cat) m_Actors.Root().RemoveChild(pb.cat);
    }
    m_Players.clear();

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
    if (m_Actors.TestBox() != nullptr) {
        m_Actors.TestBox()->SetVisible(false);
        if (m_Actors.TestBox()->GetTextObject() != nullptr) {
            m_Actors.TestBox()->GetTextObject()->SetVisible(false);
        }
    }


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
    for (const auto& pb : m_Players) {
        if (pb.cat == nullptr) continue;

        int moveDir = 0;
        const bool goLeft  = (pb.key.left  != k::UNKNOWN) && ip::IsKeyPressed(pb.key.left);
        const bool goRight = (pb.key.right != k::UNKNOWN) && ip::IsKeyPressed(pb.key.right);

        if (goLeft && !goRight) moveDir = -1;
        else if (goRight && !goLeft) moveDir = 1;

        pb.cat->SetMoveDir(moveDir);

        const bool wantJump = (pb.key.jump != k::UNKNOWN) && ip::IsKeyDown(pb.key.jump);
        if (pb.cat->IsGrounded() && wantJump) {
            pb.cat->Jump();
        }
    }
}

void LevelOneScene::UpdateTimerText() const {
    m_TimerText->SetText("TIME " + SaveManager::FormatTime(m_ElapsedSec));
}

void LevelOneScene::TryPickKey() {
    if (m_KeyCarrierIdx >= 0) return;

    for (int i = 0; i < static_cast<int>(m_Players.size()); ++i) {
        if (m_Players[i].cat == nullptr) continue;
        if (AabbOverlap(m_Players[i].cat->GetPosition(), m_Players[i].cat->GetHalfSize(),
                        m_KeySprite->GetPosition(), kKeyHalf)) {
            m_KeyCarrierIdx = i;
            return;
        }
    }
}

void LevelOneScene::UpdateKeyFollow() const {
    if (m_KeyCarrierIdx < 0 || m_KeyCarrierIdx >= static_cast<int>(m_Players.size())) return;
    const auto& carrier = m_Players[m_KeyCarrierIdx].cat;
    if (carrier == nullptr) return;

    m_KeySprite->SetPosition(carrier->GetPosition() + glm::vec2{-28.0f, 28.0f});
}

void LevelOneScene::TryOpenDoorAndClear() {
    if (m_KeyCarrierIdx < 0 || m_KeyCarrierIdx >= static_cast<int>(m_Players.size())) return;
    if (m_Actors.Door() == nullptr || m_DoorOpened) return;

    const auto& carrier = m_Players[m_KeyCarrierIdx].cat;
    if (carrier == nullptr) return;

    const glm::vec2 doorPos  = m_Actors.Door()->GetPosition();
    const glm::vec2 doorHalf = glm::abs(m_Actors.Door()->GetScaledSize()) * 0.5f;

    const bool carrierTouchDoor = AabbOverlap(
        carrier->GetPosition(), carrier->GetHalfSize(),
        doorPos, doorHalf
    );

    if (carrierTouchDoor) {
        m_DoorOpened = true;
        m_Actors.Door()->SetImage(GA_RESOURCE_DIR "/Image/Background/door_open.png");
        m_KeySprite->SetVisible(false);
    }
}

void LevelOneScene::UpdateDoorEntryAndClear() {
    if (!m_DoorOpened || m_Actors.Door() == nullptr) return;

    const glm::vec2 doorPos  = m_Actors.Door()->GetPosition();
    const glm::vec2 doorHalf = glm::abs(m_Actors.Door()->GetScaledSize()) * 0.5f;
    const int totalPlayers   = static_cast<int>(m_Players.size());

    for (int i = 0; i < totalPlayers; ++i) {
        if (i >= static_cast<int>(m_PlayerEntered.size())) break;
        if (m_PlayerEntered[static_cast<size_t>(i)]) continue;

        auto& pb  = m_Players[i];
        auto& cat = pb.cat;
        if (cat == nullptr) continue;

        const bool touchingDoor = AabbOverlap(
            cat->GetPosition(), cat->GetHalfSize(),
            doorPos, doorHalf
        );

        // Entering the door requires pressing UP.
        const bool pressedUp = (pb.key.up != k::UNKNOWN) && ip::IsKeyDown(pb.key.up);

        if (!(touchingDoor && pressedUp)) continue;

        m_PlayerEntered[static_cast<size_t>(i)] = true;
        ++m_EnteredCount;

        cat->SetVisible(false);
        cat->SetInputEnabled(false);
        cat->SetActive(false);
        cat->SetPosition({640.0f, -360.0f});
    }

    if (!m_ClearDone && m_EnteredCount == totalPlayers) {
        SaveManager::UpdateBestTime(kLevelIndex, m_Session.GetSelectedPlayerCount(), m_ElapsedSec);
        m_ClearDone = true;
    }
}

SceneId LevelOneScene::Update() {
    if (ip::IsKeyDown(k::ESCAPE)) {
        RequestSceneOp({SceneOpType::PushOverlay, SceneId::LevelExit});
        return SceneId::None;
    }

    HandlePlayerInput();
    m_World.Update();

    m_ElapsedSec += Util::Time::GetDeltaTimeMs() / 1000.0f;
    UpdateTimerText();

    TryPickKey();
    UpdateKeyFollow();
    TryOpenDoorAndClear();
    UpdateDoorEntryAndClear();
    if (m_ClearDone) {
        return SceneId::LevelSelect;
    }

    return SceneId::None;
}