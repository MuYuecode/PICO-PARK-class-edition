//
// Created by cody2 on 2026/3/15.
//

#include "SaveManager.hpp"
#include "BGMPlayer.hpp"
#include "AppUtil.hpp"

#include "Menuscene.hpp"
#include "OptionMenuScene.hpp"
#include "KeyboardConfigScene.hpp"

#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

const std::vector<std::string> OptionMenuScene::s_BgColorOptions = {
    "WHITE",
    "CREAM",
    "DARK",
};

const std::vector<std::string> OptionMenuScene::s_BgColorPaths = {
    GA_RESOURCE_DIR "/Image/Background/white_background.png",
    GA_RESOURCE_DIR "/Image/Background/cream_background.png",
    GA_RESOURCE_DIR "/Image/Background/dark_background.png",
};

OptionMenuScene::OptionMenuScene(GameContext& ctx,
                                 MenuScene* menuScene,
                                 std::shared_ptr<Character> exitGameButton)
    : Scene(ctx)
    , m_ExitGameButton(std::move(exitGameButton))
    , m_MenuScene(menuScene)
{
    const Util::Color black = Util::Color::FromRGB(0, 0, 0, 255);

    m_OptionMenuFrame = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/Option_Menu_Frame.png");
    m_OptionMenuFrame->SetZIndex(25);

    m_ChoiceFrame = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/Option_Choice_Frame.png");
    m_ChoiceFrame->SetZIndex(30);

    m_TitleText = std::make_shared<GameText>("OPTION", 65, black);
    m_TitleText->SetZIndex(35);
    m_TitleText->SetPosition({0.0f, 228.0f});

    m_KbConfigLabel = std::make_shared<GameText>("KEYBOARD CONFIG", 45, black);
    m_KbConfigLabel->SetZIndex(35);
    m_KbConfigLabel->SetPosition({AppUtil::AlignLeft(*m_KbConfigLabel, COL_LABEL_X), ROW_Y_KB});

    m_KbConfigOpen = std::make_shared<GameText>("OPEN", 45, black);
    m_KbConfigOpen->SetZIndex(35);
    m_KbConfigOpen->SetPosition({COL_VALUE_X - 5.0f, ROW_Y_KB});

    m_BgColorLabel = std::make_shared<GameText>("BG COLOR", 45, black);
    m_BgColorLabel->SetZIndex(35);
    m_BgColorLabel->SetPosition({AppUtil::AlignLeft(*m_BgColorLabel, COL_LABEL_X), ROW_Y_BG});

    m_BgColorLeftBtn = std::make_shared<UI_Triangle_Button>(
        GA_RESOURCE_DIR "/Image/Button/Left_Tri_Button.png",
        GA_RESOURCE_DIR "/Image/Button/Left_Tri_Button_Full.png");
    m_BgColorLeftBtn->SetZIndex(35);
    m_BgColorLeftBtn->SetPosition({COL_LEFT_BTN_X, ROW_Y_BG});

    m_BgColorValue = std::make_shared<GameText>("WHITE", 45, black);
    m_BgColorValue->SetZIndex(35);
    m_BgColorValue->SetPosition({COL_VALUE_X - 5.0f, ROW_Y_BG});

    m_BgColorRightBtn = std::make_shared<UI_Triangle_Button>(
        GA_RESOURCE_DIR "/Image/Button/Right_Tri_Button.png",
        GA_RESOURCE_DIR "/Image/Button/Right_Tri_Button_Full.png");
    m_BgColorRightBtn->SetZIndex(35);
    m_BgColorRightBtn->SetPosition({COL_RIGHT_BTN_X, ROW_Y_BG});

    m_BgmVolumeLabel = std::make_shared<GameText>("BGM VOLUME", 45, black);
    m_BgmVolumeLabel->SetZIndex(35);
    m_BgmVolumeLabel->SetPosition({AppUtil::AlignLeft(*m_BgmVolumeLabel, COL_LABEL_X), ROW_Y_BGM});

    m_BgmVolumeLeftBtn = std::make_shared<UI_Triangle_Button>(
        GA_RESOURCE_DIR "/Image/Button/Left_Tri_Button.png",
        GA_RESOURCE_DIR "/Image/Button/Left_Tri_Button_Full.png");
    m_BgmVolumeLeftBtn->SetZIndex(35);
    m_BgmVolumeLeftBtn->SetPosition({COL_LEFT_BTN_X, ROW_Y_BGM});

    m_BgmVolumeValue = std::make_shared<GameText>("10", 45, black);
    m_BgmVolumeValue->SetZIndex(35);
    m_BgmVolumeValue->SetPosition({COL_VALUE_X - 5.0f, ROW_Y_BGM});

    m_BgmVolumeRightBtn = std::make_shared<UI_Triangle_Button>(
        GA_RESOURCE_DIR "/Image/Button/Right_Tri_Button.png",
        GA_RESOURCE_DIR "/Image/Button/Right_Tri_Button_Full.png");
    m_BgmVolumeRightBtn->SetZIndex(35);
    m_BgmVolumeRightBtn->SetPosition({COL_RIGHT_BTN_X, ROW_Y_BGM});

    m_SeVolumeLabel = std::make_shared<GameText>("SE VOLUME", 45, black);
    m_SeVolumeLabel->SetZIndex(35);
    m_SeVolumeLabel->SetPosition({AppUtil::AlignLeft(*m_SeVolumeLabel, COL_LABEL_X), ROW_Y_SE});

    m_SeVolumeLeftBtn = std::make_shared<UI_Triangle_Button>(
        GA_RESOURCE_DIR "/Image/Button/Left_Tri_Button.png",
        GA_RESOURCE_DIR "/Image/Button/Left_Tri_Button_Full.png");
    m_SeVolumeLeftBtn->SetZIndex(35);
    m_SeVolumeLeftBtn->SetPosition({COL_LEFT_BTN_X, ROW_Y_SE});

    m_SeVolumeValue = std::make_shared<GameText>("10", 45, black);
    m_SeVolumeValue->SetZIndex(35);
    m_SeVolumeValue->SetPosition({COL_VALUE_X - 5.0f, ROW_Y_SE});

    m_SeVolumeRightBtn = std::make_shared<UI_Triangle_Button>(
        GA_RESOURCE_DIR "/Image/Button/Right_Tri_Button.png",
        GA_RESOURCE_DIR "/Image/Button/Right_Tri_Button_Full.png");
    m_SeVolumeRightBtn->SetZIndex(35);
    m_SeVolumeRightBtn->SetPosition({COL_RIGHT_BTN_X, ROW_Y_SE});

    m_DispNumberLabel = std::make_shared<GameText>("DISP NUMBER", 45, black);
    m_DispNumberLabel->SetZIndex(35);
    m_DispNumberLabel->SetPosition({AppUtil::AlignLeft(*m_DispNumberLabel, COL_LABEL_X), ROW_Y_DISP});

    m_DispNumberLeftBtn = std::make_shared<UI_Triangle_Button>(
        GA_RESOURCE_DIR "/Image/Button/Left_Tri_Button.png",
        GA_RESOURCE_DIR "/Image/Button/Left_Tri_Button_Full.png");
    m_DispNumberLeftBtn->SetZIndex(35);
    m_DispNumberLeftBtn->SetPosition({COL_LEFT_BTN_X, ROW_Y_DISP});

    m_DispNumberValue = std::make_shared<GameText>("OFF", 45, black);
    m_DispNumberValue->SetZIndex(35);
    m_DispNumberValue->SetPosition({COL_VALUE_X - 5.0f, ROW_Y_DISP});

    m_DispNumberRightBtn = std::make_shared<UI_Triangle_Button>(
        GA_RESOURCE_DIR "/Image/Button/Right_Tri_Button.png",
        GA_RESOURCE_DIR "/Image/Button/Right_Tri_Button_Full.png");
    m_DispNumberRightBtn->SetZIndex(35);
    m_DispNumberRightBtn->SetPosition({COL_RIGHT_BTN_X, ROW_Y_DISP});

    m_OkText = std::make_shared<GameText>("OK", 45, black);
    m_OkText->SetZIndex(35);
    m_OkText->SetPosition({COL_OK_X, ROW_Y_BTN});

    m_CancelText = std::make_shared<GameText>("CANCEL", 45, black);
    m_CancelText->SetZIndex(35);
    m_CancelText->SetPosition({COL_CANCEL_X, ROW_Y_BTN});

    OptionSettingsData saved;
    if (SaveManager::LoadOptionSettings(saved)) {
        m_Applied.bgColorIndex = saved.bgColorIndex;
        m_Applied.bgmVolume    = saved.bgmVolume;
        m_Applied.seVolume     = saved.seVolume;
        m_Applied.dispNumber   = saved.dispNumber;
        LOG_INFO("OptionMenuScene: loaded settings from file");
    }

    // 確保建構選單時，立刻將讀取到(或預設)的設定套用到全局的 Context 中
    ctx.Background->SetImage(s_BgColorPaths[m_Applied.bgColorIndex]);
    if (ctx.BGMPlayer) {
        ctx.BGMPlayer->SetVolume(m_Applied.bgmVolume * 6);
    }
}

