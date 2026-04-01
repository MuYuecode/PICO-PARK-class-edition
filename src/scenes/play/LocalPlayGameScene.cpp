#include "scenes/play/LocalPlayGameScene.hpp"
#include "gameplay/factories/BoundaryFactory.hpp"
#include "scenes/play/LocalPlayScene.hpp"
#include "scenes/menu/KeyboardConfigScene.hpp"
#include "gameplay/assets/CatAssets.hpp"
#include "Util/Input.hpp"
#include "Util/Logger.hpp"
#include <algorithm>
#include <cmath>

using ip = Util::Input;
using k  = Util::Keycode;

LocalPlayGameScene::LocalPlayGameScene(SceneServices services)
    : Scene(services) {}

void LocalPlayGameScene::SetupStaticBoundaries() {
    float floorSurfaceY   = -360.f;
    float ceilingSurfaceY = 360.f;

    if (m_Actors.Floor() != nullptr) {
        floorSurfaceY = m_Actors.Floor()->GetPosition().y
                      + m_Actors.Floor()->GetScaledSize().y * 0.5f;
    }

    BoundaryFactory::AddStaticRoomBoundaries(
        m_World,
        floorSurfaceY,
        ceilingSurfaceY,
        LevelGeometryPreset::kSharedMenuLikeWithCeiling);
}

void LocalPlayGameScene::OnEnter() {
    int playerCount = std::clamp(
        m_Session.GetSelectedPlayerCount(),
        LocalPlayScene::MIN_PLAYERS,
        LocalPlayScene::MAX_PLAYERS);

    m_Session.SetSelectedPlayerCount(playerCount);
    m_Session.SetCooperativePushPower(1);

    for (auto& cat : m_Actors.StartupCats()) {
        if (cat != nullptr) {
            cat->SetVisible(false);
            cat->SetInputEnabled(false);
        }
    }

    m_World.Clear();
    SpawnPlayers(playerCount);
    SetupStaticBoundaries();
    {
        const float floorY     = m_Actors.Floor()->GetPosition().y;
        const float floorHalfH = m_Actors.Floor()->GetScaledSize().y / 2.0f;
        const float doorHalfH  = m_Actors.Door()->GetScaledSize().y / 2.0f;
        m_Actors.Door()->SetPosition({0.0f, floorY + floorHalfH + doorHalfH});
    }
    m_Actors.Door()->SetImage(GA_RESOURCE_DIR "/Image/Background/door_open.png");

    m_EnteredCount = 0;
    const glm::vec2 doorPos   = m_Actors.Door()->GetPosition();
    const float     doorHalfH = m_Actors.Door()->GetScaledSize().y / 2.f;

    m_DoorCountText = std::make_shared<GameText>(
        "0/" + std::to_string(playerCount),
        40,
        Util::Color::FromRGB(0, 0, 0, 255));
    m_DoorCountText->SetZIndex(30.f);
    m_DoorCountText->SetPosition({doorPos.x + 10.f, doorPos.y + doorHalfH + 25.f});
    m_Actors.Root().AddChild(m_DoorCountText);

    LOG_INFO("LocalPlayGameScene::OnEnter players={}", playerCount);
}

void LocalPlayGameScene::OnExit() {
    for (auto& pb : m_Players) {
        if (pb.cat != nullptr) m_Actors.Root().RemoveChild(pb.cat);
    }

    m_World.Clear();
    m_Players.clear();

    m_Actors.Door()->SetImage(GA_RESOURCE_DIR "/Image/Background/door_close.png");

    if (m_DoorCountText != nullptr) {
        m_Actors.Root().RemoveChild(m_DoorCountText);
        m_DoorCountText = nullptr;
    }

    for (auto& cat : m_Actors.StartupCats()) {
        if (cat != nullptr) cat->SetVisible(true);
    }

    LOG_INFO("LocalPlayGameScene::OnExit");
}

void LocalPlayGameScene::Update() {
    if (ip::IsKeyDown(k::ESCAPE)) {
        RequestSceneOp({SceneOpType::ClearToAndGoTo, SceneId::LocalPlay});
        return;
    }

    for (auto& pb : m_Players) {
        if (pb.entered || pb.cat == nullptr) continue;

        const auto& key = pb.key;
        int moveDir = 0;

        const bool goLeft  = (key.left  != k::UNKNOWN) && ip::IsKeyPressed(key.left);
        const bool goRight = (key.right != k::UNKNOWN) && ip::IsKeyPressed(key.right);

        if      (goLeft  && !goRight) moveDir = -1;
        else if (goRight && !goLeft)  moveDir =  1;

        pb.cat->SetMoveDir(moveDir);

        const bool wantJump = (key.jump != k::UNKNOWN) && ip::IsKeyDown(key.jump);
        if (pb.cat->IsGrounded() && wantJump) {
            pb.cat->Jump();
        }
    }

    const glm::vec2 doorPos  = m_Actors.Door()->GetPosition();
    const glm::vec2 doorSize = m_Actors.Door()->GetScaledSize();
    const float halfW = doorSize.x / 2.f + 10.f;
    const float halfH = doorSize.y / 2.f + 10.f;

    for (auto& pb : m_Players) {
        if (pb.entered || pb.cat == nullptr) continue;

        const glm::vec2 pos     = pb.cat->GetPosition();
        const bool      inRange = (std::abs(pos.x - doorPos.x) <= halfW &&
                                   std::abs(pos.y - doorPos.y) <= halfH);
        const bool pressedUp    = (pb.key.up != k::UNKNOWN) &&
                                  ip::IsKeyDown(pb.key.up);

        if (inRange && pressedUp) {
            pb.entered = true;
            ++m_EnteredCount;
            pb.cat->SetVisible(false);
            pb.cat->SetInputEnabled(false);
            pb.cat->SetActive(false);
            pb.cat->SetPosition({640.f, -360.f});
            UpdateDoorCountText();

            if (m_EnteredCount == m_Session.GetSelectedPlayerCount()) {
                LOG_INFO("LocalPlayGameScene: all players entered -> LevelSelectScene");
                RequestSceneOp({SceneOpType::ClearToAndGoTo, SceneId::LevelSelect});
                return;
            }
        }
    }

    m_World.Update();
    UpdateCooperativePower();
}

