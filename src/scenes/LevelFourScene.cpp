#include "scenes/LevelFourScene.hpp"

#include <algorithm>
#include <cmath>

#include "app/AppUtil.hpp"
#include "gameplay/CatAssets.hpp"
#include "physics/BulletBody.hpp"
#include "scenes/KeyboardConfigScene.hpp"
#include "services/SaveManager.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"

using ip = Util::Input;
using k = Util::Keycode;

LevelFourScene::LevelFourScene(SceneServices services)
    : Scene(services) {
    constexpr const char* base = GA_RESOURCE_DIR "/Image/Level_Cover/LevelFourScene/";

    m_CeilingSprite = std::make_shared<Character>(std::string(base) + "Ceiling.png");
    m_FloorSprite = std::make_shared<Character>(std::string(base) + "Floor.png");
    m_LHighWallSprite = std::make_shared<Character>(std::string(base) + "LHighWall.png");
    m_LMidWallSprite = std::make_shared<Character>(std::string(base) + "LMidWall.png");
    m_RHighWallSprite = std::make_shared<Character>(std::string(base) + "RHighWall.png");
    m_RMidWallSprite = std::make_shared<Character>(std::string(base) + "RMidWall.png");
    m_JarSprite = std::make_shared<Character>(std::string(base) + "Jar0.png");
    m_ShooterSprite = std::make_shared<Character>(std::string(base) + "Shooter.png");

    m_KeySprite = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/Key.png");
    m_BulletSprite = std::make_shared<Character>(std::string(base) + "Bullet.png");
    m_TimerText = std::make_shared<GameText>("TIME 00:00.00", 42, Util::Color::FromRGB(255, 140, 0, 255));

    m_CeilingSprite->SetZIndex(4.0f);
    m_FloorSprite->SetZIndex(4.0f);
    m_LHighWallSprite->SetZIndex(4.1f);
    m_LMidWallSprite->SetZIndex(4.1f);
    m_RHighWallSprite->SetZIndex(4.1f);
    m_RMidWallSprite->SetZIndex(4.1f);
    m_ShooterSprite->SetZIndex(16.0f);
    m_KeySprite->SetZIndex(18.0f);
    m_JarSprite->SetZIndex(17.0f);
    m_BulletSprite->SetZIndex(19.0f);
    m_TimerText->SetZIndex(40.0f);
    m_TimerText->SetPosition({350.0f, 310.0f});
    m_BulletSprite->SetVisible(false);
}

bool LevelFourScene::AabbOverlap(const glm::vec2& aPos, const glm::vec2& aHalf,
                                 const glm::vec2& bPos, const glm::vec2& bHalf) {
    return std::abs(aPos.x - bPos.x) < (aHalf.x + bHalf.x) &&
           std::abs(aPos.y - bPos.y) < (aHalf.y + bHalf.y);
}

float LevelFourScene::FloorTopY() const {
    return AppUtil::TopEdge(*m_FloorSprite);
}

void LevelFourScene::SetupSceneVisuals() {
    m_CeilingSprite->SetPosition({
        0.0f,
        AppUtil::AlignSpriteTop(*m_CeilingSprite, kViewTopY)+2
    });

    m_FloorSprite->SetPosition({
        0.0f,
        AppUtil::AlignSpriteBottom(*m_FloorSprite, kViewBottomY)-1
    });

    m_LHighWallSprite->SetPosition({
        AppUtil::AlignSpriteLeft(*m_LHighWallSprite, kViewLeftX)-1,
        AppUtil::AlignSpriteBelow(*m_LHighWallSprite, *m_CeilingSprite)+3
    });

    m_LMidWallSprite->SetPosition({
        AppUtil::AlignSpriteLeft(*m_LMidWallSprite, kViewLeftX)-1,
        AppUtil::AlignSpriteBelow(*m_LMidWallSprite, *m_LHighWallSprite)+5
    });

    m_RHighWallSprite->SetPosition({
        AppUtil::AlignSpriteRight(*m_RHighWallSprite, kViewRightX)+1,
        AppUtil::AlignSpriteBelow(*m_RHighWallSprite, *m_CeilingSprite)+3
    });

    m_RMidWallSprite->SetPosition({
        AppUtil::AlignSpriteRight(*m_RMidWallSprite, kViewRightX)+1,
        AppUtil::AlignSpriteBelow(*m_RMidWallSprite, *m_RHighWallSprite)+5
    });

    m_JarSprite->SetImage(GA_RESOURCE_DIR "/Image/Level_Cover/LevelFourScene/Jar0.png");
    m_JarSprite->SetVisible(true);
    m_JarSprite->SetPosition({
        AppUtil::RightEdge(*m_LMidWallSprite) + AppUtil::SpriteHalfW(*m_JarSprite),
        FloorTopY() + AppUtil::SpriteHalfH(*m_JarSprite)
    });

    m_ShooterSprite->SetPosition({
        AppUtil::LeftEdge(*m_RMidWallSprite) - AppUtil::SpriteHalfW(*m_ShooterSprite),
        FloorTopY() + AppUtil::SpriteHalfH(*m_ShooterSprite)
    });

    m_KeySprite->SetVisible(true);
    m_KeySprite->SetPosition(m_JarSprite->GetPosition());

    if (m_Actors.Door() != nullptr) {
        const float doorHalfH = std::abs(m_Actors.Door()->GetScaledSize().y) * 0.5f;
        m_Actors.Door()->SetVisible(true);
        m_Actors.Door()->SetImage(GA_RESOURCE_DIR "/Image/Background/door_close.png");
        m_Actors.Door()->SetPosition({259.0f, FloorTopY() + doorHalfH});
    }
}