void OptionMenuScene::OnEnter() {
    LOG_INFO("OptionMenuScene::OnEnter");

    m_Ctx.Root.AddChild(m_ExitGameButton);
    m_Ctx.Root.AddChild(m_OptionMenuFrame);
    m_Ctx.Root.AddChild(m_ChoiceFrame);
    m_Ctx.Root.AddChild(m_TitleText);
    m_Ctx.Root.AddChild(m_KbConfigLabel);
    m_Ctx.Root.AddChild(m_KbConfigOpen);
    m_Ctx.Root.AddChild(m_BgColorLabel);
    m_Ctx.Root.AddChild(m_BgColorLeftBtn);
    m_Ctx.Root.AddChild(m_BgColorValue);
    m_Ctx.Root.AddChild(m_BgColorRightBtn);
    m_Ctx.Root.AddChild(m_BgmVolumeLabel);
    m_Ctx.Root.AddChild(m_BgmVolumeLeftBtn);
    m_Ctx.Root.AddChild(m_BgmVolumeValue);
    m_Ctx.Root.AddChild(m_BgmVolumeRightBtn);
    m_Ctx.Root.AddChild(m_SeVolumeLabel);
    m_Ctx.Root.AddChild(m_SeVolumeLeftBtn);
    m_Ctx.Root.AddChild(m_SeVolumeValue);
    m_Ctx.Root.AddChild(m_SeVolumeRightBtn);
    m_Ctx.Root.AddChild(m_DispNumberLabel);
    m_Ctx.Root.AddChild(m_DispNumberLeftBtn);
    m_Ctx.Root.AddChild(m_DispNumberValue);
    m_Ctx.Root.AddChild(m_DispNumberRightBtn);
    m_Ctx.Root.AddChild(m_OkText);
    m_Ctx.Root.AddChild(m_CancelText);

    m_OptionMenuFrame->SetPosition({0.0f, -5.0f});
    m_ExitGameButton->SetPosition({399.2f, 285.1f});

    m_BgColorLeftBtn->ResetState();
    m_BgColorRightBtn->ResetState();
    m_BgmVolumeLeftBtn->ResetState();
    m_BgmVolumeRightBtn->ResetState();
    m_SeVolumeLeftBtn->ResetState();
    m_SeVolumeRightBtn->ResetState();
    m_DispNumberLeftBtn->ResetState();
    m_DispNumberRightBtn->ResetState();

    m_Pending = m_Applied;

    m_SelectedRow = 0;
    UpdateValueTexts();
    UpdateChoiceFrame();
}

