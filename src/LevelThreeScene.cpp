#include "LevelThreeScene.hpp"

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

namespace {
constexpr float kLiftSpeed = 80.0f;
constexpr float kMob2Speed = 90.0f;
constexpr float kMob1RiseSpeed = 35.0f;
constexpr float kSpawnYOffset = 0.0f;
constexpr float kFallDeathYOffset = 120.0f;
constexpr float kLiftEndpointInset = 4.0f;
constexpr float kHazardContactSlack = 4.0f;

float SpriteHalfW(const std::shared_ptr<Character>& obj, float fallback) {
    if (obj == nullptr) return fallback;
    return std::max(1.0f, std::abs(obj->GetScaledSize().x) * 0.5f);
}

float SpriteHalfH(const std::shared_ptr<Character>& obj, float fallback) {
    if (obj == nullptr) return fallback;
    return std::max(1.0f, std::abs(obj->GetScaledSize().y) * 0.5f);
}

void PlaceSpriteTop(const std::shared_ptr<Character>& obj, float x, float topY) {
    if (obj == nullptr) return;
    const float halfH = SpriteHalfH(obj, 20.0f);
    obj->SetPosition({x, topY - halfH});
}

float SpriteTopY(const std::shared_ptr<Character>& obj, float fallback) {
    if (obj == nullptr) return fallback;
    return obj->GetPosition().y + SpriteHalfH(obj, 20.0f);
}
} // namespace

class LevelThreeScene::MovingLiftBody final : public IPhysicsBody {
public:
    MovingLiftBody(const glm::vec2& pos, const glm::vec2& half,
                   float lowY, float highY, float speed)
        : m_Pos(pos),
          m_Half(glm::abs(half)),
          m_LowY(std::min(lowY, highY)),
          m_HighY(std::max(lowY, highY)),
          m_Speed(std::max(1.0f, speed)) {}

    [[nodiscard]] BodyType GetBodyType() const override { return BodyType::MOVING_PLATFORM; }
    [[nodiscard]] glm::vec2 GetPosition() const override { return m_Pos; }
    void SetPosition(const glm::vec2& pos) override { m_Pos = pos; }
    [[nodiscard]] glm::vec2 GetHalfSize() const override { return m_Half; }

    [[nodiscard]] bool IsSolid() const override { return true; }
    [[nodiscard]] bool IsKinematic() const override { return true; }

    void PhysicsUpdate() override {
        const float dtSec = Util::Time::GetDeltaTimeMs() / 1000.0f;
        const float step  = m_Speed * std::max(0.0f, dtSec) * m_Direction;
        float targetY = m_Pos.y + step;

        if (targetY >= m_HighY) {
            targetY = m_HighY;
            m_Direction = -1.0f;
        } else if (targetY <= m_LowY) {
            targetY = m_LowY;
            m_Direction = 1.0f;
        }

        m_Desired = {0.0f, targetY - m_Pos.y};
    }

    [[nodiscard]] glm::vec2 GetDesiredDelta() const override { return m_Desired; }

    void ApplyResolvedDelta(const glm::vec2& delta) override {
        m_Pos += delta;
    }

private:
    glm::vec2 m_Pos;
    glm::vec2 m_Half;
    glm::vec2 m_Desired = {0.0f, 0.0f};

    float m_LowY = 0.0f;
    float m_HighY = 0.0f;
    float m_Speed = 80.0f;
    float m_Direction = 1.0f;
};

class LevelThreeScene::PatrolMobBody final : public IPhysicsBody {
public:
    PatrolMobBody(const glm::vec2& pos, const glm::vec2& half,
                  float minX, float maxX, float speed)
        : m_Pos(pos),
          m_Half(glm::abs(half)),
          m_MinX(std::min(minX, maxX)),
          m_MaxX(std::max(minX, maxX)),
          m_Speed(std::max(1.0f, speed)) {}

    [[nodiscard]] BodyType GetBodyType() const override { return BodyType::PATROL_ENEMY; }
    [[nodiscard]] glm::vec2 GetPosition() const override { return m_Pos; }
    void SetPosition(const glm::vec2& pos) override { m_Pos = pos; }
    [[nodiscard]] glm::vec2 GetHalfSize() const override { return m_Half; }

    [[nodiscard]] bool IsSolid() const override { return true; }
    [[nodiscard]] bool IsKinematic() const override { return true; }

    void PhysicsUpdate() override {
        const float dtSec = Util::Time::GetDeltaTimeMs() / 1000.0f;
        const float step  = m_Speed * std::max(0.0f, dtSec) * m_Direction;
        float targetX = m_Pos.x + step;

        if (targetX >= m_MaxX) {
            targetX = m_MaxX;
            m_Direction = -1.0f;
        } else if (targetX <= m_MinX) {
            targetX = m_MinX;
            m_Direction = 1.0f;
        }

        m_Desired = {targetX - m_Pos.x, 0.0f};
    }

    [[nodiscard]] glm::vec2 GetDesiredDelta() const override { return m_Desired; }
    [[nodiscard]] int GetFacingDir() const { return (m_Direction >= 0.0f) ? 1 : -1; }

    void ApplyResolvedDelta(const glm::vec2& delta) override {
        m_Pos += delta;
    }

private:
    glm::vec2 m_Pos;
    glm::vec2 m_Half;
    glm::vec2 m_Desired = {0.0f, 0.0f};

    float m_MinX = 0.0f;
    float m_MaxX = 0.0f;
    float m_Speed = 90.0f;
    float m_Direction = -1.0f;
};