void LocalPlayGameScene::SpawnPlayers(int count) {
    m_Players.clear();
    m_Players.reserve(static_cast<size_t>(count));

    for (int i = 0; i < count; ++i) {
        const std::string color =
            m_Actors.CatColorOrder()[static_cast<size_t>(i)];

        auto cat = std::make_shared<PlayerCat>(
            CatAssets::BuildFullAnimPaths(color),
            k::UNKNOWN, k::UNKNOWN, k::UNKNOWN);
        cat->SetZIndex(20.f + static_cast<float>(i) * 0.01f);

        PlayerBinding pb;
        pb.cat = cat;
        pb.key = m_Session.GetAppliedKeyConfigs()[static_cast<size_t>(i)];

        if (i == 0 && pb.key.left == k::UNKNOWN && pb.key.right == k::UNKNOWN) {
            pb.key = KeyboardConfigScene::k_Default1P;
        } else if (i == 1 && pb.key.left == k::UNKNOWN && pb.key.right == k::UNKNOWN) {
            pb.key = KeyboardConfigScene::k_Default2P;
        }

        m_Players.push_back(pb);
        m_Actors.Root().AddChild(cat);
    }

    ApplyInitialFormation();

    for (auto& pb : m_Players) {
        m_World.Register(pb.cat);
    }
}

void LocalPlayGameScene::ApplyInitialFormation() {
    const float floorY     = (m_Actors.Floor() != nullptr)
                                 ? m_Actors.Floor()->GetPosition().y : -340.f;
    const float floorHalfH = (m_Actors.Floor() != nullptr)
                                 ? m_Actors.Floor()->GetScaledSize().y / 2.f : 0.f;

    const float halfW0  = m_Players.empty() || m_Players[0].cat == nullptr
                              ? 32.f
                              : std::max(16.f, std::abs(
                                    m_Players[0].cat->GetScaledSize().x) * 0.5f);
    const float spacing     = halfW0 * 1.7f;
    const float centerBlank = spacing * 2.f;

    for (int i = 0; i < static_cast<int>(m_Players.size()); ++i) {
        auto& pb = m_Players[i];
        if (pb.cat == nullptr) continue;

        const float charHalfH = pb.cat->GetScaledSize().y / 2.f;
        const float spawnY    = floorY + floorHalfH + charHalfH;

        const bool  isLeft    = (i % 2 == 0);
        const int   sideLayer = i / 2;
        const float x = isLeft
                            ? (-centerBlank * 0.5f - static_cast<float>(sideLayer) * spacing)
                            : ( centerBlank * 0.5f + static_cast<float>(sideLayer) * spacing);

        pb.cat->SetPosition({x, spawnY});

        pb.cat->SetFacingByDirection(isLeft ? 1 : -1);
    }
}

void LocalPlayGameScene::UpdateCooperativePower() const {
    int bestGroup = 1;

    for (int i = 0; i < static_cast<int>(m_Players.size()); ++i) {
        const auto& pi = m_Players[i];
        if (pi.entered || pi.cat == nullptr) continue;
        if (pi.cat->GetMoveDir() == 0) continue;

        int group = 1;

        for (int j = 0; j < static_cast<int>(m_Players.size()); ++j) {
            if (i == j) continue;
            const auto& pj = m_Players[j];
            if (pj.entered || pj.cat == nullptr) continue;

            const bool sameDir = (pi.cat->GetMoveDir() == pj.cat->GetMoveDir());

            const glm::vec2 posI = pi.cat->GetPosition();
            const glm::vec2 posJ = pj.cat->GetPosition();

            const float halfWI = std::max(16.f,
                                          std::abs(pi.cat->GetScaledSize().x) * 0.5f);
            const float halfWJ = std::max(16.f,
                                          std::abs(pj.cat->GetScaledSize().x) * 0.5f);

            const bool touching =
                std::abs(posI.x - posJ.x) <= (halfWI + halfWJ) * 1.02f &&
                std::abs(posI.y - posJ.y) <= 40.f;

            if (sameDir && touching) ++group;
        }

        bestGroup = std::max(bestGroup, group);
    }

    m_Session.SetCooperativePushPower(bestGroup);
}

void LocalPlayGameScene::UpdateDoorCountText() const {
    if (m_DoorCountText == nullptr) return;
    m_DoorCountText->SetText(
        std::to_string(m_EnteredCount) + "/" +
        std::to_string(m_Session.GetSelectedPlayerCount()));
}

