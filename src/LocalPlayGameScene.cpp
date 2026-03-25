#include "LocalPlayGameScene.hpp"
#include "LocalPlayScene.hpp"
#include "LevelSelectScene.hpp"

#include "CatAssets.hpp"
#include "Util/Input.hpp"
#include "Util/Logger.hpp"

#include <algorithm>
#include <array>
#include <cmath>

using ip = Util::Input ;
using k  = Util::Keycode ;

LocalPlayGameScene::LocalPlayGameScene(GameContext& ctx,
                                       LocalPlayScene* localPlayScene,
                                       KeyboardConfigScene* kbConfigScene)
    : Scene(ctx)
    , m_LocalPlayScene(localPlayScene)
    , m_KbConfigScene(kbConfigScene) {}

void LocalPlayGameScene::OnEnter() {
    int playerCount = 2;
    if (m_LocalPlayScene != nullptr) {
        playerCount = m_LocalPlayScene->GetPlayerCount();
    }
    playerCount = std::clamp(playerCount,
                             LocalPlayScene::MIN_PLAYERS,
                             LocalPlayScene::MAX_PLAYERS);

    m_Ctx.SelectedPlayerCount  = playerCount;
    m_Ctx.CooperativePushPower = 1;

    for (auto& cat : m_Ctx.StartupCats) {
        if (cat != nullptr) {
            cat->SetVisible(false);
            cat->SetInputEnabled(false);
        }
    }

    SpawnPlayers(playerCount);

    m_Ctx.Door->SetImage(GA_RESOURCE_DIR "/Image/Background/door_open.png");

    m_EnteredCount = 0;
    const glm::vec2 doorPos   = m_Ctx.Door->GetPosition();
    const float     doorHalfH = m_Ctx.Door->GetScaledSize().y / 2.0f;

    m_DoorCountText = std::make_shared<GameText>(
        "0/" + std::to_string(playerCount),
        40,
        Util::Color::FromRGB(0, 0, 0, 255));
    m_DoorCountText->SetZIndex(30.0f);
    m_DoorCountText->SetPosition({doorPos.x + 10.0f, doorPos.y + doorHalfH + 25.0f});
    m_Ctx.Root.AddChild(m_DoorCountText);

    LOG_INFO("LocalPlayGameScene::OnEnter players={}", playerCount);
}

void LocalPlayGameScene::OnExit() {
    for (auto& pb : m_Players) {
        if (pb.agent.actor != nullptr) {
            m_Ctx.Root.RemoveChild(pb.agent.actor);
        }
    }

    m_Ctx.Door->SetImage(GA_RESOURCE_DIR "/Image/Background/door_close.png");
    if (m_DoorCountText != nullptr) {
        m_Ctx.Root.RemoveChild(m_DoorCountText);
        m_DoorCountText = nullptr;
    }

    m_Players.clear();

    for (auto& cat : m_Ctx.StartupCats) {
        if (cat != nullptr) cat->SetVisible(true);
    }

    LOG_INFO("LocalPlayGameScene::OnExit");
}

Scene* LocalPlayGameScene::Update() {

    if (ip::IsKeyDown(k::ESCAPE)) {
        return m_LocalPlayScene;
    }

    for (auto& pb : m_Players) {
        auto& st        = pb.agent.state;
        const auto& key = pb.key;

        st.moveDir = 0;

        const bool goLeft  = (key.left  != k::UNKNOWN) &&
                              ip::IsKeyPressed(key.left);
        const bool goRight = (key.right != k::UNKNOWN) &&
                              ip::IsKeyPressed(key.right);

        if      (goLeft  && !goRight) st.moveDir = -1;
        else if (goRight && !goLeft)  st.moveDir =  1;

        const bool wantJump = (key.jump != k::UNKNOWN) &&
                               ip::IsKeyDown(key.jump);

        if (st.grounded && wantJump) {
            CharacterPhysicsSystem::ApplyJump(st);
        }
    }

    const glm::vec2 doorPos  = m_Ctx.Door->GetPosition();
    const glm::vec2 doorSize = m_Ctx.Door->GetScaledSize();
    const float halfW = doorSize.x / 2.0f + 10.0f;
    const float halfH = doorSize.y / 2.0f + 10.0f;

    for (auto& pb : m_Players) {
        if (pb.entered)                  continue;
        if (pb.agent.actor == nullptr)   continue;

        const glm::vec2 pos    = pb.agent.actor->GetPosition();
        const bool      inRange = (std::abs(pos.x - doorPos.x) <= halfW &&
                                   std::abs(pos.y - doorPos.y) <= halfH);

        const bool pressedUp = (pb.key.up != k::UNKNOWN) &&
                                ip::IsKeyDown(pb.key.up);

        if (inRange && pressedUp) {
            pb.entered = true;
            ++m_EnteredCount;
            pb.agent.actor->SetVisible(false);
            pb.agent.actor->SetInputEnabled(false);
            pb.agent.actor->SetPosition({640, -360});
            UpdateDoorCountText();

            if (m_EnteredCount == m_Ctx.SelectedPlayerCount) {
                LOG_INFO("LocalPlayGameScene all players entered the door, going to LevelSelectScene");
                return m_LevelSelectScene;
            }
        }
    }

    std::vector<PhysicsAgent> agents;
    agents.reserve(m_Players.size());
    for (auto& pb : m_Players) agents.push_back(pb.agent);

    CharacterPhysicsSystem::Update(agents, m_Ctx.Floor);

    for (int i = 0; i < static_cast<int>(m_Players.size()); ++i) {
        m_Players[i].agent.state = agents[i].state;
    }

    UpdateCooperativePower();
    return nullptr;
}