class LevelThreeScene::PipeMobBody final : public IPhysicsBody {
public:
    PipeMobBody(const glm::vec2& pos, const glm::vec2& half,
                float hiddenY, float visibleY, float speed)
        : m_Pos(pos),
          m_Half(glm::abs(half)),
          m_HiddenY(hiddenY),
          m_VisibleY(visibleY),
          m_Speed(std::max(1.0f, speed)) {
        m_Pos.y = m_HiddenY;
    }

    [[nodiscard]] BodyType GetBodyType() const override { return BodyType::PATROL_ENEMY; }
    [[nodiscard]] glm::vec2 GetPosition() const override { return m_Pos; }
    void SetPosition(const glm::vec2& pos) override { m_Pos = pos; }
    [[nodiscard]] glm::vec2 GetHalfSize() const override { return m_Half; }

    [[nodiscard]] bool IsSolid() const override { return true; }
    [[nodiscard]] bool IsKinematic() const override { return true; }

    void PhysicsUpdate() override {
        const float dtSec = Util::Time::GetDeltaTimeMs() / 1000.0f;
        const float maxStep = m_Speed * std::max(0.0f, dtSec);
        m_Desired = {0.0f, 0.0f};

        switch (m_State) {
            case State::HiddenWait:
                m_Timer += dtSec;
                if (m_Timer >= 5.0f) {
                    m_Timer = 0.0f;
                    m_State = State::Rising;
                }
                break;
            case State::Rising: {
                const float dy = std::min(maxStep, m_VisibleY - m_Pos.y);
                m_Desired.y = std::max(0.0f, dy);
                if (m_Pos.y + m_Desired.y >= m_VisibleY - 0.01f) {
                    m_State = State::VisibleWait;
                    m_Timer = 0.0f;
                }
                break;
            }
            case State::VisibleWait:
                m_Timer += dtSec;
                if (m_Timer >= 3.0f) {
                    m_Timer = 0.0f;
                    m_State = State::Descending;
                }
                break;
            case State::Descending: {
                const float dy = std::min(maxStep, m_Pos.y - m_HiddenY);
                m_Desired.y = -std::max(0.0f, dy);
                if (m_Pos.y + m_Desired.y <= m_HiddenY + 0.01f) {
                    m_State = State::HiddenWait;
                    m_Timer = 0.0f;
                }
                break;
            }
        }
    }

    [[nodiscard]] glm::vec2 GetDesiredDelta() const override { return m_Desired; }

    void ApplyResolvedDelta(const glm::vec2& delta) override {
        m_Pos += delta;
    }

private:
    enum class State {
        HiddenWait,
        Rising,
        VisibleWait,
        Descending,
    };

    glm::vec2 m_Pos;
    glm::vec2 m_Half;
    glm::vec2 m_Desired = {0.0f, 0.0f};

    float m_HiddenY = 0.0f;
    float m_VisibleY = 0.0f;
    float m_Speed = 35.0f;

    State m_State = State::HiddenWait;
    float m_Timer = 0.0f;
};

LevelThreeScene::LevelThreeScene(SceneServices services)
    : Scene(services) {
    m_CeilingSprite = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/Ceiling.png");
    m_LeftWallSprite = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/Background_lwall.png");
    m_RightWallSprite = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/Background_rwall.png");

    m_GamePadSprite = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/LevelThreeScene/GamePad.png");
    m_LFloorSprite = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/LevelThreeScene/LFloor.png");
    m_RFloorSprite = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/LevelThreeScene/RFloor.png");
    m_LMidFloorSprite = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/LevelThreeScene/LMidFloor.png");
    m_RMidFloorSprite = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/LevelThreeScene/RMidFloor.png");
    m_LHighFloorSprite = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/LevelThreeScene/LHighFloor.png");
    m_RHighFloorSprite = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/LevelThreeScene/RHighFloor.png");
    m_LLiftSprite = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/LevelThreeScene/LLift.png");
    m_RLiftSprite = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/LevelThreeScene/RLift.png");
    m_MidBlockLeftSprite = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/LevelThreeScene/MidBlock.png");
    m_MidBlockRightSprite = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/LevelThreeScene/MidBlock.png");
    m_PipeSprite = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/LevelThreeScene/Pipe.png");
    m_Mob1Sprite = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/LevelThreeScene/Mob1.png");
    m_Mob2Sprite = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/LevelThreeScene/Mob2.png");
    m_FlagSprite = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/LevelThreeScene/Flag.png");

    m_KeySprite = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/Key.png");
    m_DeadCatSprite = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Character/red_cat/red_cat_die.png");

    m_CheckText = std::make_shared<GameText>("Check", 22, Util::Color::FromRGB(0, 0, 0, 255));
    m_TimerText = std::make_shared<GameText>("TIME 00:00.00", 42, Util::Color::FromRGB(255, 140, 0, 255));

    m_CeilingSprite->SetZIndex(4.0f);
    m_LeftWallSprite->SetZIndex(4.1f);
    m_RightWallSprite->SetZIndex(4.1f);

    m_GamePadSprite->SetZIndex(4.3f);

    m_LFloorSprite->SetZIndex(6.0f);
    m_RFloorSprite->SetZIndex(6.0f);
    m_LMidFloorSprite->SetZIndex(6.0f);
    m_RMidFloorSprite->SetZIndex(6.0f);
    m_LHighFloorSprite->SetZIndex(6.0f);
    m_RHighFloorSprite->SetZIndex(6.0f);

    m_LLiftSprite->SetZIndex(7.0f);
    m_RLiftSprite->SetZIndex(7.0f);

    m_MidBlockLeftSprite->SetZIndex(8.0f);
    m_MidBlockRightSprite->SetZIndex(8.0f);
    m_PipeSprite->SetZIndex(8.0f);

    m_FlagSprite->SetZIndex(16.0f);
    m_CheckText->SetZIndex(16.1f);
    m_CheckText->SetVisible(false);

    // Mob1 should render behind Pipe.
    m_Mob1Sprite->SetZIndex(7.8f);
    m_Mob2Sprite->SetZIndex(19.0f);
    m_KeySprite->SetZIndex(18.0f);

    m_DeadCatSprite->SetZIndex(24.0f);
    m_DeadCatSprite->SetVisible(false);

    m_TimerText->SetZIndex(40.0f);
    m_TimerText->SetPosition({450.0f, 320.0f});
}

