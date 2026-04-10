#include "scenes/LevelTwoScene.hpp"

#include <algorithm>
#include <cmath>

#include "gameplay/CatAssets.hpp"
#include "scenes/KeyboardConfigScene.hpp"
#include "services/SaveManager.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"

using ip = Util::Input;
using k  = Util::Keycode;

class LevelTwoScene::MovingPlankBody final : public IPhysicsBody {
public:
    MovingPlankBody(const glm::vec2& pos, const glm::vec2& half, float speed)
        : m_Pos(pos), m_Half(glm::abs(half)), m_Speed(std::max(0.0f, speed)) {}

    [[nodiscard]] const PhysicsBodyTraits& GetPhysicsTraits() const override {
        static const PhysicsBodyTraits kTraits{BodyType::MOVING_PLATFORM, false, false};
        return kTraits;
    }
    [[nodiscard]] glm::vec2 GetPosition() const override { return m_Pos; }
    void SetPosition(const glm::vec2& pos) override { m_Pos = pos; }
    [[nodiscard]] glm::vec2 GetHalfSize() const override { return m_Half; }

    [[nodiscard]] bool IsSolid() const override { return true; }
    [[nodiscard]] bool IsKinematic() const override { return true; }
    [[nodiscard]] int GetMoveDir() const override { return 0; }

    [[nodiscard]] bool IsActive() const override { return IPhysicsBody::IsActive(); }
    void SetActive(bool active) override { IPhysicsBody::SetActive(active); }
    [[nodiscard]] bool IsFrozen() const override { return IPhysicsBody::IsFrozen(); }
    void Freeze() override { IPhysicsBody::Freeze(); }
    void Unfreeze() override { IPhysicsBody::Unfreeze(); }

    void SetTargetX(float x) { m_TargetX = x; }

    void PhysicsUpdate() override {
        const float dtSec = Util::Time::GetDeltaTimeMs() / 1000.0f;
        const float dx    = m_TargetX - m_Pos.x;
        const float maxStep = m_Speed * std::max(0.0f, dtSec);

        if (std::abs(dx) <= maxStep) {
            m_Desired = {dx, 0.0f};
        } else {
            m_Desired = {(dx > 0.0f ? maxStep : -maxStep), 0.0f};
        }
    }

    [[nodiscard]] glm::vec2 GetDesiredDelta() const override { return m_Desired; }

    void ApplyResolvedDelta(const glm::vec2& delta) override {
        m_Pos += delta;
    }

    void OnCollision(const CollisionInfo& /*info*/) override {}

    void PostUpdate() override {}

private:
    glm::vec2 m_Pos;
    glm::vec2 m_Half;
    glm::vec2 m_Desired = {0.0f, 0.0f};

    float m_TargetX = 0.0f;
    float m_Speed   = 160.0f;
};

LevelTwoScene::LevelTwoScene(SceneServices services)
    : Scene(services) {
    m_LeftFloorSprite = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Level_Cover/LevelTwoScene/LeftFloor.png");
    m_RightFloorSprite = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Level_Cover/LevelTwoScene/RightFloor.png");
    m_CeilingSprite = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Level_Cover/LevelTwoScene/Ceiling.png");

    m_LeftWallSprite = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Level_Cover/Background_lwall.png");
    m_RightWallSprite = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Level_Cover/Background_rwall.png");

    m_KeySprite = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/Key.png");
    m_ButtonSprite = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Level_Cover/LevelTwoScene/ButtonUp.png");

    m_TimerText = std::make_shared<GameText>(
        "TIME 00:00.00", 42, Util::Color::FromRGB(255, 140, 0, 255));

    m_LeftFloorSprite->SetZIndex(4.0f);
    m_RightFloorSprite->SetZIndex(4.0f);
    m_CeilingSprite->SetZIndex(4.0f);
    m_LeftWallSprite->SetZIndex(4.1f);
    m_RightWallSprite->SetZIndex(4.1f);

    m_KeySprite->SetZIndex(18.0f);
    m_ButtonSprite->SetZIndex(17.0f);

    m_TimerText->SetZIndex(40.0f);
    m_TimerText->SetPosition({450.0f, 320.0f});

    m_PlankSprites.reserve(kPlankCount);
    for (int i = 0; i < kPlankCount; ++i) {
        auto plank = std::make_shared<Character>(
            GA_RESOURCE_DIR "/Image/Level_Cover/LevelTwoScene/Plank.png");
        plank->SetZIndex(3.0f + static_cast<float>(i) * 0.01f);
        m_PlankSprites.push_back(plank);
    }
}