void LocalPlayGameScene::SpawnPlayers(int count) {
    m_Players.clear();
    m_Players.reserve(static_cast<size_t>(count));

    for (int i = 0; i < count; ++i) {
        const std::string color = GameContext::kCatColorOrder[static_cast<size_t>(i)];

        auto cat = std::make_shared<PlayerCat>(
            CatAssets::BuildFullAnimPaths(color),
            k::UNKNOWN,
            k::UNKNOWN,
            k::UNKNOWN);
        cat->SetZIndex(20.0f + static_cast<float>(i) * 0.01f);

        PlayerBinding pb;
        pb.agent.actor = cat;

        if (m_KbConfigScene != nullptr) {
            pb.key = m_KbConfigScene->GetAppliedConfig(i);
        }

        if (i == 0 &&
            pb.key.left  == k::UNKNOWN &&
            pb.key.right == k::UNKNOWN) {
            pb.key = KeyboardConfigScene::k_Default1P;
        }
        else if (i == 1 &&
                 pb.key.left  == k::UNKNOWN &&
                 pb.key.right == k::UNKNOWN) {
            pb.key = KeyboardConfigScene::k_Default2P;
        }

        if (i < static_cast<int>(m_Ctx.StartupCats.size()) &&
            m_Ctx.StartupCats[i] != nullptr) {
            cat->SetPosition(m_Ctx.StartupCats[i]->GetPosition());
        } else {
            const float floorY     = (m_Ctx.Floor != nullptr) ? m_Ctx.Floor->GetPosition().y : -340.0f;
            const float floorHalfH = (m_Ctx.Floor != nullptr) ? m_Ctx.Floor->GetScaledSize().y / 2.0f : 0.0f;
            const float charHalfH  = cat->GetScaledSize().y / 2.0f;
            cat->SetPosition({0.0f, floorY + floorHalfH + charHalfH});
        }

        m_Players.push_back(pb);
        m_Ctx.Root.AddChild(cat);
    }
}

void LocalPlayGameScene::ApplyInitialFormation() {
    const float floorY     = (m_Ctx.Floor != nullptr) ? m_Ctx.Floor->GetPosition().y : -340.0f;
    const float floorHalfH = (m_Ctx.Floor != nullptr) ? m_Ctx.Floor->GetScaledSize().y / 2.0f : 0.0f;

    const float halfW0  = (m_Players.empty() || m_Players[0].agent.actor == nullptr)
                              ? 32.0f
                              : std::max(16.0f, std::abs(m_Players[0].agent.actor->GetScaledSize().x) * 0.5f);
    const float spacing     = halfW0 * 1.7f;
    const float centerBlank = spacing * 2.0f;

    for (int i = 0; i < static_cast<int>(m_Players.size()); ++i) {
        auto& pb = m_Players[i];
        if (pb.agent.actor == nullptr) continue;

        const float charHalfH   = pb.agent.actor->GetScaledSize().y / 2.0f;
        const float standOffset = floorHalfH + charHalfH;
        const float spawnY      = floorY + standOffset;

        const bool  isLeftSide = (i % 2 == 0);
        const int   sideLayer  = i / 2;
        const float x = isLeftSide
                            ? (-centerBlank * 0.5f - static_cast<float>(sideLayer) * spacing)
                            : ( centerBlank * 0.5f + static_cast<float>(sideLayer) * spacing);

        pb.agent.actor->SetPosition({x, spawnY});
        pb.agent.state = PhysicsState{};

        const float faceScale = std::abs(pb.agent.actor->m_Transform.scale.x);
        pb.agent.actor->m_Transform.scale.x = isLeftSide ? faceScale : -faceScale;
    }
}

void LocalPlayGameScene::UpdateCooperativePower() const {
    int bestGroup = 1;

    for (int i = 0; i < static_cast<int>(m_Players.size()); ++i) {
        if (m_Players[i].agent.state.moveDir == 0) continue;

        int group = 1;
        for (int j = 0; j < static_cast<int>(m_Players.size()); ++j) {
            if (i == j) continue;

            const bool sameDir = (m_Players[i].agent.state.moveDir ==
                                  m_Players[j].agent.state.moveDir);

            const glm::vec2 pi = m_Players[i].agent.actor->GetPosition();
            const glm::vec2 pj = m_Players[j].agent.actor->GetPosition();

            const float halfWi = std::max(16.0f, std::abs(m_Players[i].agent.actor->GetScaledSize().x) * 0.5f);
            const float halfWj = std::max(16.0f, std::abs(m_Players[j].agent.actor->GetScaledSize().x) * 0.5f);

            const bool touching =
                std::abs(pi.x - pj.x) <= (halfWi + halfWj) * 1.02f &&
                std::abs(pi.y - pj.y) <= 40.0f;

            if (sameDir && touching) ++group;
        }
        bestGroup = std::max(bestGroup, group);
    }

    m_Ctx.CooperativePushPower = bestGroup;
}

void LocalPlayGameScene::UpdateDoorCountText() const {
    if (m_DoorCountText == nullptr) return;
    m_DoorCountText->SetText(
        std::to_string(m_EnteredCount) + "/" +
        std::to_string(m_Ctx.SelectedPlayerCount));
}