bool LevelThreeScene::AabbOverlap(const glm::vec2& aPos, const glm::vec2& aHalf,
                                  const glm::vec2& bPos, const glm::vec2& bHalf) {
    return std::abs(aPos.x - bPos.x) < (aHalf.x + bHalf.x) &&
           std::abs(aPos.y - bPos.y) < (aHalf.y + bHalf.y);
}

void LevelThreeScene::BuildConsensusBindings(int playerCount) {
    m_ConsensusBindings.clear();
    m_ConsensusBindings.reserve(static_cast<size_t>(playerCount));

    const auto& allConfigs = m_Session.GetAppliedKeyConfigs();
    for (int i = 0; i < playerCount; ++i) {
        PlayerKeyConfig cfg = allConfigs[static_cast<size_t>(i)];
        if (i == 0) {
            const auto& d = KeyboardConfigScene::k_Default1P;
            if (cfg.left  == k::UNKNOWN) cfg.left = d.left;
            if (cfg.right == k::UNKNOWN) cfg.right = d.right;
            if (cfg.jump  == k::UNKNOWN) cfg.jump = d.jump;
            if (cfg.up    == k::UNKNOWN) cfg.up = d.up;
        } else if (i == 1) {
            const auto& d = KeyboardConfigScene::k_Default2P;
            if (cfg.left  == k::UNKNOWN) cfg.left = d.left;
            if (cfg.right == k::UNKNOWN) cfg.right = d.right;
            if (cfg.jump  == k::UNKNOWN) cfg.jump = d.jump;
            if (cfg.up    == k::UNKNOWN) cfg.up = d.up;
        }
        m_ConsensusBindings.push_back(cfg);
    }
}

bool LevelThreeScene::IsAllPlayersHolding(Util::Keycode PlayerKeyConfig::* keyMember) const {
    if (m_ConsensusBindings.empty()) return false;

    int participants = 0;

    for (const auto& cfg : m_ConsensusBindings) {
        const Util::Keycode keycode = cfg.*keyMember;
        if (keycode == k::UNKNOWN) {
            continue;
        }
        ++participants;
        if (!ip::IsKeyPressed(keycode)) {
            return false;
        }
    }
    return participants > 0;
}

void LevelThreeScene::HandleConsensusInput() {
    if (m_Player == nullptr || !m_Player->IsActive() || !m_Player->GetInputEnabled()) {
        return;
    }

    const bool allLeft  = IsAllPlayersHolding(&PlayerKeyConfig::left);
    const bool allRight = IsAllPlayersHolding(&PlayerKeyConfig::right);

    int moveDir = 0;
    if (allLeft && !allRight) moveDir = -1;
    else if (allRight && !allLeft) moveDir = 1;

    m_Player->SetMoveDir(moveDir);

    const bool allJumpHeld = IsAllPlayersHolding(&PlayerKeyConfig::jump);
    const bool blockJumpForDoorEntry = IsPlayerInOpenDoorZone();
    if (allJumpHeld && !m_JumpConsensusLatched && m_Player->IsGrounded() && !blockJumpForDoorEntry) {
        m_Player->Jump();
        m_JumpConsensusLatched = true;
    }
    if (!allJumpHeld) {
        m_JumpConsensusLatched = false;
    }
}

bool LevelThreeScene::IsPlayerInOpenDoorZone() const {
    if (!m_DoorOpened || m_PlayerDead || m_Player == nullptr || m_Actors.Door() == nullptr) {
        return false;
    }

    // Slightly expand the doorway interaction area so same-key up/jump can clear reliably.
    glm::vec2 doorHalf = glm::abs(m_Actors.Door()->GetScaledSize()) * 0.5f;
    doorHalf.x += 12.0f;
    doorHalf.y += 8.0f;

    return AabbOverlap(m_Player->GetPosition(), m_Player->GetHalfSize(),
                       m_Actors.Door()->GetPosition(), doorHalf);
}

