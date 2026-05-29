#include "scenes/ExitConfirmScene.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

ExitConfirmScene::ExitConfirmScene(SceneServices services)
    : Scene(services)
{
    const Util::Color black = Util::Color::FromRGB(0, 0, 0, 255);

    m_MenuFrame = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/Menu_Frame.png");
    m_MenuFrame->SetZIndex(10);
    m_MenuFrame->SetPosition({0.f, -105.f});

    m_ExitGameButton = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Button/ExitButton.png");
    m_ExitGameButton->SetZIndex(30);
    m_ExitGameButton->SetPosition({331.f, -14.f});

    m_ExitGame1Text = std::make_shared<GameText>("EXIT GAME ?", 65, black);
    m_ExitGame1Text->SetZIndex(25);
    m_ExitGame1Text->SetPosition({10.0f, -53.0f});

    m_YesText = std::make_shared<GameText>("YES", 65, black);
    m_YesText->SetZIndex(25);
    m_YesText->SetPosition({-110.0f, -183.0f});

    m_NoText = std::make_shared<GameText>("NO", 65, black);
    m_NoText->SetZIndex(25);
    m_NoText->SetPosition({130.0f, -183.0f});

    m_ChoiceFrame = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/Choice_Frame.png");
    m_ChoiceFrame->SetZIndex(20);

    m_ActorGroup
        .Add(m_MenuFrame)
        .Add(m_ExitGameButton)
        .Add(m_ExitGame1Text)
        .Add(m_YesText)
        .Add(m_NoText)
        .Add(m_ChoiceFrame);
}

void ExitConfirmScene::OnEnter() {
    LOG_INFO("ExitConfirmScene::OnEnter");

    m_ActorGroup.AddTo(m_Actors.Root());

    m_MenuFrame->SetScale({408.0f / 695.0f, 287.0f / 218.0f});
    m_MenuFrame->SetVisible(true);
    m_ExitGameButton->SetPosition({188.0f, 15.5f});
    m_ExitGameButton->SetVisible(true);

    m_IsYesSelected = true;
    UpdateChoiceFramePosition();
}

void ExitConfirmScene::OnExit() {
    LOG_INFO("ExitConfirmScene::OnExit");

    m_MenuFrame->SetScale({1.0f, 1.0f});
    m_ExitGameButton->SetPosition({331.0f, -14.0f});
    m_ActorGroup.RemoveFrom(m_Actors.Root());
}

void ExitConfirmScene::Update() {
    if (Util::Input::IsKeyDown(Util::Keycode::A) ||
        Util::Input::IsKeyDown(Util::Keycode::D)) {
        m_IsYesSelected = !m_IsYesSelected;
        m_Audio.PlaySe(SoundEffect::Button);
        UpdateChoiceFramePosition();
    }

    if (m_YesText->IsMouseHovering() && !m_IsYesSelected) {
        m_IsYesSelected = true;
        UpdateChoiceFramePosition();
    }
    if (m_NoText->IsMouseHovering() && m_IsYesSelected) {
        m_IsYesSelected = false;
        UpdateChoiceFramePosition();
    }

    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE) ||
        m_ExitGameButton->IsLeftClicked()) {
        LOG_INFO("ExitConfirmScene: cancelled, back to MenuScene");
        m_Audio.PlaySe(SoundEffect::Button);
        RequestSceneOp({SceneOpType::ClearToAndGoTo, SceneId::Menu});
        return;
    }

    const bool confirmByKeyboard = Util::Input::IsKeyDown(Util::Keycode::RETURN);
    const bool confirmByMouseYes = m_YesText->IsLeftClicked() ;
    const bool confirmByMouseNo  = m_NoText->IsLeftClicked() ;

    if ((confirmByKeyboard || confirmByMouseYes) && m_IsYesSelected) {
        LOG_INFO("ExitConfirmScene: YES confirmed -> ShouldQuit");
        m_Audio.PlaySe(SoundEffect::Button);
        m_Session.RequestQuit();
        return;
    }
    if ((confirmByKeyboard || confirmByMouseNo) && !m_IsYesSelected) {
        LOG_INFO("ExitConfirmScene: NO confirmed, back to MenuScene");
        m_Audio.PlaySe(SoundEffect::Button);
        RequestSceneOp({SceneOpType::ClearToAndGoTo, SceneId::Menu});
        return;
    }
}

void ExitConfirmScene::UpdateChoiceFramePosition() const {
    if (m_IsYesSelected) {
        m_ChoiceFrame->SetPosition({-120.0f, -180.0f});
    }
    else {
        m_ChoiceFrame->SetPosition({120.0f, -180.0f});
    }
}