bool LevelTwoScene::AabbOverlap(const glm::vec2& aPos, const glm::vec2& aHalf,
                                const glm::vec2& bPos, const glm::vec2& bHalf) {
    return std::abs(aPos.x - bPos.x) < (aHalf.x + bHalf.x) &&
           std::abs(aPos.y - bPos.y) < (aHalf.y + bHalf.y);
}

void LevelTwoScene::SetupStaticBoundaries() {
    constexpr float wallHalfW = 32.0f;
    constexpr float wallHalfH = 360.0f;
    constexpr float floorHalfH = 35.0f;
    constexpr float ceilHalfH = 35.0f;

    m_World.AddStaticBoundary(
        {kRoomLeftX - 15.0f, 0.0f},
        {wallHalfW, wallHalfH},
        BodyType::STATIC_BOUNDARY);
    m_World.AddStaticBoundary(
        {kRoomRightX + 15.0f, 0.0f},
        {wallHalfW, wallHalfH},
        BodyType::STATIC_BOUNDARY);

    // Keep floor colliders horizontally aligned with the visual floor sprites.
    float leftFloorCenterX = -450.0f;
    float leftFloorHalfW   = 150.0f;
    if (m_LeftFloorSprite) {
        leftFloorCenterX = m_LeftFloorSprite->GetPosition().x;
        const float spriteHalfW = std::abs(m_LeftFloorSprite->GetScaledSize().x) * 0.5f;
        if (spriteHalfW > 1.0f) leftFloorHalfW = spriteHalfW;
    }

    float rightFloorCenterX = kRightFloorCenterX;
    float rightFloorHalfW   = (kRightFloorMaxX - kRightFloorMinX) * 0.5f;
    if (m_RightFloorSprite) {
        rightFloorCenterX = m_RightFloorSprite->GetPosition().x;
        const float spriteHalfW = std::abs(m_RightFloorSprite->GetScaledSize().x) * 0.5f;
        if (spriteHalfW > 1.0f) rightFloorHalfW = spriteHalfW;
    }

    m_World.AddStaticBoundary(
        {leftFloorCenterX, kRoomFloorY - floorHalfH},
        {leftFloorHalfW, floorHalfH},
        BodyType::STATIC_BOUNDARY);
    m_World.AddStaticBoundary(
        {rightFloorCenterX, kRoomFloorY - floorHalfH},
        {rightFloorHalfW, floorHalfH},
        BodyType::STATIC_BOUNDARY);

    const float ceilHalfW = (kCeilingRightX - kCeilingLeftX) * 0.5f;
    const float ceilCenterX = (kCeilingLeftX + kCeilingRightX) * 0.5f;

    m_World.AddStaticBoundary(
        {ceilCenterX, kRoomTopY - ceilHalfH},
        {ceilHalfW, ceilHalfH},
        BodyType::STATIC_BOUNDARY);
}

void LevelTwoScene::SpawnPlayers(int count) {
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
        m_World.Register(cat);
    }

    ApplyInitialFormation();
}

void LevelTwoScene::ApplyInitialFormation() const {
    const float spawnY = kRoomFloorY + PlayerCat::kHalfHeight;
    for (int i = 0; i < static_cast<int>(m_Players.size()); ++i) {
        const auto& pb = m_Players[i];
        if (pb.cat == nullptr) continue;

        const float x = -570.0f + static_cast<float>(i) * 38.0f;
        pb.cat->SetPosition({x, spawnY});
    }
}