void LevelThreeScene::SetupSceneVisuals() {
    constexpr float kRoomInnerLeft  = -610.0f;
    constexpr float kRoomInnerRight = 610.0f;
    constexpr float kBottomLayerTopY = kBottomTopY - 33.0f;

    const float ceilHalfH = SpriteHalfH(m_CeilingSprite, 35.0f);
    m_CeilingSprite->SetPosition({0.0f, kRoomTopY + ceilHalfH});
    m_LeftWallSprite->SetPosition({kRoomLeftX, 0.0f});
    m_RightWallSprite->SetPosition({kRoomRightX, 0.0f});

    // Slightly intersects ceiling while mostly below it.
    m_GamePadSprite->SetPosition({0.0f, kRoomTopY - 37.0f});

    // Bottom layer
    const float lFloorHalfW = SpriteHalfW(m_LFloorSprite, 178.0f);
    const float lFloorX = kRoomInnerLeft + lFloorHalfW;
    PlaceSpriteTop(m_LFloorSprite, lFloorX, kBottomLayerTopY);

    const float rFloorHalfW = SpriteHalfW(m_RFloorSprite, 290.0f);
    const float rFloorLeftX = -180.0f;
    const float rFloorX = rFloorLeftX + rFloorHalfW;
    PlaceSpriteTop(m_RFloorSprite, rFloorX, kBottomLayerTopY);

    // Middle layer
    PlaceSpriteTop(m_LMidFloorSprite, -180.0f, kMidTopY);
    PlaceSpriteTop(m_RMidFloorSprite, 300.0f, kMidTopY);

    // High layer
    PlaceSpriteTop(m_LHighFloorSprite, -115.0f, kHighTopY);
    PlaceSpriteTop(m_RHighFloorSprite, 430.0f, kHighTopY);

    // Lifts: left of LHigh, and right of RMid.
    const float lLiftHalfW = SpriteHalfW(m_LLiftSprite, 60.0f);
    PlaceSpriteTop(m_LLiftSprite, kRoomInnerLeft + lLiftHalfW + 5.0f, kMidTopY);

    const float rLiftHalfW = SpriteHalfW(m_RLiftSprite, 59.5f);
    PlaceSpriteTop(m_RLiftSprite, kRoomInnerRight - rLiftHalfW - 70.0f, kBottomLayerTopY);

    // Mid blocks and patrol lane.
    PlaceSpriteTop(m_MidBlockLeftSprite, -250.0f, kMidTopY + 36.0f);
    PlaceSpriteTop(m_MidBlockRightSprite, 40.0f, kMidTopY + 36.0f);

    // Pipe sits on RFloor.
    PlaceSpriteTop(m_PipeSprite, 50.0f, kBottomLayerTopY + SpriteHalfH(m_PipeSprite, 34.0f) * 2.0f - 2.0f);

    const float mob2Y = kMidTopY + SpriteHalfH(m_Mob2Sprite, 15.0f);
    m_Mob2Sprite->SetPosition({-180.0f, mob2Y});

    const float pipeTop = m_PipeSprite->GetPosition().y + SpriteHalfH(m_PipeSprite, 32.0f);
    const float mob1HiddenY = pipeTop - SpriteHalfH(m_Mob1Sprite, 24.0f) * 0.8f;
    m_Mob1Sprite->SetPosition({m_PipeSprite->GetPosition().x, mob1HiddenY - 10.0f});

    m_FlagSprite->SetPosition({300.0f, kMidTopY + SpriteHalfH(m_FlagSprite, 34.0f) - 4.0f});
    m_CheckText->SetPosition(m_FlagSprite->GetPosition() + glm::vec2{0.0f, 42.0f});

    m_KeySprite->SetVisible(true);
    const float keyHalfW = SpriteHalfW(m_KeySprite, 20.0f);
    const float keyHalfH = SpriteHalfH(m_KeySprite, 26.0f);
    const float rHighTop = SpriteTopY(m_RHighFloorSprite, kHighTopY);
    m_KeySprite->SetPosition({kRoomInnerRight - keyHalfW - 48.0f,
                              rHighTop + keyHalfH + 10.0f});

    if (m_Actors.Door() != nullptr) {
        const float doorHalfH = std::abs(m_Actors.Door()->GetScaledSize().y) * 0.5f;
        m_Actors.Door()->SetPosition({
            -325.0f,
            SpriteTopY(m_LHighFloorSprite, kHighTopY) + doorHalfH - 2.0f
        });
    }
}

void LevelThreeScene::SetupStaticBoundaries() {
    constexpr float wallHalfW = 32.0f;
    constexpr float wallHalfH = 360.0f;
    constexpr float ceilHalfH = 35.0f;

    m_World.AddStaticBoundary({-639.0f, 0.0f}, {wallHalfW, wallHalfH}, BodyType::STATIC_BOUNDARY);
    m_World.AddStaticBoundary({639.0f, 0.0f}, {wallHalfW, wallHalfH}, BodyType::STATIC_BOUNDARY);

    const float ceilingSurfaceY = m_CeilingSprite->GetPosition().y - SpriteHalfH(m_CeilingSprite, ceilHalfH);
    m_World.AddStaticBoundary({0.0f, ceilingSurfaceY + ceilHalfH}, {600.0f, ceilHalfH}, BodyType::STATIC_BOUNDARY);

    const auto addSpriteAsPlatform = [&](const std::shared_ptr<Character>& obj) {
        if (obj == nullptr) return;
        const glm::vec2 size = glm::abs(obj->GetScaledSize());
        const glm::vec2 half = {
            std::max(16.0f, size.x * 0.5f),
            std::max(2.0f, size.y * 0.5f)
        };
        m_World.AddStaticBoundary(obj->GetPosition(), half, BodyType::STATIC_BOUNDARY);
    };

    addSpriteAsPlatform(m_LFloorSprite);
    addSpriteAsPlatform(m_RFloorSprite);
    addSpriteAsPlatform(m_LMidFloorSprite);
    addSpriteAsPlatform(m_RMidFloorSprite);
    addSpriteAsPlatform(m_LHighFloorSprite);
    addSpriteAsPlatform(m_RHighFloorSprite);
    addSpriteAsPlatform(m_MidBlockLeftSprite);
    addSpriteAsPlatform(m_MidBlockRightSprite);
    addSpriteAsPlatform(m_PipeSprite);
}