void OptionMenuScene::OnExit() {
    LOG_INFO("OptionMenuScene::OnExit");

    m_Ctx.Root.RemoveChild(m_OptionMenuFrame);
    m_Ctx.Root.RemoveChild(m_TitleText);
    m_Ctx.Root.RemoveChild(m_KbConfigLabel);
    m_Ctx.Root.RemoveChild(m_KbConfigOpen);
    m_Ctx.Root.RemoveChild(m_BgColorLabel);
    m_Ctx.Root.RemoveChild(m_BgColorLeftBtn);
    m_Ctx.Root.RemoveChild(m_BgColorValue);
    m_Ctx.Root.RemoveChild(m_BgColorRightBtn);
    m_Ctx.Root.RemoveChild(m_BgmVolumeLabel);
    m_Ctx.Root.RemoveChild(m_BgmVolumeLeftBtn);
    m_Ctx.Root.RemoveChild(m_BgmVolumeValue);
    m_Ctx.Root.RemoveChild(m_BgmVolumeRightBtn);
    m_Ctx.Root.RemoveChild(m_SeVolumeLabel);
    m_Ctx.Root.RemoveChild(m_SeVolumeLeftBtn);
    m_Ctx.Root.RemoveChild(m_SeVolumeValue);
    m_Ctx.Root.RemoveChild(m_SeVolumeRightBtn);
    m_Ctx.Root.RemoveChild(m_DispNumberLabel);
    m_Ctx.Root.RemoveChild(m_DispNumberLeftBtn);
    m_Ctx.Root.RemoveChild(m_DispNumberValue);
    m_Ctx.Root.RemoveChild(m_DispNumberRightBtn);
    m_Ctx.Root.RemoveChild(m_OkText);
    m_Ctx.Root.RemoveChild(m_CancelText);
    m_Ctx.Root.RemoveChild(m_ChoiceFrame);

    m_ExitGameButton->SetPosition({331.0f, -14.0f});
    m_Ctx.Root.RemoveChild(m_ExitGameButton);
}

