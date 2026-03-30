//
// Created by cody2 on 2026/3/17.
//

#include "LocalPlayScene.hpp"
#include "KeyboardConfigScene.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

const Util::Color LocalPlayScene::k_Black = Util::Color::FromRGB(0,   0,   0,   255);
const Util::Color LocalPlayScene::k_Red   = Util::Color::FromRGB(220, 50,  50,  255);

using ip = Util::Input ;
using k  = Util::Keycode ;

LocalPlayScene::LocalPlayScene(GameContext& ctx,
                               std::shared_ptr<Character>          menuFrame,
                               std::shared_ptr<Character>          exitGameButton,
                               std::shared_ptr<UITriangleButton> leftTriButton,
                               std::shared_ptr<UITriangleButton> rightTriButton,
                               std::shared_ptr<Character>          blueCatRunImg,
                               KeyboardConfigScene* kbConfigScene)
    : Scene(ctx)
    , m_MenuFrame(std::move(menuFrame))
    , m_ExitGameButton(std::move(exitGameButton))
    , m_LeftTriButton(std::move(leftTriButton))
    , m_RightTriButton(std::move(rightTriButton))
    , m_BlueCatRunImg(std::move(blueCatRunImg))
    , m_KbConfigScene(kbConfigScene)
{
    m_PlayerCountText = std::make_shared<GameText>("2PLAYER GAME", 65, k_Black);
    m_PlayerCountText->SetZIndex(20);
    m_PlayerCountText->SetPosition({0.0f, -153.0f}); // -118

    m_NoConfigText = std::make_shared<GameText>("No keyboard config", 30, k_Red);
    m_NoConfigText->SetZIndex(20);
    m_NoConfigText->SetPosition({0.0f, -188.0f});
    m_NoConfigText->SetVisible(false);
}

void LocalPlayScene::OnEnter() {
    LOG_INFO("LocalPlayScene::OnEnter  players={}", m_PlayerCount);
    
    m_Ctx.Root.AddChild(m_MenuFrame);
    m_Ctx.Root.AddChild(m_ExitGameButton);
    m_Ctx.Root.AddChild(m_LeftTriButton);
    m_Ctx.Root.AddChild(m_RightTriButton);
    m_Ctx.Root.AddChild(m_BlueCatRunImg);

    m_Ctx.Root.AddChild(m_PlayerCountText);
    m_Ctx.Root.AddChild(m_NoConfigText);

    m_MenuFrame->SetVisible(true);
    m_MenuFrame->SetScale({1.0f, 1.0f});
    m_MenuFrame->SetPosition({0.0f, -105.0f});

    m_BlueCatRunImg->SetVisible(true);
    m_BlueCatRunImg->SetZIndex(10);
    m_BlueCatRunImg->SetScale({1.2f, 1.2f});
    m_BlueCatRunImg->SetPosition({0.0f, -74.0f});

    m_ExitGameButton->SetVisible(true);
    m_ExitGameButton->SetPosition({331.0f, -14.0f});

    m_LeftTriButton->SetVisible(true);
    m_LeftTriButton->SetPosition({-305.0f, -153.0f});
    m_LeftTriButton->ResetState();

    m_RightTriButton->SetVisible(true);
    m_RightTriButton->SetPosition({305.0f, -153.0f});
    m_RightTriButton->ResetState();

    UpdateDisplay();
}

void LocalPlayScene::OnExit() {
    LOG_INFO("LocalPlayScene::OnExit");

    m_Ctx.Root.RemoveChild(m_PlayerCountText);
    m_Ctx.Root.RemoveChild(m_NoConfigText);

    m_MenuFrame->SetScale({1.0f, 1.0f});
    m_ExitGameButton->SetPosition({331.0f, -14.0f});

    m_Ctx.Root.RemoveChild(m_LeftTriButton);
    m_Ctx.Root.RemoveChild(m_RightTriButton);
    m_Ctx.Root.RemoveChild(m_BlueCatRunImg);
    m_Ctx.Root.RemoveChild(m_ExitGameButton);
    m_Ctx.Root.RemoveChild(m_MenuFrame);
}

SceneId LocalPlayScene::Update() {
    m_LeftTriButton->UpdateButton();
    m_RightTriButton->UpdateButton();

    if ( ip::IsKeyDown(k::ESCAPE) ||
        m_ExitGameButton->IsLeftClicked()) {
        LOG_INFO("LocalPlayScene: back to MenuScene");
        return SceneId::Menu;
    }

    const bool pressedLeft  =  ip::IsKeyDown(k::A)    ||
                               ip::IsKeyDown(k::LEFT)  ||
                              m_LeftTriButton->IsLeftClicked();

    const bool pressedRight =  ip::IsKeyDown(k::D)    ||
                               ip::IsKeyDown(k::RIGHT) ||
                              m_RightTriButton->IsLeftClicked();

    if (pressedLeft) {
        m_PlayerCount = ((m_PlayerCount - MIN_PLAYERS - 1 + (MAX_PLAYERS - MIN_PLAYERS + 1))
                         % (MAX_PLAYERS - MIN_PLAYERS + 1))
                        + MIN_PLAYERS;
        m_LeftTriButton->Press(75.0f);
        UpdateDisplay();
    }
    else if (pressedRight) {
        m_PlayerCount = ((m_PlayerCount - MIN_PLAYERS + 1)
                         % (MAX_PLAYERS - MIN_PLAYERS + 1))
                        + MIN_PLAYERS;
        m_RightTriButton->Press(75.0f);
        UpdateDisplay();
    }

    if ( ip::IsKeyDown(k::RETURN)) {
        int configuredCount = 0;
        if (m_KbConfigScene != nullptr) {
            configuredCount = m_KbConfigScene->GetConfiguredPlayerCount();
        }

        if (m_PlayerCount <= configuredCount) {
            m_Ctx.SelectedPlayerCount = m_PlayerCount;
            LOG_INFO("LocalPlayScene: ENTER confirmed with {} players", m_PlayerCount);
            return SceneId::LocalPlayGame;
        }

        LOG_INFO("LocalPlayScene: ENTER blocked, selected={} configured={}",
                 m_PlayerCount, configuredCount);
    }

    return SceneId::None;
}

void LocalPlayScene::UpdateDisplay() const {
    m_PlayerCountText->SetText(std::to_string(m_PlayerCount) + "PLAYER GAME");

    int configuredCount = 0;
    if (m_KbConfigScene != nullptr) {
        configuredCount = m_KbConfigScene->GetConfiguredPlayerCount();
    }

    const bool hasWarning = (m_PlayerCount > configuredCount);

    m_PlayerCountText->SetColor(hasWarning ? k_Red : k_Black);
    m_NoConfigText->SetVisible(hasWarning);
}