void LevelThreeScene::SetupDynamicBodies() {
    glm::vec2 lLiftHalf = glm::abs(m_LLiftSprite->GetScaledSize()) * 0.5f;
    glm::vec2 rLiftHalf = glm::abs(m_RLiftSprite->GetScaledSize()) * 0.5f;
    lLiftHalf.y = std::max(4.0f, lLiftHalf.y - 2.0f);
    rLiftHalf.y = std::max(4.0f, rLiftHalf.y - 2.0f);

    const float lLiftLowY = SpriteTopY(m_LMidFloorSprite, kMidTopY) - lLiftHalf.y - kLiftEndpointInset;
    const float lLiftHighY = SpriteTopY(m_LHighFloorSprite, kHighTopY) - lLiftHalf.y - kLiftEndpointInset;
    m_LeftLiftBody = std::make_shared<MovingLiftBody>(
        glm::vec2{m_LLiftSprite->GetPosition().x, lLiftLowY}, lLiftHalf,
        lLiftLowY, lLiftHighY, kLiftSpeed);
    m_World.Register(m_LeftLiftBody);

    const float rLiftLowY = SpriteTopY(m_RFloorSprite, kBottomTopY) - rLiftHalf.y - kLiftEndpointInset;
    const float rLiftHighY = SpriteTopY(m_RMidFloorSprite, kMidTopY) - rLiftHalf.y - kLiftEndpointInset;
    m_RightLiftBody = std::make_shared<MovingLiftBody>(
        glm::vec2{m_RLiftSprite->GetPosition().x, rLiftLowY}, rLiftHalf,
        rLiftLowY, rLiftHighY, kLiftSpeed);
    m_World.Register(m_RightLiftBody);

    const float mob2HalfW = SpriteHalfW(m_Mob2Sprite, 26.0f);
    const float mob2HalfH = SpriteHalfH(m_Mob2Sprite, 15.0f);
    const float leftBlockRight = m_MidBlockLeftSprite->GetPosition().x + SpriteHalfW(m_MidBlockLeftSprite, 24.0f);
    const float rightBlockLeft = m_MidBlockRightSprite->GetPosition().x - SpriteHalfW(m_MidBlockRightSprite, 24.0f);
    const float mobMinX = leftBlockRight + mob2HalfW + 2.0f;
    const float mobMaxX = rightBlockLeft - mob2HalfW - 2.0f;

    m_Mob2Body = std::make_shared<PatrolMobBody>(
        glm::vec2{(mobMinX + mobMaxX) * 0.5f, m_Mob2Sprite->GetPosition().y},
        glm::vec2{mob2HalfW, mob2HalfH},
        mobMinX, mobMaxX, kMob2Speed);
    m_World.Register(m_Mob2Body);

    const float mob1HalfW = SpriteHalfW(m_Mob1Sprite, 29.5f);
    const float mob1HalfH = SpriteHalfH(m_Mob1Sprite, 22.5f);
    const float pipeTop = m_PipeSprite->GetPosition().y + SpriteHalfH(m_PipeSprite, 32.0f);
    const float mobHiddenY = pipeTop - mob1HalfH * 0.8f - 10.0f;
    const float mobVisibleY = pipeTop + mob1HalfH - 5.0f;

    m_Mob1Body = std::make_shared<PipeMobBody>(
        glm::vec2{m_PipeSprite->GetPosition().x, mobHiddenY},
        glm::vec2{mob1HalfW, mob1HalfH},
        mobHiddenY, mobVisibleY, kMob1RiseSpeed);
    m_World.Register(m_Mob1Body);

    m_LLiftSprite->SetPosition(m_LeftLiftBody->GetPosition());
    m_RLiftSprite->SetPosition(m_RightLiftBody->GetPosition());
    m_Mob1Sprite->SetPosition(m_Mob1Body->GetPosition());
    m_Mob2Sprite->SetPosition(m_Mob2Body->GetPosition());
}

void LevelThreeScene::LogLayoutSnapshot() const {
    const auto dump = [](const char* name, const std::shared_ptr<Character>& obj) {
        if (obj == nullptr) {
            LOG_INFO("L3_LAYOUT {}: null", name);
            return;
        }
        const auto size = obj->GetScaledSize();
        const auto pos = obj->GetPosition();
        LOG_INFO("L3_LAYOUT {} pos=({}, {}) size=({}, {})", name, pos.x, pos.y, size.x, size.y);
    };

    dump("Ceiling", m_CeilingSprite);
    dump("LWall", m_LeftWallSprite);
    dump("RWall", m_RightWallSprite);
    dump("LFloor", m_LFloorSprite);
    dump("RFloor", m_RFloorSprite);
    dump("LMidFloor", m_LMidFloorSprite);
    dump("RMidFloor", m_RMidFloorSprite);
    dump("LHighFloor", m_LHighFloorSprite);
    dump("RHighFloor", m_RHighFloorSprite);
    dump("LLift", m_LLiftSprite);
    dump("RLift", m_RLiftSprite);
    dump("MidBlockL", m_MidBlockLeftSprite);
    dump("MidBlockR", m_MidBlockRightSprite);
    dump("Pipe", m_PipeSprite);
    dump("Mob1", m_Mob1Sprite);
    dump("Mob2", m_Mob2Sprite);
    dump("Flag", m_FlagSprite);
    dump("Key", m_KeySprite);
    if (m_Player != nullptr) {
        const auto size = m_Player->GetScaledSize();
        const auto pos = m_Player->GetPosition();
        LOG_INFO("L3_LAYOUT Player pos=({}, {}) size=({}, {})", pos.x, pos.y, size.x, size.y);
    }
    if (m_Actors.Door() != nullptr) dump("Door", m_Actors.Door());
}