void LevelFourScene::SetupStaticBoundaries() {
    const auto addSpriteBoundary = [&](const std::shared_ptr<Character>& sprite) {
        if (sprite == nullptr) return;
        const glm::vec2 half = glm::abs(sprite->GetScaledSize()) * 0.5f;
        m_World.AddStaticBoundary(sprite->GetPosition(), half, BodyType::STATIC_BOUNDARY);
    };

    addSpriteBoundary(m_CeilingSprite);
    addSpriteBoundary(m_FloorSprite);
    addSpriteBoundary(m_LHighWallSprite);
    addSpriteBoundary(m_LMidWallSprite);
    addSpriteBoundary(m_RHighWallSprite);
    addSpriteBoundary(m_RMidWallSprite);

    if (m_JarSprite != nullptr) {
        m_JarBody = m_World.AddStaticBoundary(
            m_JarSprite->GetPosition(),
            glm::abs(m_JarSprite->GetScaledSize()) * 0.5f,
            BodyType::STATIC_BOUNDARY);
    }
    if (m_ShooterSprite != nullptr) {
        m_ShooterBody = m_World.AddStaticBoundary(
            m_ShooterSprite->GetPosition(),
            glm::abs(m_ShooterSprite->GetScaledSize()) * 0.5f,
            BodyType::STATIC_BOUNDARY);
    }
}

