#include "LocalPlayGameScene.hpp"

#include <algorithm>
#include <array>
#include <cmath>

#include "LocalPlayScene.hpp"
#include "Util/Input.hpp"
#include "Util/Logger.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// 工具（匿名 namespace）
// ─────────────────────────────────────────────────────────────────────────────
namespace {

std::string BuildCatFramePath(const std::string& color,
                               const std::string& action,
                               int frameNum) {
    return std::string(GA_RESOURCE_DIR) +
           "/Image/Character/" + color + "_cat/" +
           color + "_cat_" + action + "_" + std::to_string(frameNum) + ".png";
}

std::vector<std::string> BuildCatFramePaths(const std::string& color,
                                             const std::string& action,
                                             int numFrames) {
    std::vector<std::string> paths;
    paths.reserve(static_cast<size_t>(numFrames));
    for (int f = 1; f <= numFrames; ++f) {
        paths.push_back(BuildCatFramePath(color, action, f));
    }
    return paths;
}

CatAnimPaths BuildCatAnimPaths(const std::string& color) {
    CatAnimPaths p;
    p.stand     = BuildCatFramePaths(color, "stand", 2);
    p.run       = BuildCatFramePaths(color, "run",   2);
    p.jump_rise = { BuildCatFramePath(color, "jump", 1) };
    p.jump_fall = { BuildCatFramePath(color, "jump", 2) };
    p.land      = { BuildCatFramePath(color, "land", 1) };
    p.push      = BuildCatFramePaths(color, "push",  2);
    return p;
}

} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// 建構子
// ─────────────────────────────────────────────────────────────────────────────
LocalPlayGameScene::LocalPlayGameScene(GameContext& ctx,
                                       LocalPlayScene* localPlayScene,
                                       KeyboardConfigScene* kbConfigScene)
    : Scene(ctx)
    , m_LocalPlayScene(localPlayScene)
    , m_KbConfigScene(kbConfigScene) {}

// ─────────────────────────────────────────────────────────────────────────────
// OnEnter
// ─────────────────────────────────────────────────────────────────────────────
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
    ApplyInitialFormation();

    LOG_INFO("LocalPlayGameScene::OnEnter players={}", playerCount);
}

// ─────────────────────────────────────────────────────────────────────────────
// OnExit
// ─────────────────────────────────────────────────────────────────────────────
void LocalPlayGameScene::OnExit() {
    for (auto& pb : m_Players) {
        if (pb.agent.actor != nullptr) {
            m_Ctx.Root.RemoveChild(pb.agent.actor);
        }
    }
    m_Players.clear();

    for (auto& cat : m_Ctx.StartupCats) {
        if (cat != nullptr) cat->SetVisible(true);
    }

    LOG_INFO("LocalPlayGameScene::OnExit");
}

// ─────────────────────────────────────────────────────────────────────────────
// Update
// ─────────────────────────────────────────────────────────────────────────────
Scene* LocalPlayGameScene::Update() {
    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
        return m_LocalPlayScene;
    }

    // ── 讀取輸入（Scene 的責任）──────────────────────────────────────────
    for (auto& pb : m_Players) {
        auto& st  = pb.agent.state;
        const auto& key = pb.key;

        st.moveDir = 0;

        const bool goLeft  = (key.left  != Util::Keycode::UNKNOWN) &&
                              Util::Input::IsKeyPressed(key.left);
        const bool goRight = (key.right != Util::Keycode::UNKNOWN) &&
                              Util::Input::IsKeyPressed(key.right);

        if      (goLeft  && !goRight) st.moveDir = -1;
        else if (goRight && !goLeft)  st.moveDir =  1;

        const bool wantJump = (key.jump != Util::Keycode::UNKNOWN) &&
                               Util::Input::IsKeyDown(key.jump);

        if (st.grounded && wantJump) {
            st.velocityY    = CharacterPhysicsSystem::kJumpForce;
            st.grounded     = false;
            st.supportIndex = -1;
        }
    }

    // ── 攤平成 vector<PhysicsAgent>（System 需要整體視野）────────────────
    std::vector<PhysicsAgent> agents;
    agents.reserve(m_Players.size());
    for (auto& pb : m_Players) agents.push_back(pb.agent);

    // ── 委託物理系統 ──────────────────────────────────────────────────────
    m_Physics.Update(agents, m_Ctx.Floor);

    // ── 把更新後的狀態寫回（System 修改的是副本）─────────────────────────
    for (int i = 0; i < static_cast<int>(m_Players.size()); ++i) {
        m_Players[i].agent.state = agents[i].state;
    }

    UpdateCooperativePower();
    return nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// SpawnPlayers
// ─────────────────────────────────────────────────────────────────────────────
void LocalPlayGameScene::SpawnPlayers(int count) {
    static const std::array<const char*, 8> kColorOrder = {
        "blue", "red", "yellow", "green", "purple", "pink", "orange", "gray"
    };

    m_Players.clear();
    m_Players.reserve(static_cast<size_t>(count));

    for (int i = 0; i < count; ++i) {
        const std::string color = kColorOrder[static_cast<size_t>(i)];

        auto cat = std::make_shared<PlayerCat>(
            BuildCatAnimPaths(color),
            Util::Keycode::UNKNOWN,
            Util::Keycode::UNKNOWN,
            Util::Keycode::UNKNOWN);
        cat->SetZIndex(20.0f + static_cast<float>(i) * 0.01f);

        PlayerBinding pb;
        pb.agent.actor = cat;

        if (m_KbConfigScene != nullptr) {
            pb.key = m_KbConfigScene->GetAppliedConfig(i);
        }

        // 預設按鍵 fallback
        if (i == 0 &&
            pb.key.left  == Util::Keycode::UNKNOWN &&
            pb.key.right == Util::Keycode::UNKNOWN) {
            pb.key = KeyboardConfigScene::k_Default1P;
        }
        else if (i == 1 &&
                 pb.key.left  == Util::Keycode::UNKNOWN &&
                 pb.key.right == Util::Keycode::UNKNOWN) {
            pb.key = KeyboardConfigScene::k_Default2P;
        }

        m_Players.push_back(pb);
        m_Ctx.Root.AddChild(cat);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// ApplyInitialFormation
// ─────────────────────────────────────────────────────────────────────────────
void LocalPlayGameScene::ApplyInitialFormation() {
    const float floorY     = (m_Ctx.Floor != nullptr) ? m_Ctx.Floor->GetPosition().y : -340.0f;
    const float floorHalfH = (m_Ctx.Floor != nullptr) ? m_Ctx.Floor->GetScaledSize().y / 2.0f : 0.0f;

    // spacing：角色半寬 * 1.7 * 2 = 問題 2 修正（原 0.42 已加倍至 0.84）
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
        pb.agent.state = PhysicsState{};  // 重置所有物理狀態

        const float faceScale = std::abs(pb.agent.actor->m_Transform.scale.x);
        pb.agent.actor->m_Transform.scale.x = isLeftSide ? faceScale : -faceScale;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateCooperativePower
// ─────────────────────────────────────────────────────────────────────────────
void LocalPlayGameScene::UpdateCooperativePower() {
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