void LevelThreeScene::OnEnter() {
    const int playerCount = std::clamp(m_Session.GetSelectedPlayerCount(), 2, 8);
    m_Session.SetSelectedPlayerCount(playerCount);
    BuildConsensusBindings(playerCount);

    m_World.Clear();

    m_ElapsedSec = 0.0f;
    m_DeathWaitSec = 0.0f;
    m_ShakeTimerSec = 0.0f;
    m_PlayerDead = false;
    m_CheckpointReached = false;
    m_HasKey = false;
    m_DoorOpened = false;
    m_ClearDone = false;
    m_JumpConsensusLatched = false;
    m_CheckText->SetVisible(false);
    m_CheckText->SetColor(Util::Color::FromRGB(0, 0, 0, 255));

    if (m_Actors.Header()) m_Actors.Header()->SetVisible(false);
    if (m_Actors.Floor()) m_Actors.Floor()->SetVisible(false);
    if (m_Actors.TestBox()) {
        m_Actors.TestBox()->SetVisible(false);
        if (m_Actors.TestBox()->GetTextObject()) {
            m_Actors.TestBox()->GetTextObject()->SetVisible(false);
        }
    }
    for (auto& cat : m_Actors.StartupCats()) {
        if (cat) cat->SetVisible(false);
    }

    if (m_Actors.Door() != nullptr) {
        m_Actors.Door()->SetVisible(true);
        m_Actors.Door()->SetImage(GA_RESOURCE_DIR "/Image/Background/door_close.png");
    }

    SetupSceneVisuals();

    const float lFloorTop = SpriteTopY(m_LFloorSprite, kBottomTopY);
    m_SpawnPoint = {
        m_LFloorSprite->GetPosition().x - SpriteHalfW(m_LFloorSprite, 178.0f) + 120.0f,
        lFloorTop + PlayerCat::kHalfHeight + kSpawnYOffset
    };
    m_RespawnPoint = m_SpawnPoint;
    m_PlayerPrevPos = m_SpawnPoint;

    m_Player = std::make_shared<PlayerCat>(
        CatAssets::BuildFullAnimPaths("red"),
        k::UNKNOWN, k::UNKNOWN, k::UNKNOWN);
    m_Player->SetPosition(m_SpawnPoint);
    m_Player->SetZIndex(30.0f);
    m_Player->SetInputEnabled(true);
    m_Player->SetVisible(true);
    m_Player->SetActive(true);
    m_Player->SetMoveDir(0);

    m_DeadCatSprite->SetVisible(false);
    m_DeadCatSprite->SetPosition(m_SpawnPoint);

    m_Actors.Root().AddChild(m_CeilingSprite);
    m_Actors.Root().AddChild(m_LeftWallSprite);
    m_Actors.Root().AddChild(m_RightWallSprite);
    m_Actors.Root().AddChild(m_GamePadSprite);

    m_Actors.Root().AddChild(m_LFloorSprite);
    m_Actors.Root().AddChild(m_RFloorSprite);
    m_Actors.Root().AddChild(m_LMidFloorSprite);
    m_Actors.Root().AddChild(m_RMidFloorSprite);
    m_Actors.Root().AddChild(m_LHighFloorSprite);
    m_Actors.Root().AddChild(m_RHighFloorSprite);

    m_Actors.Root().AddChild(m_LLiftSprite);
    m_Actors.Root().AddChild(m_RLiftSprite);

    m_Actors.Root().AddChild(m_MidBlockLeftSprite);
    m_Actors.Root().AddChild(m_MidBlockRightSprite);
    m_Actors.Root().AddChild(m_PipeSprite);
    m_Actors.Root().AddChild(m_Mob1Sprite);
    m_Actors.Root().AddChild(m_Mob2Sprite);

    m_Actors.Root().AddChild(m_FlagSprite);
    m_Actors.Root().AddChild(m_CheckText);
    m_Actors.Root().AddChild(m_KeySprite);

    m_Actors.Root().AddChild(m_Player);
    m_Actors.Root().AddChild(m_DeadCatSprite);
    m_Actors.Root().AddChild(m_TimerText);

    UpdateTimerText();

    SetupStaticBoundaries();
    SetupDynamicBodies();
    LogLayoutSnapshot();

    m_World.Register(m_Player);

    LOG_INFO("LevelThreeScene::OnEnter players={} (consensus control)", playerCount);
}

void LevelThreeScene::OnExit() {
    m_World.UnfreezeAll();
    m_World.Clear();

    m_Actors.Root().RemoveChild(m_CeilingSprite);
    m_Actors.Root().RemoveChild(m_LeftWallSprite);
    m_Actors.Root().RemoveChild(m_RightWallSprite);
    m_Actors.Root().RemoveChild(m_GamePadSprite);

    m_Actors.Root().RemoveChild(m_LFloorSprite);
    m_Actors.Root().RemoveChild(m_RFloorSprite);
    m_Actors.Root().RemoveChild(m_LMidFloorSprite);
    m_Actors.Root().RemoveChild(m_RMidFloorSprite);
    m_Actors.Root().RemoveChild(m_LHighFloorSprite);
    m_Actors.Root().RemoveChild(m_RHighFloorSprite);

    m_Actors.Root().RemoveChild(m_LLiftSprite);
    m_Actors.Root().RemoveChild(m_RLiftSprite);

    m_Actors.Root().RemoveChild(m_MidBlockLeftSprite);
    m_Actors.Root().RemoveChild(m_MidBlockRightSprite);
    m_Actors.Root().RemoveChild(m_PipeSprite);
    m_Actors.Root().RemoveChild(m_Mob1Sprite);
    m_Actors.Root().RemoveChild(m_Mob2Sprite);

    m_Actors.Root().RemoveChild(m_FlagSprite);
    m_Actors.Root().RemoveChild(m_CheckText);
    m_Actors.Root().RemoveChild(m_KeySprite);

    if (m_Player != nullptr) {
        m_Actors.Root().RemoveChild(m_Player);
    }
    m_Actors.Root().RemoveChild(m_DeadCatSprite);
    m_Actors.Root().RemoveChild(m_TimerText);

    m_Player.reset();
    m_LeftLiftBody.reset();
    m_RightLiftBody.reset();
    m_Mob1Body.reset();
    m_Mob2Body.reset();

    if (m_Actors.Door() != nullptr) {
        m_Actors.Door()->SetVisible(false);
        m_Actors.Door()->SetImage(GA_RESOURCE_DIR "/Image/Background/door_close.png");
    }

    for (auto& cat : m_Actors.StartupCats()) {
        if (cat) cat->SetVisible(true);
    }

    ResetShake();
}