void LevelFourScene::SpawnPlayers(int count) {
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

void LevelFourScene::ApplyInitialFormation() const {
    const float spawnY = FloorTopY() + PlayerCat::kHalfHeight;
    for (int i = 0; i < static_cast<int>(m_Players.size()); ++i) {
        const auto& pb = m_Players[i];
        if (pb.cat == nullptr) continue;
        const float x = -500.0f + static_cast<float>(i) * 38.0f;
        pb.cat->SetPosition({x, spawnY});
    }
}

void LevelFourScene::HandlePlayerInput() const {
    for (const auto& pb : m_Players) {
        if (pb.cat == nullptr || !pb.cat->IsActive() || !pb.cat->GetInputEnabled()) continue;

        int moveDir = 0;
        const bool goLeft = (pb.key.left != k::UNKNOWN) && ip::IsKeyPressed(pb.key.left);
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

void LevelFourScene::UpdateTimerText() const {
    m_TimerText->SetText("TIME " + SaveManager::FormatTime(m_ElapsedSec));
}

float LevelFourScene::CurrentBulletSpeed() const {
    switch (m_JarState) {
        case JarState::Jar0: return kBulletBaseSpeed;
        case JarState::Jar1: return kBulletBaseSpeed * 1.7f;
        case JarState::Jar2: return kBulletBaseSpeed * 1.7f * 2.0f;
        case JarState::Gone: return 0.0f;
    }
    return kBulletBaseSpeed;
}

void LevelFourScene::SpawnBullet() {
    if (m_BulletBody == nullptr || m_JarState == JarState::Gone) return;

    const float spawnX = AppUtil::LeftEdge(*m_ShooterSprite) - kBulletHalf.x - 2.0f;
    const float spawnY = m_JarSprite->GetPosition().y;

    m_BulletBody->SetPosition({spawnX, spawnY});
    m_BulletBody->SetSpeed(CurrentBulletSpeed());
    m_BulletBody->SetActive(true);
    m_BulletActive = true;

    m_BulletSprite->SetPosition(m_BulletBody->GetPosition());
    m_BulletSprite->SetVisible(true);
}

void LevelFourScene::DeactivateBullet() {
    if (m_BulletBody != nullptr) m_BulletBody->SetActive(false);
    if (m_BulletSprite != nullptr) m_BulletSprite->SetVisible(false);
    m_BulletActive = false;
    m_BulletCooldownSec = kBulletRespawnDelaySec;
}

void LevelFourScene::SyncBulletSprite() const {
    if (m_BulletBody == nullptr || m_BulletSprite == nullptr) return;
    if (!m_BulletActive || !m_BulletBody->IsActive()) return;
    m_BulletSprite->SetPosition(m_BulletBody->GetPosition());
}

void LevelFourScene::UpdateBulletSpawner(float dtSec) {
    if (m_JarState == JarState::Gone) {
        if (m_BulletActive) DeactivateBullet();
        return;
    }
    if (m_BulletActive) return;

    m_BulletCooldownSec -= dtSec;
    if (m_BulletCooldownSec <= 0.0f) {
        SpawnBullet();
    }
}

void LevelFourScene::HandleBulletHits() {
    if (!m_BulletActive || m_BulletBody == nullptr || !m_BulletBody->IsActive()) return;

    const glm::vec2 bulletPos = m_BulletBody->GetPosition();

    for (const auto& pb : m_Players) {
        if (pb.cat == nullptr || !pb.cat->IsActive()) continue;
        if (AabbOverlap(bulletPos, kBulletHalf, pb.cat->GetPosition(), pb.cat->GetHalfSize())) {
            DeactivateBullet();
            return;
        }
    }

    if (m_JarState != JarState::Gone) {
        const glm::vec2 jarHalf = glm::abs(m_JarSprite->GetScaledSize()) * 0.5f;
        if (AabbOverlap(bulletPos, kBulletHalf, m_JarSprite->GetPosition(), jarHalf)) {
            AdvanceJarState();
            DeactivateBullet();
            return;
        }
    }

    const auto bulletHitSprite = [&](const std::shared_ptr<Character>& sprite) {
        if (sprite == nullptr) return false;
        const glm::vec2 half = glm::abs(sprite->GetScaledSize()) * 0.5f;
        return AabbOverlap(bulletPos, kBulletHalf, sprite->GetPosition(), half);
    };

    if (bulletHitSprite(m_CeilingSprite) ||
        bulletHitSprite(m_FloorSprite) ||
        bulletHitSprite(m_LHighWallSprite) ||
        bulletHitSprite(m_LMidWallSprite) ||
        bulletHitSprite(m_RHighWallSprite) ||
        bulletHitSprite(m_RMidWallSprite)) {
        DeactivateBullet();
        return;
    }

    if (bulletPos.x + kBulletHalf.x < kViewLeftX) {
        DeactivateBullet();
    }
}

void LevelFourScene::AdvanceJarState() {
    switch (m_JarState) {
        case JarState::Jar0:
            m_JarState = JarState::Jar1;
            m_JarSprite->SetImage(GA_RESOURCE_DIR "/Image/Level_Cover/LevelFourScene/Jar1.png");
            break;
        case JarState::Jar1:
            m_JarState = JarState::Jar2;
            m_JarSprite->SetImage(GA_RESOURCE_DIR "/Image/Level_Cover/LevelFourScene/Jar2.png");
            break;
        case JarState::Jar2:
            m_JarState = JarState::Gone;
            m_JarSprite->SetVisible(false);
            if (m_JarBody != nullptr) {
                m_JarBody->SetActive(false);
            }
            break;
        case JarState::Gone:
            break;
    }
}

void LevelFourScene::TryPickKey() {
    if (m_JarState != JarState::Gone) return;
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

void LevelFourScene::UpdateKeyFollow() const {
    if (m_KeyCarrierIdx < 0 || m_KeyCarrierIdx >= static_cast<int>(m_Players.size())) return;

    const auto& carrier = m_Players[m_KeyCarrierIdx].cat;
    if (carrier == nullptr || !carrier->IsActive()) return;

    m_KeySprite->SetPosition(carrier->GetPosition() + glm::vec2{-28.0f, 28.0f});
}

void LevelFourScene::TryOpenDoorAndClear() {
    if (m_KeyCarrierIdx < 0 || m_KeyCarrierIdx >= static_cast<int>(m_Players.size())) return;
    if (m_Actors.Door() == nullptr || m_DoorOpened) return;

    const auto& carrier = m_Players[m_KeyCarrierIdx].cat;
    if (carrier == nullptr || !carrier->IsActive()) return;

    const glm::vec2 doorPos = m_Actors.Door()->GetPosition();
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

void LevelFourScene::UpdateDoorEntryAndClear() {
    if (!m_DoorOpened || m_Actors.Door() == nullptr) return;

    const glm::vec2 doorPos = m_Actors.Door()->GetPosition();
    const glm::vec2 doorHalf = glm::abs(m_Actors.Door()->GetScaledSize()) * 0.5f;
    const int totalPlayers = static_cast<int>(m_Players.size());

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

void LevelFourScene::OnEnter() {
    const int playerCount = std::clamp(m_Session.GetSelectedPlayerCount(), 2, 8);
    m_Session.SetSelectedPlayerCount(playerCount);

    m_PlayerEntered.assign(static_cast<size_t>(playerCount), false);
    m_EnteredCount = 0;
    m_ClearDone = false;
    m_DoorOpened = false;
    m_KeyCarrierIdx = -1;
    m_ElapsedSec = 0.0f;
    m_JarState = JarState::Jar0;
    m_BulletActive = false;
    m_BulletCooldownSec = kInitialBulletDelaySec;
    m_BulletSprite->SetVisible(false);

    m_World.Clear();
    m_JarBody = nullptr;
    m_ShooterBody = nullptr;

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

    SetupSceneVisuals();

    m_Actors.Root().AddChild(m_CeilingSprite);
    m_Actors.Root().AddChild(m_FloorSprite);
    m_Actors.Root().AddChild(m_LHighWallSprite);
    m_Actors.Root().AddChild(m_LMidWallSprite);
    m_Actors.Root().AddChild(m_RHighWallSprite);
    m_Actors.Root().AddChild(m_RMidWallSprite);
    m_Actors.Root().AddChild(m_ShooterSprite);
    m_Actors.Root().AddChild(m_KeySprite);
    m_Actors.Root().AddChild(m_JarSprite);
    m_Actors.Root().AddChild(m_BulletSprite);
    m_Actors.Root().AddChild(m_TimerText);

    UpdateTimerText();
    SpawnPlayers(playerCount);
    SetupStaticBoundaries();

    m_BulletBody = std::make_shared<BulletBody>(m_ShooterSprite->GetPosition(), kBulletHalf);
    m_World.Register(m_BulletBody);

    LOG_INFO("LevelFourScene::OnEnter players={}", playerCount);
}

void LevelFourScene::OnExit() {
    for (auto& pb : m_Players) {
        if (pb.cat) m_Actors.Root().RemoveChild(pb.cat);
    }
    m_Players.clear();

    m_World.UnfreezeAll();
    m_World.Clear();

    m_Actors.Root().RemoveChild(m_CeilingSprite);
    m_Actors.Root().RemoveChild(m_FloorSprite);
    m_Actors.Root().RemoveChild(m_LHighWallSprite);
    m_Actors.Root().RemoveChild(m_LMidWallSprite);
    m_Actors.Root().RemoveChild(m_RHighWallSprite);
    m_Actors.Root().RemoveChild(m_RMidWallSprite);
    m_Actors.Root().RemoveChild(m_ShooterSprite);
    m_Actors.Root().RemoveChild(m_KeySprite);
    m_Actors.Root().RemoveChild(m_JarSprite);
    m_Actors.Root().RemoveChild(m_BulletSprite);
    m_Actors.Root().RemoveChild(m_TimerText);

    m_BulletBody.reset();
    m_JarBody = nullptr;
    m_ShooterBody = nullptr;

    if (m_Actors.Door()) {
        m_Actors.Door()->SetVisible(false);
        m_Actors.Door()->SetImage(GA_RESOURCE_DIR "/Image/Background/door_close.png");
    }

    if (m_Actors.Header()) m_Actors.Header()->SetVisible(false);
    if (m_Actors.Floor()) m_Actors.Floor()->SetVisible(false);
    if (m_Actors.TestBox()) {
        m_Actors.TestBox()->SetVisible(false);
        if (m_Actors.TestBox()->GetTextObject()) {
            m_Actors.TestBox()->GetTextObject()->SetVisible(false);
        }
    }
    for (auto& cat : m_Actors.StartupCats()) {
        if (cat) cat->SetVisible(true);
    }
}

void LevelFourScene::PauseGameplay() {
    m_World.FreezeAll();
}

void LevelFourScene::ResumeGameplay() {
    m_World.UnfreezeAll();
}

void LevelFourScene::Update() {
    if (ip::IsKeyDown(k::ESCAPE)) {
        RequestSceneOp({SceneOpType::PushOverlay, SceneId::LevelExit});
        return;
    }

    const float dtSec = Util::Time::GetDeltaTimeMs() / 1000.0f;

    HandlePlayerInput();
    m_World.Update();
    SyncBulletSprite();

    UpdateBulletSpawner(dtSec);
    HandleBulletHits();

    m_ElapsedSec += dtSec;
    UpdateTimerText();

    TryPickKey();
    UpdateKeyFollow();
    TryOpenDoorAndClear();
    UpdateDoorEntryAndClear();

    if (m_ClearDone) {
        RequestSceneOp({SceneOpType::ClearToAndGoTo, SceneId::LevelSelect});
    }
}
