#include "LevelExitScene.hpp"

#include "Util/Input.hpp"
#include "Util/Keycode.hpp"

namespace {
constexpr float kChoiceFrameX = 0.0f;
}

LevelExitScene::LevelExitScene(SceneServices services)
    : Scene(services) {
    const Util::Color black = Util::Color::FromRGB(0, 0, 0, 255);

    m_DimBg = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/LevelExitSceneBkgrd.png");
    m_DimBg->SetZIndex(90.0f);
    m_DimBg->SetPosition({0.0f, 0.0f});

    m_ExitFrame = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/ExitFrame.png");
    m_ExitFrame->SetZIndex(95.0f);
    m_ExitFrame->SetPosition({0.0f, 0.0f});

    m_ExitButton = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Button/ExitButton.png");
    m_ExitButton->SetZIndex(95.0f);
    m_ExitButton->SetPosition({226.0f, 200.0f});

    m_BlueCatRunIcon = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Character/blue_cat/blue_cat_run_img.png");
    m_BlueCatRunIcon->SetZIndex(95.0f);
    m_BlueCatRunIcon->SetPosition({0.0f, 140.0f});
    m_BlueCatRunIcon->SetScale({1.2f, 1.2f});

    m_ChoiceFrame = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Background/Option_Choice_Frame.png");
    m_ChoiceFrame->SetZIndex(96.0f);
    m_ChoiceFrame->SetScale({1.2f, 1.3f});

    m_ReturnGameText = std::make_shared<GameText>("RETURN GAME", 60, black);
    m_RetryText = std::make_shared<GameText>("RETRY", 60, black);
    m_LevelSelectText = std::make_shared<GameText>("LEVEL SELECT", 60, black);
    m_TitleText = std::make_shared<GameText>("TITLE", 60, black);

    m_ReturnGameText->SetZIndex(97.0f);
    m_RetryText->SetZIndex(97.0f);
    m_LevelSelectText->SetZIndex(97.0f);
    m_TitleText->SetZIndex(97.0f);

    m_ReturnGameText->SetPosition({kTextX, kRowY[0]});
    m_RetryText->SetPosition({kTextX, kRowY[1]});
    m_LevelSelectText->SetPosition({kTextX, kRowY[2]});
    m_TitleText->SetPosition({kTextX, kRowY[3]});

}

void LevelExitScene::OnEnter() {
    m_Actors.Root().AddChild(m_DimBg);
    m_Actors.Root().AddChild(m_ExitFrame);
    m_Actors.Root().AddChild(m_ExitButton);
    m_Actors.Root().AddChild(m_BlueCatRunIcon);
    m_Actors.Root().AddChild(m_ChoiceFrame);
    m_Actors.Root().AddChild(m_ReturnGameText);
    m_Actors.Root().AddChild(m_RetryText);
    m_Actors.Root().AddChild(m_LevelSelectText);
    m_Actors.Root().AddChild(m_TitleText);

    m_SelectedRow = 0;
    m_WaitEscRelease = true;
    UpdateChoiceFrame();
}

void LevelExitScene::OnExit() {
    m_Actors.Root().RemoveChild(m_DimBg);
    m_Actors.Root().RemoveChild(m_ExitFrame);
    m_Actors.Root().RemoveChild(m_ExitButton);
    m_Actors.Root().RemoveChild(m_BlueCatRunIcon);
    m_Actors.Root().RemoveChild(m_ChoiceFrame);
    m_Actors.Root().RemoveChild(m_ReturnGameText);
    m_Actors.Root().RemoveChild(m_RetryText);
    m_Actors.Root().RemoveChild(m_LevelSelectText);
    m_Actors.Root().RemoveChild(m_TitleText);
}

SceneId LevelExitScene::Update() {
    using ip = Util::Input;
    using k = Util::Keycode;

    if (m_WaitEscRelease) {
        if (!ip::IsKeyPressed(k::ESCAPE)) {
            m_WaitEscRelease = false;
        }
    }

    if (ip::IsKeyDown(k::W) || ip::IsKeyDown(k::UP)) {
        m_SelectedRow = (m_SelectedRow + ROW_COUNT - 1) % ROW_COUNT;
        UpdateChoiceFrame();
    }
    else if (ip::IsKeyDown(k::S) || ip::IsKeyDown(k::DOWN)) {
        m_SelectedRow = (m_SelectedRow + 1) % ROW_COUNT;
        UpdateChoiceFrame();
    }

    if (m_ReturnGameText->IsMouseHovering() && m_SelectedRow != 0) {
        m_SelectedRow = 0;
        UpdateChoiceFrame();
    }
    else if (m_RetryText->IsMouseHovering() && m_SelectedRow != 1) {
        m_SelectedRow = 1;
        UpdateChoiceFrame();
    }
    else if (m_LevelSelectText->IsMouseHovering() && m_SelectedRow != 2) {
        m_SelectedRow = 2;
        UpdateChoiceFrame();
    }
    else if (m_TitleText->IsMouseHovering() && m_SelectedRow != 3) {
        m_SelectedRow = 3;
        UpdateChoiceFrame();
    }

    if (m_ExitButton->IsLeftClicked()) {
        RequestSceneOp({SceneOpType::PopOverlay, SceneId::None});
        return SceneId::None;
    }

    if (!m_WaitEscRelease && ip::IsKeyDown(k::ESCAPE)) {
        RequestSceneOp({SceneOpType::PopOverlay, SceneId::None});
        return SceneId::None;
    }

    bool confirmed = false;
    if (!m_WaitEscRelease && ip::IsKeyDown(k::RETURN)) {
        confirmed = true;
    }

    if (!confirmed) {
        if (m_ReturnGameText->IsLeftClicked()) {
            m_SelectedRow = 0;
            confirmed = true;
        }
        else if (m_RetryText->IsLeftClicked()) {
            m_SelectedRow = 1;
            confirmed = true;
        }
        else if (m_LevelSelectText->IsLeftClicked()) {
            m_SelectedRow = 2;
            confirmed = true;
        }
        else if (m_TitleText->IsLeftClicked()) {
            m_SelectedRow = 3;
            confirmed = true;
        }
    }

    if (confirmed) {
        ConfirmSelection();
    }

    return SceneId::None;
}

void LevelExitScene::UpdateChoiceFrame() const {
    m_ChoiceFrame->SetPosition({kChoiceFrameX, kRowY[static_cast<size_t>(m_SelectedRow)]});
}

void LevelExitScene::ConfirmSelection() {
    switch (m_SelectedRow) {
        case 0:
            RequestSceneOp({SceneOpType::PopOverlay, SceneId::None});
            break;
        case 1:
            RequestSceneOp({SceneOpType::RestartUnderlying, SceneId::None});
            break;
        case 2:
            RequestSceneOp({SceneOpType::ClearToAndGoTo, SceneId::LevelSelect});
            break;
        case 3:
            RequestSceneOp({SceneOpType::ClearToAndGoTo, SceneId::Title});
            break;
        default:
            break;
    }
}