void LevelThreeScene::PauseGameplay() {
    m_World.FreezeAll();
}

void LevelThreeScene::ResumeGameplay() {
    m_World.UnfreezeAll();
}

void LevelThreeScene::SyncDynamicSprites() const {
    if (m_LeftLiftBody != nullptr) m_LLiftSprite->SetPosition(m_LeftLiftBody->GetPosition());
    if (m_RightLiftBody != nullptr) m_RLiftSprite->SetPosition(m_RightLiftBody->GetPosition());
    if (m_Mob1Body != nullptr) m_Mob1Sprite->SetPosition(m_Mob1Body->GetPosition());
    if (m_Mob2Body != nullptr) {
        m_Mob2Sprite->SetPosition(m_Mob2Body->GetPosition());
        const float absX = std::abs(m_Mob2Sprite->m_Transform.scale.x);
        const float sy = m_Mob2Sprite->m_Transform.scale.y;
        m_Mob2Sprite->SetScale({(m_Mob2Body->GetFacingDir() < 0) ? absX : -absX, sy});
    }
}

void LevelThreeScene::UpdateTimerText() const {
    m_TimerText->SetText("TIME " + SaveManager::FormatTime(m_ElapsedSec));
}

void LevelThreeScene::UpdateCheckpoint() {
    if (m_PlayerDead || m_CheckpointReached || m_Player == nullptr) return;

    const glm::vec2 flagHalf = glm::abs(m_FlagSprite->GetScaledSize()) * 0.5f;
    if (!AabbOverlap(m_Player->GetPosition(), m_Player->GetHalfSize(),
                     m_FlagSprite->GetPosition(), flagHalf)) {
        return;
    }

    m_CheckpointReached = true;
    const float floorTopY = m_RMidFloorSprite->GetPosition().y +
                            SpriteHalfH(m_RMidFloorSprite, 16.0f);
    m_RespawnPoint = {m_FlagSprite->GetPosition().x,
                      floorTopY + PlayerCat::kHalfHeight};
    m_CheckText->SetVisible(true);
    m_CheckText->SetColor(Util::Color::FromRGB(30, 150, 40, 255));
}

void LevelThreeScene::TryPickKey() {
    if (m_PlayerDead || m_HasKey || m_Player == nullptr) return;

    const glm::vec2 keyHalf = glm::abs(m_KeySprite->GetScaledSize()) * 0.5f;
    if (!AabbOverlap(m_Player->GetPosition(), m_Player->GetHalfSize(),
                     m_KeySprite->GetPosition(), keyHalf)) {
        return;
    }

    m_HasKey = true;
}

void LevelThreeScene::UpdateKeyFollow() const {
    if (!m_HasKey || m_Player == nullptr) return;
    m_KeySprite->SetPosition(m_Player->GetPosition() + glm::vec2{-26.0f, 28.0f});
}

void LevelThreeScene::TryOpenDoor() {
    if (!m_HasKey || m_DoorOpened || m_Player == nullptr || m_Actors.Door() == nullptr) return;

    const glm::vec2 doorHalf = glm::abs(m_Actors.Door()->GetScaledSize()) * 0.5f;
    if (!AabbOverlap(m_Player->GetPosition(), m_Player->GetHalfSize(),
                     m_Actors.Door()->GetPosition(), doorHalf)) {
        return;
    }

    m_DoorOpened = true;
    m_Actors.Door()->SetImage(GA_RESOURCE_DIR "/Image/Background/door_open.png");
    m_KeySprite->SetVisible(false);
}

void LevelThreeScene::TryClearLevel() {
    if (m_ClearDone || !m_DoorOpened || m_PlayerDead || m_Player == nullptr || m_Actors.Door() == nullptr) {
        return;
    }

    const glm::vec2 doorHalf = glm::abs(m_Actors.Door()->GetScaledSize()) * 0.5f;
    if (!AabbOverlap(m_Player->GetPosition(), m_Player->GetHalfSize(),
                     m_Actors.Door()->GetPosition(), doorHalf)) {
        return;
    }

    bool allUpDown = false;
    int upParticipants = 0;
    for (const auto& cfg : m_ConsensusBindings) {
        if (cfg.up == k::UNKNOWN) {
            continue;
        }
        ++upParticipants;
        if (!ip::IsKeyPressed(cfg.up)) {
            allUpDown = false;
            break;
        }
        allUpDown = true;
    }

    if (upParticipants == 0) allUpDown = false;

    if (!allUpDown) return;

    SaveManager::UpdateBestTime(kLevelIndex, m_Session.GetSelectedPlayerCount(), m_ElapsedSec);
    m_ClearDone = true;
}