void LevelTwoScene::SetupPlanks(int playerCount) {
    m_PlankBodies.clear();
    m_PlankTargetX.assign(static_cast<size_t>(kPlankCount), 0.0f);

    const int initialExtended = std::clamp(8 - playerCount, 0, kPlankCount);

    glm::vec2 plankHalf = {24.0f, 12.0f};
    if (!m_PlankSprites.empty() && m_PlankSprites[0] != nullptr) {
        const glm::vec2 size = glm::abs(m_PlankSprites[0]->GetScaledSize());
        if (size.x > 1.0f && size.y > 1.0f) {
            plankHalf = size * 0.5f;
        }
    }

    const float plankWidth = plankHalf.x * 2.0f;
    const float plankPitch = std::max(1.0f, plankWidth - (kPlankSeamInsetPx * 2.0f));
    const float plankY     = kRoomFloorY - plankHalf.y - 3.0f; // -3.0f avoid cat be moved

    float rightFloorCenterX = kRightFloorCenterX;
    float rightFloorHalfW   = (kRightFloorMaxX - kRightFloorMinX) * 0.5f;
    if (m_RightFloorSprite) {
        rightFloorCenterX = m_RightFloorSprite->GetPosition().x;
        const float spriteHalfW = std::abs(m_RightFloorSprite->GetScaledSize().x) * 0.5f;
        if (spriteHalfW > 1.0f) rightFloorHalfW = spriteHalfW;
    }
    const float rightFloorLeftX = rightFloorCenterX - rightFloorHalfW;
    const float collapsedX = rightFloorCenterX;

    m_PlankBodies.reserve(kPlankCount);

    for (int i = 0; i < kPlankCount; ++i) {
        const float finalTargetX = rightFloorLeftX - plankPitch * (static_cast<float>(i) + 0.5f);
        const bool isInitiallyExtended = i < initialExtended;
        const float startX = isInitiallyExtended ? finalTargetX : collapsedX;

        auto body = std::make_shared<MovingPlankBody>(glm::vec2{startX, plankY}, plankHalf, kPlankMoveSpeed);
        body->SetTargetX(startX); // Stay fixed until button is pressed.

        m_PlankBodies.push_back(body);
        m_PlankTargetX[static_cast<size_t>(i)] = finalTargetX;
        m_World.Register(body);
    }

    SyncPlankSprites();
}

void LevelTwoScene::SyncPlankSprites() const {
    const int count = std::min(static_cast<int>(m_PlankSprites.size()), static_cast<int>(m_PlankBodies.size()));
    for (int i = 0; i < count; ++i) {
        if (m_PlankSprites[i] == nullptr || m_PlankBodies[i] == nullptr) continue;
        m_PlankSprites[i]->SetPosition(m_PlankBodies[i]->GetPosition());
    }
}

void LevelTwoScene::OnEnter() {
    const int playerCount = std::clamp(m_Session.GetSelectedPlayerCount(), 2, 8);
    m_Session.SetSelectedPlayerCount(playerCount);

    m_PlayerEntered.assign(static_cast<size_t>(playerCount), false);
    m_EnteredCount = 0;
    m_ClearDone = false;

    m_World.Clear();
    m_ElapsedSec = 0.0f;
    m_ButtonPressed = false;
    m_KeyCarrierIdx = -1;
    m_DoorOpened = false;

    if (m_Actors.Header()) m_Actors.Header()->SetVisible(false);
    if (m_Actors.Floor()) m_Actors.Floor()->SetVisible(false);
    if (m_Actors.TestBox()) {
        m_Actors.TestBox()->SetVisible(false);
        if (m_Actors.TestBox()->GetTextObject()) {
            m_Actors.TestBox()->GetTextObject()->SetVisible(false);
        }
    }

    if (m_Actors.Door()) {
        m_Actors.Door()->SetVisible(true);
        m_Actors.Door()->SetImage(GA_RESOURCE_DIR "/Image/Background/door_close.png");
        m_Actors.Door()->SetPosition({480.0f, kRoomFloorY + kDoorHalf.y});
    }

    for (auto& cat : m_Actors.StartupCats()) {
        if (cat) cat->SetVisible(false);
    }

    // Keep all local surfaces aligned on the same floor surface Y.
    if (m_LeftFloorSprite) {
        const float halfH = m_LeftFloorSprite->GetScaledSize().y * 0.5f;
        m_LeftFloorSprite->SetPosition({-330.0f, kRoomFloorY - halfH});
    }
    if (m_RightFloorSprite) {
        const float halfH = m_RightFloorSprite->GetScaledSize().y * 0.5f;
        m_RightFloorSprite->SetPosition({kRightFloorCenterX, kRoomFloorY - halfH});
    }
    if (m_CeilingSprite) {
        const float halfH = m_CeilingSprite->GetScaledSize().y * 0.5f;
        m_CeilingSprite->SetPosition({271.0f, kRoomTopY + halfH});
    }

    m_LeftWallSprite->SetPosition({kRoomLeftX, 0.0f});
    m_RightWallSprite->SetPosition({kRoomRightX, 0.0f});

    m_KeySprite->SetVisible(true);
    m_KeySprite->SetPosition({480.0f, kRoomFloorY + kDoorHalf.y + 90.0f});

    const float buttonY = kRoomFloorY + kButtonTriggerHalfH;
    m_ButtonSprite->SetImage(GA_RESOURCE_DIR "/Image/Level_Cover/LevelTwoScene/ButtonUp.png");
    m_ButtonSprite->SetPosition({430.0f, buttonY-11.0f});

    m_Actors.Root().AddChild(m_LeftFloorSprite);
    m_Actors.Root().AddChild(m_RightFloorSprite);
    m_Actors.Root().AddChild(m_CeilingSprite);
    m_Actors.Root().AddChild(m_LeftWallSprite);
    m_Actors.Root().AddChild(m_RightWallSprite);
    m_Actors.Root().AddChild(m_KeySprite);
    m_Actors.Root().AddChild(m_ButtonSprite);

    for (const auto& plank : m_PlankSprites) {
        m_Actors.Root().AddChild(plank);
    }

    m_Actors.Root().AddChild(m_TimerText);
    UpdateTimerText();

    SpawnPlayers(playerCount);
    SetupStaticBoundaries();
    SetupPlanks(playerCount);

    LOG_INFO("LevelTwoScene::OnEnter players={}", playerCount);
}