Scene* OptionMenuScene::Update() {
    m_BgColorLeftBtn->UpdateButton();
    m_BgColorRightBtn->UpdateButton();
    m_BgmVolumeLeftBtn->UpdateButton();
    m_BgmVolumeRightBtn->UpdateButton();
    m_SeVolumeLeftBtn->UpdateButton();
    m_SeVolumeRightBtn->UpdateButton();
    m_DispNumberLeftBtn->UpdateButton();
    m_DispNumberRightBtn->UpdateButton();

    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE) ||
        m_ExitGameButton->IsLeftClicked()              ||
        m_CancelText->IsLeftClicked())
    {
        m_Ctx.BGMPlayer->SetVolume(m_Applied.bgmVolume * 6);
        m_Ctx.Background->SetImage(s_BgColorPaths[m_Applied.bgColorIndex]);
        LOG_INFO("OptionMenuScene: cancelled => MenuScene");
        return m_MenuScene;
    }

    if (m_KbConfigOpen->IsLeftClicked()) {
        LOG_INFO("OptionMenuScene: KEYBOARD CONFIG OPEN (mouse) => KeyboardConfigScene");
        return m_KeyboardConfigScene;
    }
    if (m_OkText->IsLeftClicked()) {
        m_Applied = m_Pending;
        m_Ctx.BGMPlayer->SetVolume(m_Applied.bgmVolume * 6);
        m_Ctx.Background->SetImage(s_BgColorPaths[m_Applied.bgColorIndex]);

        {
            OptionSettingsData toSave{
                m_Applied.bgColorIndex,
                m_Applied.bgmVolume,
                m_Applied.seVolume,
                m_Applied.dispNumber
            };
            SaveManager::SaveOptionSettings(toSave);
        }

        LOG_INFO("OptionMenuScene: OK (mouse) => MenuScene");
        return m_MenuScene;
    }

    if (Util::Input::IsKeyDown(Util::Keycode::W) || Util::Input::IsKeyDown(Util::Keycode::UP)) {
        DecrementRow();
    }
    else if (Util::Input::IsKeyDown(Util::Keycode::S) || Util::Input::IsKeyDown(Util::Keycode::DOWN)) {
        IncrementRow();
    }

    auto hoverTo = [&](const std::shared_ptr<GameText>& text, int row) {
        if (text->IsMouseHovering() && m_SelectedRow != row) {
            m_SelectedRow = row;
            UpdateChoiceFrame();
        }
    };
    hoverTo(m_KbConfigOpen,    0);
    hoverTo(m_BgColorValue,    1);
    hoverTo(m_BgmVolumeValue,  2);
    hoverTo(m_SeVolumeValue,   3);
    hoverTo(m_DispNumberValue, 4);
    hoverTo(m_OkText,          5);
    hoverTo(m_CancelText,      6);

    if (m_BgColorLeftBtn->IsLeftClicked()) {
        m_SelectedRow = 1; UpdateChoiceFrame(); AdjustLeft(1);
    } else if (m_BgColorRightBtn->IsLeftClicked()) {
        m_SelectedRow = 1; UpdateChoiceFrame(); AdjustRight(1);
    }
    else if (m_BgmVolumeLeftBtn->IsLeftClicked()) {
        m_SelectedRow = 2; UpdateChoiceFrame(); AdjustLeft(2);
    } else if (m_BgmVolumeRightBtn->IsLeftClicked()) {
        m_SelectedRow = 2; UpdateChoiceFrame(); AdjustRight(2);
    }
    else if (m_SeVolumeLeftBtn->IsLeftClicked()) {
        m_SelectedRow = 3; UpdateChoiceFrame(); AdjustLeft(3);
    } else if (m_SeVolumeRightBtn->IsLeftClicked()) {
        m_SelectedRow = 3; UpdateChoiceFrame(); AdjustRight(3);
    }
    else if (m_DispNumberLeftBtn->IsLeftClicked()) {
        m_SelectedRow = 4; UpdateChoiceFrame(); AdjustLeft(4);
    } else if (m_DispNumberRightBtn->IsLeftClicked()) {
        m_SelectedRow = 4; UpdateChoiceFrame(); AdjustRight(4);
    }

    if (Util::Input::IsKeyDown(Util::Keycode::A) || Util::Input::IsKeyDown(Util::Keycode::LEFT)) {
        AdjustLeft(m_SelectedRow);
    }
    else if (Util::Input::IsKeyDown(Util::Keycode::D) || Util::Input::IsKeyDown(Util::Keycode::RIGHT)) {
        AdjustRight(m_SelectedRow);
    }

    if (Util::Input::IsKeyDown(Util::Keycode::RETURN)) {
        switch (m_SelectedRow) {
        case 0:
            LOG_INFO("OptionMenuScene: KEYBOARD CONFIG OPEN (keyboard) => KeyboardConfigScene");
            return m_KeyboardConfigScene;
        case 5:
            m_Applied = m_Pending;
            m_Ctx.BGMPlayer->SetVolume(m_Applied.bgmVolume * 6);
            m_Ctx.Background->SetImage(s_BgColorPaths[m_Applied.bgColorIndex]);

            {
            OptionSettingsData toSave{
                m_Applied.bgColorIndex,
                m_Applied.bgmVolume,
                m_Applied.seVolume,
                m_Applied.dispNumber
            };
            SaveManager::SaveOptionSettings(toSave);
            }

            LOG_INFO("OptionMenuScene: OK (keyboard) => MenuScene");
            return m_MenuScene;
        case 6:
            m_Ctx.BGMPlayer->SetVolume(m_Applied.bgmVolume * 6);
            m_Ctx.Background->SetImage(s_BgColorPaths[m_Applied.bgColorIndex]);
            LOG_INFO("OptionMenuScene: CANCEL (keyboard) => MenuScene");
            return m_MenuScene;
        default:
            break;
        }
    }

    return nullptr;
}

