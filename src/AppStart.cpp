#include "App.hpp"
#include "BGMPlayer.hpp"
#include "CatAssets.hpp"

#include "Titlescene.hpp"
#include "Menuscene.hpp"
#include "ExitConfirmScene.hpp"
#include "OptionMenuScene.hpp"
#include "KeyboardConfigScene.hpp"
#include "LocalPlayScene.hpp"
#include "LocalPlayGameScene.hpp"
#include "LevelSelectScene.hpp"

#include "Util/Logger.hpp"
#include <array>
#include <cmath>

// App::Start
void App::Start() {
    LOG_TRACE("Start");

    // GameContext
    m_Ctx = std::make_unique<GameContext>(m_Root);

    m_Ctx->Background = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/white_background.png");
    m_Ctx->Background->SetZIndex(-10);
    m_Ctx->Background->SetScale({100.0f, 100.0f});
    m_Root.AddChild(m_Ctx->Background);

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

    m_Ctx->Door = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/door_close.png");
    m_Ctx->Door->SetScale({0.148f, 0.161f});   // ← 新增這一行
    m_Ctx->Door->SetZIndex(5.0f);
    {
        const float floorY     = m_Ctx->Floor->GetPosition().y;
        const float floorHalfH = m_Ctx->Floor->GetScaledSize().y / 2.0f;
        const float doorHalfH  = m_Ctx->Door->GetScaledSize().y  / 2.0f;
        m_Ctx->Door->SetPosition({0.0f, floorY + floorHalfH + doorHalfH});
    }
    m_Root.AddChild(m_Ctx->Door);

    m_Ctx->BGMPlayer = std::make_unique<BGMPlayer>();
    m_Ctx->BGMPlayer->Play();

    // 建立 8 隻貓
    m_Ctx->StartupCats.clear();
    m_Ctx->StartupCats.reserve(GameContext::kCatColorOrder.size());

    for (int i = 0; i < static_cast<int>(GameContext::kCatColorOrder.size()); ++i) {
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
            CatAssets::BuildFullAnimPaths(GameContext::kCatColorOrder[static_cast<size_t>(i)]),
            left, right, jump);
        cat->SetZIndex(20.0f + static_cast<float>(i) * 0.01f);
        cat->SetInputEnabled(i < 2);
        m_Root.AddChild(cat);
        m_Ctx->StartupCats.push_back(cat);
    }

    // 計算初始位置
    const float floorY      = m_Ctx->Floor->GetPosition().y;
    const float floorHalfH  = m_Ctx->Floor->GetScaledSize().y / 2.0f;

    constexpr float spacing     = 75.0f;
    constexpr float centerBlank = spacing * 2.0f;

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

    // 場景建立
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
        menuScene->GetBlueCatRunImg(),   // ← 新增
        kbConfigScene.get());

    auto localPlayGameScene = std::make_unique<LocalPlayGameScene>(
        *m_Ctx,
        localPlayScene.get(),
        kbConfigScene.get());

    auto levelSelectScene = std::make_unique<LevelSelectScene>(
        *m_Ctx,
        localPlayGameScene.get());

    titleScene->SetMenuScene(menuScene.get());
    menuScene->SetExitConfirmScene(exitConfirmScene.get());
    menuScene->SetOptionScene(optionMenuScene.get());
    menuScene->SetLocalPlayScene(localPlayScene.get());
    optionMenuScene->SetKeyboardConfigScene(kbConfigScene.get());
    localPlayScene->SetGameScene(localPlayGameScene.get());
    localPlayGameScene->SetLevelSelectScene(levelSelectScene.get());

    m_TitleScene          = std::move(titleScene);
    m_MenuScene           = std::move(menuScene);
    m_ExitConfirmScene    = std::move(exitConfirmScene);
    m_OptionMenuScene     = std::move(optionMenuScene);
    m_KeyboardConfigScene = std::move(kbConfigScene);
    m_LocalPlayScene      = std::move(localPlayScene);
    m_LocalPlayGameScene  = std::move(localPlayGameScene);
    m_LevelSelectScene    = std::move(levelSelectScene);

    TransitionTo(m_TitleScene.get());

    m_CurrentState = State::UPDATE;
}