bool LevelThreeScene::IsStompCollision(const IPhysicsBody& mobBody) const {
    if (m_Player == nullptr) return false;

    const glm::vec2 playerPos = m_Player->GetPosition();
    const glm::vec2 playerHalf = m_Player->GetHalfSize();
    const glm::vec2 mobPos = mobBody.GetPosition();
    const glm::vec2 mobHalf = mobBody.GetHalfSize();

    const bool descending = m_PlayerPrevPos.y > playerPos.y + 0.1f;
    const float playerBottom = playerPos.y - playerHalf.y;
    const float mobTop = mobPos.y + mobHalf.y;
    const bool fromAbove = playerBottom >= mobTop - 8.0f;

    return descending && fromAbove;
}

void LevelThreeScene::TriggerStompBounce(const IPhysicsBody& mobBody) const {
    if (m_Player == nullptr) return;

    const glm::vec2 mobPos = mobBody.GetPosition();
    const glm::vec2 mobHalf = mobBody.GetHalfSize();
    glm::vec2 pos = m_Player->GetPosition();

    pos.y = mobPos.y + mobHalf.y + m_Player->GetHalfSize().y + 2.0f;
    m_Player->SetPosition(pos);
    m_Player->Jump();
}

void LevelThreeScene::StartDeath() {
    if (m_Player == nullptr || m_PlayerDead) return;

    m_PlayerDead = true;
    m_DeathWaitSec = kDeathRespawnDelaySec;

    m_DeadCatSprite->SetPosition(m_Player->GetPosition());
    m_DeadCatSprite->SetVisible(true);

    m_Player->SetVisible(false);
    m_Player->SetActive(false);
    m_Player->SetInputEnabled(false);
    m_Player->SetMoveDir(0);

    m_ShakeTimerSec = kShakeDurationSec;
    m_ShakeSeed += 0.137f;
}

void LevelThreeScene::RespawnPlayer() {
    if (m_Player == nullptr) return;

    m_PlayerDead = false;
    m_DeathWaitSec = 0.0f;

    m_Player->SetPosition(m_RespawnPoint);
    m_Player->SetVisible(true);
    m_Player->SetActive(true);
    m_Player->SetInputEnabled(true);
    m_Player->SetMoveDir(0);
    m_Player->SetCatAnimState(CatAnimState::STAND);
    m_PlayerPrevPos = m_RespawnPoint;
    m_JumpConsensusLatched = false;

    m_DeadCatSprite->SetVisible(false);
}

void LevelThreeScene::HandleHazardsAndRespawn() {
    if (m_Player == nullptr) return;

    if (m_PlayerDead) {
        m_DeathWaitSec -= Util::Time::GetDeltaTimeMs() / 1000.0f;
        if (m_DeathWaitSec <= 0.0f) {
            RespawnPlayer();
        }
        return;
    }

    // Falling below the play area counts as death in this level.
    if (m_Player->GetPosition().y < (kRoomFloorY - kFallDeathYOffset)) {
        StartDeath();
        return;
    }

    const auto checkMob = [&](const std::shared_ptr<IPhysicsBody>& mob) -> bool {
        if (mob == nullptr || !mob->IsActive()) return false;
        const glm::vec2 playerPos = m_Player->GetPosition();
        const glm::vec2 playerHalf = m_Player->GetHalfSize();
        const glm::vec2 mobPos = mob->GetPosition();
        const glm::vec2 mobHalf = mob->GetHalfSize();

        const bool overlapNow = AabbOverlap(playerPos, playerHalf, mobPos, mobHalf);
        const bool nearContact =
            std::abs(playerPos.x - mobPos.x) <= (playerHalf.x + mobHalf.x + kHazardContactSlack) &&
            std::abs(playerPos.y - mobPos.y) <= (playerHalf.y + mobHalf.y + kHazardContactSlack);

        if (!(overlapNow || nearContact)) {
            return false;
        }

        if (IsStompCollision(*mob)) {
            TriggerStompBounce(*mob);
            return true;
        }

        StartDeath();
        return true;
    };

    if (checkMob(m_Mob1Body)) return;
    checkMob(m_Mob2Body);
}

void LevelThreeScene::UpdateShake() {
    if (m_ShakeTimerSec <= 0.0f) {
        ResetShake();
        return;
    }

    const float dtSec = Util::Time::GetDeltaTimeMs() / 1000.0f;
    m_ShakeTimerSec = std::max(0.0f, m_ShakeTimerSec - dtSec);

    // Screen-shake hook disabled by request; keep timer decay for future reuse.
    (void)kShakeAmplitudePx;
    (void)m_ShakeSeed;
}

void LevelThreeScene::ResetShake() const {
    // Screen-shake hook disabled by request.
}

void LevelThreeScene::Update() {
    if (ip::IsKeyDown(k::ESCAPE)) {
        RequestSceneOp({SceneOpType::PushOverlay, SceneId::LevelExit});
        return;
    }

    if (m_Player != nullptr) {
        m_PlayerPrevPos = m_Player->GetPosition();
    }

    if (!m_PlayerDead) {
        HandleConsensusInput();
        // Safety: keep the controllable cat visible while alive.
        if (m_Player != nullptr) {
            m_Player->SetVisible(true);
            m_Player->SetActive(true);
        }
    }

    m_World.Update();
    SyncDynamicSprites();

    UpdateCheckpoint();
    TryPickKey();
    UpdateKeyFollow();
    TryOpenDoor();
    TryClearLevel();
    HandleHazardsAndRespawn();

    m_ElapsedSec += Util::Time::GetDeltaTimeMs() / 1000.0f;
    UpdateTimerText();
    UpdateShake();

    if (m_ClearDone) {
        RequestSceneOp({SceneOpType::ClearToAndGoTo, SceneId::LevelSelect});
    }
}