void LevelTwoScene::OnExit() {
    for (auto& pb : m_Players) {
        if (pb.cat) m_Actors.Root().RemoveChild(pb.cat);
    }
    m_Players.clear();

    m_World.UnfreezeAll();
    m_World.Clear();

    m_Actors.Root().RemoveChild(m_LeftFloorSprite);
    m_Actors.Root().RemoveChild(m_RightFloorSprite);
    m_Actors.Root().RemoveChild(m_CeilingSprite);
    m_Actors.Root().RemoveChild(m_LeftWallSprite);
    m_Actors.Root().RemoveChild(m_RightWallSprite);
    m_Actors.Root().RemoveChild(m_KeySprite);
    m_Actors.Root().RemoveChild(m_ButtonSprite);
    m_Actors.Root().RemoveChild(m_TimerText);

    for (const auto& plank : m_PlankSprites) {
        m_Actors.Root().RemoveChild(plank);
    }
    m_PlankBodies.clear();
    m_PlankTargetX.clear();

    if (m_Actors.Header()) m_Actors.Header()->SetVisible(false);
    if (m_Actors.Floor()) m_Actors.Floor()->SetVisible(false);

    if (m_Actors.TestBox()) {
        m_Actors.TestBox()->SetVisible(false);
        if (m_Actors.TestBox()->GetTextObject()) {
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

void LevelTwoScene::PauseGameplay() {
    m_World.FreezeAll();
}

void LevelTwoScene::ResumeGameplay() {
    m_World.UnfreezeAll();
}

void LevelTwoScene::HandlePlayerInput() const {
    for (const auto& pb : m_Players) {
        if (pb.cat == nullptr || !pb.cat->IsActive() || !pb.cat->GetInputEnabled()) continue;

        int moveDir = 0;
        const bool goLeft  = (pb.key.left != k::UNKNOWN) && ip::IsKeyPressed(pb.key.left);
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

void LevelTwoScene::UpdateTimerText() const {
    m_TimerText->SetText("TIME " + SaveManager::FormatTime(m_ElapsedSec));
}

void LevelTwoScene::TryActivateButton() {
    if (m_ButtonPressed) return;

    const glm::vec2 buttonPos = m_ButtonSprite->GetPosition();
    const glm::vec2 buttonHalf = {kButtonTriggerHalfW, kButtonTriggerHalfH};

    for (const auto& pb : m_Players) {
        if (pb.cat == nullptr || !pb.cat->IsActive()) continue;

        if (!AabbOverlap(pb.cat->GetPosition(), pb.cat->GetHalfSize(), buttonPos, buttonHalf)) {
            continue;
        }

        m_ButtonPressed = true;
        m_ButtonSprite->SetImage(GA_RESOURCE_DIR "/Image/Level_Cover/LevelTwoScene/ButtonDown.png");

        for (int i = 0; i < static_cast<int>(m_PlankBodies.size()); ++i) {
            if (m_PlankBodies[i] == nullptr) continue;
            m_PlankBodies[i]->SetTargetX(m_PlankTargetX[static_cast<size_t>(i)]);
        }
        LOG_INFO("LevelTwoScene: button pressed, extending bridge");
        break;
    }
}

void LevelTwoScene::UpdateFallenPlayers() {
    for (int i = 0; i < static_cast<int>(m_Players.size()); ++i) {
        auto& cat = m_Players[i].cat;
        if (cat == nullptr || !cat->IsActive()) continue;

        if (cat->GetPosition().y >= kFallTeleportThresholdY) continue;

        cat->SetPosition(kTeleportPos);
        cat->SetVisible(true);

        LOG_INFO("LevelTwoScene: player {} teleported after fall", i);
    }
}

void LevelTwoScene::TryPickKey() {
    if (m_KeyCarrierIdx >= 0) return;

    for (int i = 0; i < static_cast<int>(m_Players.size()); ++i) {
        if (m_Players[i].cat == nullptr || !m_Players[i].cat->IsActive()) continue;

        if (AabbOverlap(m_Players[i].cat->GetPosition(), m_Players[i].cat->GetHalfSize(),
                        m_KeySprite->GetPosition(), kKeyHalf)) {
            m_KeyCarrierIdx = i;
            return;
        }
    }
}

void LevelTwoScene::UpdateKeyFollow() const {
    if (m_KeyCarrierIdx < 0 || m_KeyCarrierIdx >= static_cast<int>(m_Players.size())) return;

    const auto& carrier = m_Players[m_KeyCarrierIdx].cat;
    if (carrier == nullptr || !carrier->IsActive()) return;

    m_KeySprite->SetPosition(carrier->GetPosition() + glm::vec2{-28.0f, 28.0f});
}

void LevelTwoScene::TryOpenDoorAndClear() {
    if (m_KeyCarrierIdx < 0 || m_KeyCarrierIdx >= static_cast<int>(m_Players.size())) return;
    if (m_Actors.Door() == nullptr || m_DoorOpened) return;

    const auto& carrier = m_Players[m_KeyCarrierIdx].cat;
    if (carrier == nullptr || !carrier->IsActive()) return;

    const glm::vec2 doorPos  = m_Actors.Door()->GetPosition();
    const glm::vec2 doorHalf = glm::abs(m_Actors.Door()->GetScaledSize()) * 0.5f;

    const bool carrierTouchDoor = AabbOverlap(
        carrier->GetPosition(), carrier->GetHalfSize(),
        doorPos, doorHalf);

    if (carrierTouchDoor) {
        m_DoorOpened = true;
        m_Actors.Door()->SetImage(GA_RESOURCE_DIR "/Image/Background/door_open.png");
        m_KeySprite->SetVisible(false);
    }
}

void LevelTwoScene::UpdateDoorEntryAndClear() {
    if (!m_DoorOpened || m_Actors.Door() == nullptr) return;

    const glm::vec2 doorPos  = m_Actors.Door()->GetPosition();
    const glm::vec2 doorHalf = glm::abs(m_Actors.Door()->GetScaledSize()) * 0.5f;
    const int totalPlayers   = static_cast<int>(m_Players.size());

    for (int i = 0; i < totalPlayers; ++i) {
        if (i >= static_cast<int>(m_PlayerEntered.size())) break;
        if (m_PlayerEntered[static_cast<size_t>(i)]) continue;

        auto& pb = m_Players[i];
        auto& cat = pb.cat;
        if (cat == nullptr || !cat->IsActive()) continue;

        const bool touchingDoor = AabbOverlap(
            cat->GetPosition(), cat->GetHalfSize(),
            doorPos, doorHalf);

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

void LevelTwoScene::Update() {
    if (ip::IsKeyDown(k::ESCAPE)) {
        RequestSceneOp({SceneOpType::PushOverlay, SceneId::LevelExit});
        return;
    }

    HandlePlayerInput();
    m_World.Update();
    SyncPlankSprites();

    m_ElapsedSec += Util::Time::GetDeltaTimeMs() / 1000.0f;
    UpdateTimerText();

    UpdateFallenPlayers();
    TryActivateButton();

    TryPickKey();
    UpdateKeyFollow();
    TryOpenDoorAndClear();
    UpdateDoorEntryAndClear();

    if (m_ClearDone) {
        RequestSceneOp({SceneOpType::ClearToAndGoTo, SceneId::LevelSelect});
        return;
    }
}


