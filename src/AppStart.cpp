#include "App.hpp"
#include "BGMPlayer.hpp"
#include "Titlescene.hpp"
#include "Menuscene.hpp"
#include "ExitConfirmScene.hpp"
#include "OptionMenuScene.hpp"
#include "KeyboardConfigScene.hpp"
#include "LocalPlayScene.hpp"
#include "LocalPlayGameScene.hpp"
#include "Util/Logger.hpp"

#include <array>
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
// 工具：建立單幀路徑 / 多幀路徑
// ─────────────────────────────────────────────────────────────────────────────
static std::string BuildCatFramePath(const std::string& color,
                                     const std::string& action,
                                     int frameNum) {
    return std::string(GA_RESOURCE_DIR) +
           "/Image/Character/" + color + "_cat/" +
           color + "_cat_" + action + "_" + std::to_string(frameNum) + ".png";
}

static std::vector<std::string> BuildCatFramePaths(const std::string& color,
                                                    const std::string& action,
                                                    int numFrames) {
    std::vector<std::string> paths;
    paths.reserve(static_cast<size_t>(numFrames));
    for (int f = 1; f <= numFrames; ++f) {
        paths.push_back(BuildCatFramePath(color, action, f));
    }
    return paths;
}

// ─────────────────────────────────────────────────────────────────────────────
// 建立一隻貓的完整動畫路徑集合
//   stand : stand_1, stand_2
//   run   : run_1, run_2
//   jump_rise : jump_1         ← 起跳到最高點
//   jump_fall : jump_2         ← 最高點到落地前
//   land  : land_1
//   push  : push_1, push_2
// ─────────────────────────────────────────────────────────────────────────────
static CatAnimPaths BuildCatAnimPaths(const std::string& color) {
    CatAnimPaths p;
    p.stand     = BuildCatFramePaths(color, "stand", 8);
    p.run       = BuildCatFramePaths(color, "run",   9);
    p.jump_rise = { BuildCatFramePath(color, "jump", 1) };  // jump_1 只有一幀
    p.jump_fall = { BuildCatFramePath(color, "jump", 2) };  // jump_2 只有一幀
    p.land      = { BuildCatFramePath(color, "land", 1) };  // land_1 只有一幀
    p.push      = BuildCatFramePaths(color, "push",  3);
    return p;
}

