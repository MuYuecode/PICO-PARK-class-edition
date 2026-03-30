#include "App.hpp"
#include "BGMPlayer.hpp"
#include "CatAssets.hpp"
#include "PushableBox.hpp"

#include "TitleScene.hpp"
#include "MenuScene.hpp"
#include "ExitConfirmScene.hpp"
#include "OptionMenuScene.hpp"
#include "KeyboardConfigScene.hpp"
#include "LocalPlayScene.hpp"
#include "LocalPlayGameScene.hpp"
#include "LevelSelectScene.hpp"
#include "LevelOneScene.hpp"
#include "SceneId.hpp"

#include "Util/Logger.hpp"
#include <array>
#include <cmath>

using namespace std;

void App::Start() {
    LOG_TRACE("Start");

    m_Ctx = make_unique<GameContext>(m_Root);
    m_SceneManager = make_unique<SceneManager>(*m_Ctx);

    m_Ctx->Background = make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/white_background.png");
    m_Ctx->Background->SetZIndex(-10);
    m_Ctx->Background->SetScale({100.0f, 100.0f});
    m_Root.AddChild(m_Ctx->Background);

    m_Ctx->Floor = make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/background_floor.png");
    m_Ctx->Floor->SetZIndex(0);
    m_Ctx->Floor->SetPosition({0.0f, -340.0f});
    m_Root.AddChild(m_Ctx->Floor);

    m_Ctx->Header = make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/header.png");
    m_Ctx->Header->SetZIndex(0);
    m_Ctx->Header->SetPosition({0.0f, 135.0f});
    m_Root.AddChild(m_Ctx->Header);

    m_Ctx->Door = make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/door_close.png");
    m_Ctx->Door->SetScale({0.148f, 0.161f});
    m_Ctx->Door->SetZIndex(5.0f);
    {
        const float floorY     = m_Ctx->Floor->GetPosition().y;
        const float floorHalfH = m_Ctx->Floor->GetScaledSize().y / 2.0f;
        const float doorHalfH  = m_Ctx->Door->GetScaledSize().y / 2.0f;
        m_Ctx->Door->SetPosition({0.0f, floorY + floorHalfH + doorHalfH});
    }
    m_Root.AddChild(m_Ctx->Door);

    m_Ctx->TestBox = make_shared<PushableBox>(
        GA_RESOURCE_DIR "/Image/Level_Cover/LevelOneScene/Box.png", 1);
    m_Ctx->TestBox->SetZIndex(15.0f);
    {
        const float floorY     = m_Ctx->Floor->GetPosition().y;
        const float floorHalfH = m_Ctx->Floor->GetScaledSize().y / 2.0f;
        const float boxHalfH   = m_Ctx->TestBox->GetScaledSize().y / 2.0f;
        m_Ctx->TestBox->SetPosition({400.0f, floorY + floorHalfH + boxHalfH});
    }
    m_Root.AddChild(m_Ctx->TestBox);
    if (m_Ctx->TestBox->GetTextObject()) {
        m_Ctx->TestBox->GetTextObject()->SetZIndex(16.0f);
        m_Root.AddChild(m_Ctx->TestBox->GetTextObject());
    }

    m_Ctx->BGMPlayer = make_shared<BGMPlayer>();
    m_Ctx->BGMPlayer->Play();

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
        } else if (i == 1) {
            left  = Util::Keycode::LEFT;
            right = Util::Keycode::RIGHT;
            jump  = Util::Keycode::UP;
        }

        auto cat = make_shared<PlayerCat>(
            CatAssets::BuildFullAnimPaths(GameContext::kCatColorOrder[static_cast<size_t>(i)]),
            left, right, jump);
        cat->SetZIndex(20.0f + static_cast<float>(i) * 0.01f);
        cat->SetInputEnabled(i < 2);
        m_Root.AddChild(cat);
        m_Ctx->StartupCats.push_back(cat);
    }

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
        cat->SetZIndex(15.0f + static_cast<float>(i) * 0.01f);

        const float faceScale = abs(cat->m_Transform.scale.x);
        cat->m_Transform.scale.x = isLeftSide ? faceScale : -faceScale;
    }

    auto titleScene = make_unique<TitleScene>(*m_Ctx);

    auto menuScene = make_unique<MenuScene>(*m_Ctx);

    auto exitConfirmScene = make_unique<ExitConfirmScene>(
        *m_Ctx,
        menuScene->GetMenuFrame(),
        menuScene->GetExitGameButton());

    auto optionMenuScene = make_unique<OptionMenuScene>(
        *m_Ctx,
        menuScene->GetExitGameButton());

    auto kbConfigScene = make_unique<KeyboardConfigScene>(
        *m_Ctx,
        menuScene->GetExitGameButton());

    auto localPlayScene = make_unique<LocalPlayScene>(
        *m_Ctx,
        menuScene->GetMenuFrame(),
        menuScene->GetExitGameButton(),
        menuScene->GetLeftTriButton(),
        menuScene->GetRightTriButton(),
        menuScene->GetBlueCatRunImg(),
        kbConfigScene.get());

    auto localPlayGameScene = make_unique<LocalPlayGameScene>(*m_Ctx);

    auto levelSelectScene = make_unique<LevelSelectScene>(*m_Ctx);
    levelSelectScene->SetLevelSceneId(0, SceneId::Level01);

    auto levelOneScene = make_unique<LevelOneScene>(*m_Ctx);

    m_SceneManager->Register(SceneId::Title, std::move(titleScene));
    m_SceneManager->Register(SceneId::Menu, std::move(menuScene));
    m_SceneManager->Register(SceneId::ExitConfirm, std::move(exitConfirmScene));
    m_SceneManager->Register(SceneId::OptionMenu, std::move(optionMenuScene));
    m_SceneManager->Register(SceneId::KeyboardConfig, std::move(kbConfigScene));
    m_SceneManager->Register(SceneId::LocalPlay, std::move(localPlayScene));
    m_SceneManager->Register(SceneId::LocalPlayGame, std::move(localPlayGameScene));
    m_SceneManager->Register(SceneId::LevelSelect, std::move(levelSelectScene));
    m_SceneManager->Register(SceneId::Level01, std::move(levelOneScene));

    m_SceneManager->GoTo(SceneId::Title);
    m_CurrentState = State::UPDATE;
}
