//
// Created by cody2 on 2026/3/17.
//

#include "scenes/play/LocalPlayScene.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

const Util::Color LocalPlayScene::k_Black = Util::Color::FromRGB(0,   0,   0,   255);
const Util::Color LocalPlayScene::k_Red   = Util::Color::FromRGB(220, 50,  50,  255);

using ip = Util::Input ;
using k  = Util::Keycode ;

LocalPlayScene::LocalPlayScene(SceneServices services)
    : Scene(services)
{
    m_MenuFrame = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/Menu_Frame.png");
    m_MenuFrame->SetZIndex(10);
    m_MenuFrame->SetPosition({0.0f, -105.0f});

    m_ExitGameButton = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Button/ExitButton.png");
    m_ExitGameButton->SetZIndex(30);
    m_ExitGameButton->SetPosition({331.0f, -14.0f});

    m_BlueCatRunImg = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Character/blue_cat/blue_cat_run_img.png");
    m_BlueCatRunImg->SetZIndex(10);
    m_BlueCatRunImg->SetScale({1.2f, 1.2f});
    m_BlueCatRunImg->SetPosition({0.0f, -74.0f});

    m_LeftTriButton = std::make_shared<UITriangleButton>(
        GA_RESOURCE_DIR "/Image/Button/Left_Tri_Button.png",
        GA_RESOURCE_DIR "/Image/Button/Left_Tri_Button_Full.png");
    m_LeftTriButton->SetZIndex(15);
    m_LeftTriButton->SetPosition({-305.0f, -153.0f});

    m_RightTriButton = std::make_shared<UITriangleButton>(
        GA_RESOURCE_DIR "/Image/Button/Right_Tri_Button.png",
        GA_RESOURCE_DIR "/Image/Button/Right_Tri_Button_Full.png");
    m_RightTriButton->SetZIndex(15);
    m_RightTriButton->SetPosition({305.0f, -153.0f});

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

    m_Actors.Root().AddChild(m_MenuFrame);
    m_Actors.Root().AddChild(m_ExitGameButton);
    m_Actors.Root().AddChild(m_LeftTriButton);
    m_Actors.Root().AddChild(m_RightTriButton);
    m_Actors.Root().AddChild(m_BlueCatRunImg);

    m_Actors.Root().AddChild(m_PlayerCountText);
    m_Actors.Root().AddChild(m_NoConfigText);

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

    m_Actors.Root().RemoveChild(m_PlayerCountText);
    m_Actors.Root().RemoveChild(m_NoConfigText);

    m_MenuFrame->SetScale({1.0f, 1.0f});
    m_ExitGameButton->SetPosition({331.0f, -14.0f});

    m_Actors.Root().RemoveChild(m_LeftTriButton);
    m_Actors.Root().RemoveChild(m_RightTriButton);
    m_Actors.Root().RemoveChild(m_BlueCatRunImg);
    m_Actors.Root().RemoveChild(m_ExitGameButton);
    m_Actors.Root().RemoveChild(m_MenuFrame);
}

void LocalPlayScene::Update() {
    m_LeftTriButton->UpdateButton();
    m_RightTriButton->UpdateButton();

    if ( ip::IsKeyDown(k::ESCAPE) ||
        m_ExitGameButton->IsLeftClicked()) {
        LOG_INFO("LocalPlayScene: back to MenuScene");
        RequestSceneOp({SceneOpType::ClearToAndGoTo, SceneId::Menu});
        return;
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
        const int configuredCount = m_Session.GetConfiguredPlayerCount();

        if (m_PlayerCount <= configuredCount) {
            m_Session.SetSelectedPlayerCount(m_PlayerCount);
            LOG_INFO("LocalPlayScene: ENTER confirmed with {} players", m_PlayerCount);
            RequestSceneOp({SceneOpType::ClearToAndGoTo, SceneId::LocalPlayGame});
            return;
        }

        LOG_INFO("LocalPlayScene: ENTER blocked, selected={} configured={}",
                 m_PlayerCount, configuredCount);
    }
}

void LocalPlayScene::UpdateDisplay() const {
    m_PlayerCountText->SetText(std::to_string(m_PlayerCount) + "PLAYER GAME");

    const int configuredCount = m_Session.GetConfiguredPlayerCount();

    const bool hasWarning = (m_PlayerCount > configuredCount);

    m_PlayerCountText->SetColor(hasWarning ? k_Red : k_Black);
    m_NoConfigText->SetVisible(hasWarning);
}