// ─────────────────────────────────────────────────────────────────────────────
// App::Start
// ─────────────────────────────────────────────────────────────────────────────
void App::Start() {
    LOG_TRACE("Start");

    // ── Step 1：GameContext ───────────────────────────────────────────────────
    m_Ctx = std::make_unique<GameContext>(m_Root);

    m_Ctx->WhiteBackground = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/white_background.png");
    m_Ctx->WhiteBackground->SetZIndex(-10);
    m_Ctx->WhiteBackground->SetScale({100.0f, 100.0f});
    m_Root.AddChild(m_Ctx->WhiteBackground);

    m_Ctx->Floor = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/background_floor.png");
    m_Ctx->Floor->SetZIndex(0);
    m_Ctx->Floor->SetPosition({0.0f, -340.0f});
    m_Root.AddChild(m_Ctx->Floor);

    m_Ctx->Header = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/header.png");
    m_Ctx->Header->SetZIndex(0);
    m_Ctx->Header->SetPosition({0.0f, 135.0f});
    m_Root.AddChild(m_Ctx->Header);

    m_Ctx->BGMPlayer = std::make_unique<BGMPlayer>();
    m_Ctx->BGMPlayer->Play();

    // ── 建立 8 隻裝飾貓（StartupCats）────────────────────────────────────────
    static const std::array<const char*, 8> kColorOrder = {
        "blue", "red", "yellow", "green", "purple", "pink", "orange", "gray"
    };

    m_Ctx->StartupCats.clear();
    m_Ctx->StartupCats.reserve(kColorOrder.size());

    for (int i = 0; i < static_cast<int>(kColorOrder.size()); ++i) {
        Util::Keycode left  = Util::Keycode::UNKNOWN;
        Util::Keycode right = Util::Keycode::UNKNOWN;
        Util::Keycode jump  = Util::Keycode::UNKNOWN;

        if (i == 0) {
            left  = Util::Keycode::A;
            right = Util::Keycode::D;
            jump  = Util::Keycode::W;
        }
        else if (i == 1) {
            left  = Util::Keycode::LEFT;
            right = Util::Keycode::RIGHT;
            jump  = Util::Keycode::UP;
        }

        auto cat = std::make_shared<PlayerCat>(
            BuildCatAnimPaths(kColorOrder[static_cast<size_t>(i)]),
            left, right, jump);
        cat->SetZIndex(20.0f + static_cast<float>(i) * 0.01f);
        cat->SetInputEnabled(i < 2);
        m_Root.AddChild(cat);
        m_Ctx->StartupCats.push_back(cat);
    }

    // ── 計算初始位置 ──────────────────────────────────────────────────────────
    // 問題 1 修正：spawnY 用 floor 半高 + 角色半高（動態，每隻貓各自計算）
    // 問題 2 修正：spacing 係數 0.42 → 0.84（乘 2 倍），避免碰撞箱重疊
    const float floorY      = m_Ctx->Floor->GetPosition().y;
    const float floorHalfH  = m_Ctx->Floor->GetScaledSize().y / 2.0f;

    const float spacing     = 75.0f;
    const float centerBlank = spacing * 2.0f;

    for (int i = 0; i < static_cast<int>(m_Ctx->StartupCats.size()); ++i) {
        auto& cat = m_Ctx->StartupCats[i];

        const float charHalfH   = cat->GetScaledSize().y / 2.0f;
        const float standOffset = floorHalfH + charHalfH;
        const float spawnY      = floorY + standOffset + 100.0f;

        const bool  isLeftSide = (i % 2 == 0);
        const int   sideLayer  = i / 2;
        const float x = isLeftSide
                            ? (-centerBlank * 0.5f - static_cast<float>(sideLayer) * spacing)
                            : ( centerBlank * 0.5f + static_cast<float>(sideLayer) * spacing);

        cat->SetPosition({x, spawnY});

        const float faceScale = std::abs(cat->m_Transform.scale.x);
        cat->m_Transform.scale.x = isLeftSide ? faceScale : -faceScale;
    }

    // ── Step 2：場景建立 ──────────────────────────────────────────────────────
    auto titleScene = std::make_unique<TitleScene>(*m_Ctx, nullptr);

    auto menuScene = std::make_unique<MenuScene>(
        *m_Ctx, titleScene.get(), nullptr, nullptr, nullptr);

    auto exitConfirmScene = std::make_unique<ExitConfirmScene>(
        *m_Ctx,
        menuScene.get(),
        menuScene->GetMenuFrame(),
        menuScene->GetExitGameButton());

    auto optionMenuScene = std::make_unique<OptionMenuScene>(
        *m_Ctx,
        menuScene.get(),
        menuScene->GetExitGameButton());

    auto kbConfigScene = std::make_unique<KeyboardConfigScene>(
        *m_Ctx,
        optionMenuScene.get(),
        menuScene->GetExitGameButton());

    auto localPlayScene = std::make_unique<LocalPlayScene>(
        *m_Ctx,
        menuScene.get(),
        menuScene->GetMenuFrame(),
        menuScene->GetExitGameButton(),
        menuScene->GetLeftTriButton(),
        menuScene->GetRightTriButton(),
        kbConfigScene.get());

    auto localPlayGameScene = std::make_unique<LocalPlayGameScene>(
        *m_Ctx,
        localPlayScene.get(),
        kbConfigScene.get());

    titleScene->SetMenuScene(menuScene.get());
    menuScene->SetExitConfirmScene(exitConfirmScene.get());
    menuScene->SetOptionScene(optionMenuScene.get());
    menuScene->SetLocalPlayScene(localPlayScene.get());
    optionMenuScene->SetKeyboardConfigScene(kbConfigScene.get());
    localPlayScene->SetGameScene(localPlayGameScene.get());

    m_TitleScene          = std::move(titleScene);
    m_MenuScene           = std::move(menuScene);
    m_ExitConfirmScene    = std::move(exitConfirmScene);
    m_OptionMenuScene     = std::move(optionMenuScene);
    m_KeyboardConfigScene = std::move(kbConfigScene);
    m_LocalPlayScene      = std::move(localPlayScene);
    m_LocalPlayGameScene  = std::move(localPlayGameScene);

    TransitionTo(m_TitleScene.get());

    m_CurrentState = State::UPDATE;
}