void OptionMenuScene::DecrementRow() {
    m_SelectedRow = (m_SelectedRow + ROW_COUNT - 1) % ROW_COUNT;
    UpdateChoiceFrame();
}

void OptionMenuScene::IncrementRow() {
    m_SelectedRow = (m_SelectedRow + 1) % ROW_COUNT;
    UpdateChoiceFrame();
}

void OptionMenuScene::SwapOkCancel() {
    m_SelectedRow = (m_SelectedRow == 5) ? 6 : 5;
    UpdateChoiceFrame();
}

void OptionMenuScene::AdjustLeft(int row) {
    switch (row) {
    case 1:
        m_Pending.bgColorIndex = (m_Pending.bgColorIndex
                          + static_cast<int>(s_BgColorOptions.size()) - 1)
                         % static_cast<int>(s_BgColorOptions.size());
        m_BgColorLeftBtn->Press(75.0f);
        m_Ctx.Background->SetImage(s_BgColorPaths[m_Pending.bgColorIndex]);
        break;
    case 2:
        if (m_Pending.bgmVolume > 0) {
            --m_Pending.bgmVolume;
            m_BgmVolumeLeftBtn->Press(75.0f);
            m_Ctx.BGMPlayer->SetVolume(m_Pending.bgmVolume * 6);
        }
        break;
    case 3:
        if (m_Pending.seVolume > 0) {
            --m_Pending.seVolume;
            m_SeVolumeLeftBtn->Press(75.0f);
        }
        break;
    case 4:
        if (m_Pending.dispNumber) {
            m_Pending.dispNumber = false;
            m_DispNumberLeftBtn->Press(75.0f);
        }
        break;
    case 5:
    case 6:
        SwapOkCancel();   // ← 兩個 case 共用同一段邏輯
        break;
    default:
        break;
    }
    UpdateValueTexts();
}

void OptionMenuScene::AdjustRight(int row) {
    switch (row) {
    case 1:
        m_Pending.bgColorIndex = (m_Pending.bgColorIndex + 1)
                         % static_cast<int>(s_BgColorOptions.size());
        m_BgColorRightBtn->Press(75.0f);
        m_Ctx.Background->SetImage(s_BgColorPaths[m_Pending.bgColorIndex]);
        break;
    case 2:
        if (m_Pending.bgmVolume < 20) {
            ++m_Pending.bgmVolume;
            m_BgmVolumeRightBtn->Press(75.0f);
            m_Ctx.BGMPlayer->SetVolume(m_Pending.bgmVolume * 6);
        }
        break;
    case 3:
        if (m_Pending.seVolume < 20) {
            ++m_Pending.seVolume;
            m_SeVolumeRightBtn->Press(75.0f);
        }
        break;
    case 4:
        if (!m_Pending.dispNumber) {
            m_Pending.dispNumber = true;
            m_DispNumberRightBtn->Press(75.0f);
        }
        break;
    case 5:
    case 6:
        SwapOkCancel();   // ← 兩個 case 共用同一段邏輯
        break;
    default:
        break;
    }
    UpdateValueTexts();
}

void OptionMenuScene::UpdateValueTexts() const {
    m_BgColorValue->SetText(s_BgColorOptions[m_Pending.bgColorIndex]);
    m_BgmVolumeValue->SetText(std::to_string(m_Pending.bgmVolume));
    m_SeVolumeValue->SetText(std::to_string(m_Pending.seVolume));
    m_DispNumberValue->SetText(m_Pending.dispNumber ? "ON" : "OFF");
}

void OptionMenuScene::UpdateChoiceFrame() const {
    switch (m_SelectedRow) {
    case 0:
        m_ChoiceFrame->SetScale({277.0f/367.0f, 1.1f});
        m_ChoiceFrame->SetPosition({COL_VALUE_X - 15.0f, ROW_Y_KB});
        break;
    case 1:
        m_ChoiceFrame->SetScale({187.0f/367.0f, 1.1f});
        m_ChoiceFrame->SetPosition({COL_VALUE_X - 13.5f, ROW_Y_BG});
        break;
    case 2:
        m_ChoiceFrame->SetScale({187.0f/367.0f, 1.1f});
        m_ChoiceFrame->SetPosition({COL_VALUE_X - 13.5f, ROW_Y_BGM});
        break;
    case 3:
        m_ChoiceFrame->SetScale({187.0f/367.0f, 1.1f});
        m_ChoiceFrame->SetPosition({COL_VALUE_X - 13.5f, ROW_Y_SE});
        break;
    case 4:
        m_ChoiceFrame->SetScale({187.0f/367.0f, 1.1f});
        m_ChoiceFrame->SetPosition({COL_VALUE_X - 13.5f, ROW_Y_DISP});
        break;
    case 5:
        m_ChoiceFrame->SetScale({207.0f/367.0f, 1.1f});
        m_ChoiceFrame->SetPosition({COL_OK_X - 15.0f, ROW_Y_BTN});
        break;
    case 6:
        m_ChoiceFrame->SetScale({207.0f/367.0f, 1.1f});
        m_ChoiceFrame->SetPosition({COL_CANCEL_X - 15.0f, ROW_Y_BTN});
        break;
    default:
        break;
    }
}