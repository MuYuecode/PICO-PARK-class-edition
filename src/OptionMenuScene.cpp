//
// Created by cody2 on 2026/3/15.
//

#include "Menuscene.hpp"
#include "OptionMenuScene.hpp"
#include "KeyboardConfigScene.hpp"
#include "BGMPlayer.hpp"
#include "AppUtil.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

// ────────────────────────────────────────────────────────────────────────────
// 靜態資料：BG COLOR 選項清單（新增顏色只需在此處追加字串）
// ────────────────────────────────────────────────────────────────────────────
const std::vector<std::string> OptionMenuScene::s_BgColorOptions = {
    "WHITE",
    // "BLACK",   ← 示例：未來可新增其他顏色
};

// ────────────────────────────────────────────────────────────────────────────
// 建構子：建立所有 UI 物件（尚未加入渲染樹）
// ────────────────────────────────────────────────────────────────────────────
OptionMenuScene::OptionMenuScene(GameContext& ctx,
                                 MenuScene* menuScene,
                                 std::shared_ptr<Character> exitGameButton)
    : Scene(ctx)
    , m_ExitGameButton(std::move(exitGameButton))
    , m_MenuScene(menuScene)
{
    const Util::Color black = Util::Color::FromRGB(0, 0, 0, 255);

    // ── 大框架 ─────────────────────────────────────────────────────────────
    m_OptionMenuFrame = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/Option_Menu_Frame.png");
    m_OptionMenuFrame->SetZIndex(25);

    // ── 選擇框 ──────────────────────────────────────────────────────────────
    m_ChoiceFrame = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/Option_Choice_Frame.png") ;
    m_ChoiceFrame->SetZIndex(30);

    // ── 標題 "OPTION" ────────────────────────────────────────────────────────
    m_TitleText = std::make_shared<GameText>("OPTION", 65, black);
    m_TitleText->SetZIndex(35);
    m_TitleText->SetPosition({0.0f, 228.0f});

    // ── KEYBOARD CONFIG 列 ──────────────────────────────────────────────────
    m_KbConfigLabel = std::make_shared<GameText>("KEYBOARD CONFIG", 45, black);
    m_KbConfigLabel->SetZIndex(35);
    m_KbConfigLabel->SetPosition({AppUtil::AlignLeft(*m_KbConfigLabel, COL_LABEL_X), ROW_Y_KB});

    m_KbConfigOpen = std::make_shared<GameText>("OPEN", 45, black);
    m_KbConfigOpen->SetZIndex(35);
    m_KbConfigOpen->SetPosition({COL_VALUE_X-5.0f, ROW_Y_KB});

    // ── BG COLOR 列 ─────────────────────────────────────────────────────────
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
    m_BgColorValue->SetPosition({COL_VALUE_X-5.0f, ROW_Y_BG});

    m_BgColorRightBtn = std::make_shared<UI_Triangle_Button>(
        GA_RESOURCE_DIR "/Image/Button/Right_Tri_Button.png",
        GA_RESOURCE_DIR "/Image/Button/Right_Tri_Button_Full.png");
    m_BgColorRightBtn->SetZIndex(35);
    m_BgColorRightBtn->SetPosition({COL_RIGHT_BTN_X, ROW_Y_BG});

    // ── BGM VOLUME 列 ───────────────────────────────────────────────────────
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
    m_BgmVolumeValue->SetPosition({COL_VALUE_X-5.0f, ROW_Y_BGM});

    m_BgmVolumeRightBtn = std::make_shared<UI_Triangle_Button>(
        GA_RESOURCE_DIR "/Image/Button/Right_Tri_Button.png",
        GA_RESOURCE_DIR "/Image/Button/Right_Tri_Button_Full.png");
    m_BgmVolumeRightBtn->SetZIndex(35);
    m_BgmVolumeRightBtn->SetPosition({COL_RIGHT_BTN_X, ROW_Y_BGM});

    // ── SE VOLUME 列 ────────────────────────────────────────────────────────
    m_SeVolumeLabel = std::make_shared<GameText>("SE VOLUME", 45, black);
    m_SeVolumeLabel->SetZIndex(35);
    m_SeVolumeLabel->SetPosition({AppUtil::AlignLeft(*m_SeVolumeLabel,COL_LABEL_X), ROW_Y_SE});

    m_SeVolumeLeftBtn = std::make_shared<UI_Triangle_Button>(
        GA_RESOURCE_DIR "/Image/Button/Left_Tri_Button.png",
        GA_RESOURCE_DIR "/Image/Button/Left_Tri_Button_Full.png");
    m_SeVolumeLeftBtn->SetZIndex(35);
    m_SeVolumeLeftBtn->SetPosition({COL_LEFT_BTN_X, ROW_Y_SE});

    m_SeVolumeValue = std::make_shared<GameText>("10", 45, black);
    m_SeVolumeValue->SetZIndex(35);
    m_SeVolumeValue->SetPosition({COL_VALUE_X-5.0f, ROW_Y_SE});

    m_SeVolumeRightBtn = std::make_shared<UI_Triangle_Button>(
        GA_RESOURCE_DIR "/Image/Button/Right_Tri_Button.png",
        GA_RESOURCE_DIR "/Image/Button/Right_Tri_Button_Full.png");
    m_SeVolumeRightBtn->SetZIndex(35);
    m_SeVolumeRightBtn->SetPosition({COL_RIGHT_BTN_X, ROW_Y_SE});

    // ── DISP NUMBER 列 ──────────────────────────────────────────────────────
    m_DispNumberLabel = std::make_shared<GameText>("DISP NUMBER", 45, black);
    m_DispNumberLabel->SetZIndex(35);
    m_DispNumberLabel->SetPosition({AppUtil::AlignLeft(*m_DispNumberLabel,COL_LABEL_X), ROW_Y_DISP});

    m_DispNumberLeftBtn = std::make_shared<UI_Triangle_Button>(
        GA_RESOURCE_DIR "/Image/Button/Left_Tri_Button.png",
        GA_RESOURCE_DIR "/Image/Button/Left_Tri_Button_Full.png");
    m_DispNumberLeftBtn->SetZIndex(35);
    m_DispNumberLeftBtn->SetPosition({COL_LEFT_BTN_X, ROW_Y_DISP});

    m_DispNumberValue = std::make_shared<GameText>("OFF", 45, black);
    m_DispNumberValue->SetZIndex(35);
    m_DispNumberValue->SetPosition({COL_VALUE_X-5.0f, ROW_Y_DISP});

    m_DispNumberRightBtn = std::make_shared<UI_Triangle_Button>(
        GA_RESOURCE_DIR "/Image/Button/Right_Tri_Button.png",
        GA_RESOURCE_DIR "/Image/Button/Right_Tri_Button_Full.png");
    m_DispNumberRightBtn->SetZIndex(35);
    m_DispNumberRightBtn->SetPosition({COL_RIGHT_BTN_X, ROW_Y_DISP});

    // ── 底部按鈕 ────────────────────────────────────────────────────────────
    m_OkText = std::make_shared<GameText>("OK", 45, black);
    m_OkText->SetZIndex(35);
    m_OkText->SetPosition({COL_OK_X, ROW_Y_BTN});

    m_CancelText = std::make_shared<GameText>("CANCEL", 45, black);
    m_CancelText->SetZIndex(35);
    m_CancelText->SetPosition({COL_CANCEL_X, ROW_Y_BTN});
}

// ────────────────────────────────────────────────────────────────────────────
// OnEnter
// ────────────────────────────────────────────────────────────────────────────
void OptionMenuScene::OnEnter() {
    LOG_INFO("OptionMenuScene::OnEnter");

    // 借用的共用物件
    m_Ctx.Root.AddChild(m_ExitGameButton);

    // 大框架
    m_Ctx.Root.AddChild(m_OptionMenuFrame);

    // 選擇框
    m_Ctx.Root.AddChild(m_ChoiceFrame);

    // 標題
    m_Ctx.Root.AddChild(m_TitleText);

    // KEYBOARD CONFIG 列
    m_Ctx.Root.AddChild(m_KbConfigLabel);
    m_Ctx.Root.AddChild(m_KbConfigOpen);

    // BG COLOR 列
    m_Ctx.Root.AddChild(m_BgColorLabel);
    m_Ctx.Root.AddChild(m_BgColorLeftBtn);
    m_Ctx.Root.AddChild(m_BgColorValue);
    m_Ctx.Root.AddChild(m_BgColorRightBtn);

    // BGM VOLUME 列
    m_Ctx.Root.AddChild(m_BgmVolumeLabel);
    m_Ctx.Root.AddChild(m_BgmVolumeLeftBtn);
    m_Ctx.Root.AddChild(m_BgmVolumeValue);
    m_Ctx.Root.AddChild(m_BgmVolumeRightBtn);

    // SE VOLUME 列
    m_Ctx.Root.AddChild(m_SeVolumeLabel);
    m_Ctx.Root.AddChild(m_SeVolumeLeftBtn);
    m_Ctx.Root.AddChild(m_SeVolumeValue);
    m_Ctx.Root.AddChild(m_SeVolumeRightBtn);

    // DISP NUMBER 列
    m_Ctx.Root.AddChild(m_DispNumberLabel);
    m_Ctx.Root.AddChild(m_DispNumberLeftBtn);
    m_Ctx.Root.AddChild(m_DispNumberValue);
    m_Ctx.Root.AddChild(m_DispNumberRightBtn);

    // 底部按鈕
    m_Ctx.Root.AddChild(m_OkText);
    m_Ctx.Root.AddChild(m_CancelText);

    // 調整大框架與關閉按鈕位置
    m_OptionMenuFrame->SetPosition({0.0f, -5.0f});
    m_ExitGameButton->SetPosition({399.2f, 285.1f});

    // 重置按鈕動畫狀態
    m_BgColorLeftBtn->ResetState();
    m_BgColorRightBtn->ResetState();
    m_BgmVolumeLeftBtn->ResetState();
    m_BgmVolumeRightBtn->ResetState();
    m_SeVolumeLeftBtn->ResetState();
    m_SeVolumeRightBtn->ResetState();
    m_DispNumberLeftBtn->ResetState();
    m_DispNumberRightBtn->ResetState();

    // ── 新增：以已儲存設定初始化暫存 ──────────────────────────────────────
    m_Pending = m_Applied;

    // 重置游標到第一列 (KEYBOARD CONFIG)
    m_SelectedRow = 0;
    UpdateValueTexts();
    UpdateChoiceFrame();
}

// ────────────────────────────────────────────────────────────────────────────
// OnExit
// ────────────────────────────────────────────────────────────────────────────
void OptionMenuScene::OnExit() {
    LOG_INFO("OptionMenuScene::OnExit");

    // 本場景私有物件
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

    // 還原共用物件（順序與 ExitConfirmScene 一致）
    m_ExitGameButton->SetPosition({331.0f, -14.0f});
    m_Ctx.Root.RemoveChild(m_ExitGameButton);
}

// ────────────────────────────────────────────────────────────────────────────
// Update  （每 frame 呼叫）
// ────────────────────────────────────────────────────────────────────────────
Scene* OptionMenuScene::Update() {

    // ── 推進按鈕動畫計時器 ──────────────────────────────────────────────────
    m_BgColorLeftBtn->UpdateButton();
    m_BgColorRightBtn->UpdateButton();
    m_BgmVolumeLeftBtn->UpdateButton();
    m_BgmVolumeRightBtn->UpdateButton();
    m_SeVolumeLeftBtn->UpdateButton();
    m_SeVolumeRightBtn->UpdateButton();
    m_DispNumberLeftBtn->UpdateButton();
    m_DispNumberRightBtn->UpdateButton();

    // ── ESC / X 按鈕 / CANCEL 點擊 → 返回 MenuScene ────────────────────────
    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE) ||
        m_ExitGameButton->IsLeftClicked()              ||
        m_CancelText->IsLeftClicked())
    {
        // 玩家取消：把音量還原成上次 OK 確認的值，預覽的調整全部作廢
        m_Ctx.BGMPlayer->SetVolume(m_Applied.bgmVolume * 6);
        LOG_INFO("OptionMenuScene: cancelled => MenuScene");
        return m_MenuScene;
    }

    // ── OK 點擊（滑鼠） ─────────────────────────────────────────────────────
    if (m_KbConfigOpen->IsLeftClicked()) {
        LOG_INFO("OptionMenuScene: KEYBOARD CONFIG OPEN (mouse) => KeyboardConfigScene");
        return m_KeyboardConfigScene;
    }
    if (m_OkText->IsLeftClicked()) {
        m_Applied = m_Pending;
        m_Ctx.BGMPlayer->SetVolume(m_Applied.bgmVolume * 6);
        LOG_INFO("OptionMenuScene: OK (mouse) => MenuScene");
        return m_MenuScene;
    }

    // ── 列游標：W 上移；S /下移 ──────────────────────────────────
    if (Util::Input::IsKeyDown(Util::Keycode::W) || Util::Input::IsKeyDown(Util::Keycode::UP)) {
        DecrementRow();
    }
    else if (Util::Input::IsKeyDown(Util::Keycode::S) || Util::Input::IsKeyDown(Util::Keycode::DOWN)) {
        IncrementRow();
    }

    // ── 滑鼠 hover 自動切換游標列 ───────────────────────────────────────────
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

    // ── 直接點擊各列按鈕（同時跳到該列並調整值）───────────────────────────
    // BG COLOR
    if (m_BgColorLeftBtn->IsLeftClicked()) {
        m_SelectedRow = 1; UpdateChoiceFrame(); AdjustLeft(1);
    } else if (m_BgColorRightBtn->IsLeftClicked()) {
        m_SelectedRow = 1; UpdateChoiceFrame(); AdjustRight(1);
    }
    // BGM VOLUME
    else if (m_BgmVolumeLeftBtn->IsLeftClicked()) {
        m_SelectedRow = 2; UpdateChoiceFrame(); AdjustLeft(2);
    } else if (m_BgmVolumeRightBtn->IsLeftClicked()) {
        m_SelectedRow = 2; UpdateChoiceFrame(); AdjustRight(2);
    }
    // SE VOLUME
    else if (m_SeVolumeLeftBtn->IsLeftClicked()) {
        m_SelectedRow = 3; UpdateChoiceFrame(); AdjustLeft(3);
    } else if (m_SeVolumeRightBtn->IsLeftClicked()) {
        m_SelectedRow = 3; UpdateChoiceFrame(); AdjustRight(3);
    }
    // DISP NUMBER
    else if (m_DispNumberLeftBtn->IsLeftClicked()) {
        m_SelectedRow = 4; UpdateChoiceFrame(); AdjustLeft(4);
    } else if (m_DispNumberRightBtn->IsLeftClicked()) {
        m_SelectedRow = 4; UpdateChoiceFrame(); AdjustRight(4);
    }

    // ── A / D 鍵調整當前列的值 ─────────────────────────────────────────────
    if (Util::Input::IsKeyDown(Util::Keycode::A) || Util::Input::IsKeyDown(Util::Keycode::LEFT)) {
        AdjustLeft(m_SelectedRow) ;
    }
    else if (Util::Input::IsKeyDown(Util::Keycode::D) || Util::Input::IsKeyDown(Util::Keycode::RIGHT)) {
        AdjustRight(m_SelectedRow);
    }

    // ── ENTER 鍵確認 ────────────────────────────────────────────────────────
    if (Util::Input::IsKeyDown(Util::Keycode::RETURN)) {
        switch (m_SelectedRow) {
        case 0:
            LOG_INFO("OptionMenuScene: KEYBOARD CONFIG OPEN (stub)");
            if (m_KeyboardConfigScene != nullptr) return m_KeyboardConfigScene;
            break;
        case 5:
            m_Applied = m_Pending;
            m_Ctx.BGMPlayer->SetVolume(m_Applied.bgmVolume * 6);
            LOG_INFO("OptionMenuScene: OK (keyboard) => MenuScene");
            return m_MenuScene;
        case 6:
            LOG_INFO("OptionMenuScene: CANCEL (keyboard) => MenuScene");
            return m_MenuScene;
        default:
            break;
        }
    }

    return nullptr;
}

// ────────────────────────────────────────────────────────────────────────────
// 私有輔助：游標移動
// ────────────────────────────────────────────────────────────────────────────
void OptionMenuScene::DecrementRow() {
    m_SelectedRow = (m_SelectedRow + ROW_COUNT - 1) % ROW_COUNT;
    UpdateChoiceFrame();
}

void OptionMenuScene::IncrementRow() {
    m_SelectedRow = (m_SelectedRow + 1) % ROW_COUNT;
    UpdateChoiceFrame();
}

// ────────────────────────────────────────────────────────────────────────────
// 私有輔助：左調整（← 方向）
// ────────────────────────────────────────────────────────────────────────────
void OptionMenuScene::AdjustLeft(int row) {
    switch (row) {
    case 1:  // BG COLOR（循環）
        m_Pending.bgColorIndex = (m_Pending.bgColorIndex
                          + static_cast<int>(s_BgColorOptions.size()) - 1)
                         % static_cast<int>(s_BgColorOptions.size());
        m_BgColorLeftBtn->Press(75.0f);
        break;
    case 2:  // BGM VOLUME（下限 0）
        if (m_Pending.bgmVolume > 0) {
            --m_Pending.bgmVolume;
            m_BgmVolumeLeftBtn->Press(75.0f);
            // 即時預覽：m_Pending 還沒 OK，但讓玩家先聽到效果
            m_Ctx.BGMPlayer->SetVolume(m_Pending.bgmVolume * 6);
        }
        break;
    case 3:  // SE VOLUME（下限 0）
        if (m_Pending.seVolume > 0) {
            --m_Pending.seVolume;
            m_SeVolumeLeftBtn->Press(75.0f);
        }
        break;
    case 4:  // DISP NUMBER（LEFT = OFF）
        if (m_Pending.dispNumber) {
            m_Pending.dispNumber = false;
            m_DispNumberLeftBtn->Press(75.0f);
        }
        break;
    case 5:
        m_SelectedRow = 6 ;
        UpdateChoiceFrame();
        break ;
    case 6:
        m_SelectedRow = 5 ;
        UpdateChoiceFrame();
        break ;
    default:
        break;
    }
    UpdateValueTexts();
}

// ────────────────────────────────────────────────────────────────────────────
// 私有輔助：右調整（→ 方向）
// ────────────────────────────────────────────────────────────────────────────
void OptionMenuScene::AdjustRight(int row) {
    switch (row) {
    case 1:  // BG COLOR（循環）
        m_Pending.bgColorIndex = (m_Pending.bgColorIndex + 1)
                         % static_cast<int>(s_BgColorOptions.size());
        m_BgColorRightBtn->Press(75.0f);
        break;
    case 2:  // BGM VOLUME（上限 20）
        if (m_Pending.bgmVolume < 20) {
            ++m_Pending.bgmVolume;
            m_BgmVolumeRightBtn->Press(75.0f);
            // 即時預覽
            m_Ctx.BGMPlayer->SetVolume(m_Pending.bgmVolume * 6);
        }
        break;
    case 3:  // SE VOLUME（上限 20）
        if (m_Pending.seVolume < 20) {
            ++m_Pending.seVolume;
            m_SeVolumeRightBtn->Press(75.0f);
        }
        break;
    case 4:  // DISP NUMBER（RIGHT = ON）
        if (!m_Pending.dispNumber) {
            m_Pending.dispNumber = true;
            m_DispNumberRightBtn->Press(75.0f);
        }
        break;
    case 5:
        m_SelectedRow = 6 ;
        UpdateChoiceFrame();
        break ;
    case 6:
        m_SelectedRow = 5 ;
        UpdateChoiceFrame();
        break ;
    }
    UpdateValueTexts();
}

// ────────────────────────────────────────────────────────────────────────────
// 私有輔助：根據內部狀態重新設定所有值文字
// ────────────────────────────────────────────────────────────────────────────
void OptionMenuScene::UpdateValueTexts() {
    m_BgColorValue->SetText(s_BgColorOptions[m_Pending.bgColorIndex]);
    m_BgmVolumeValue->SetText(std::to_string(m_Pending.bgmVolume));
    m_SeVolumeValue->SetText(std::to_string(m_Pending.seVolume));
    m_DispNumberValue->SetText(m_Pending.dispNumber ? "ON" : "OFF");
}

// ────────────────────────────────────────────────────────────────────────────
// 私有輔助：把 m_ChoiceFrame 移到當前選中列的「值區域」
// ────────────────────────────────────────────────────────────────────────────
void OptionMenuScene::UpdateChoiceFrame() {
    switch (m_SelectedRow) {
    case 0:  // KEYBOARD CONFIG → 圍住 OPEN
        m_ChoiceFrame->SetScale({277.0f/367.0f, 1.1f}) ;
        m_ChoiceFrame->SetPosition({COL_VALUE_X-15.0f, ROW_Y_KB});
        break;
    case 1:  // BG COLOR → 圍住中間值
        m_ChoiceFrame->SetScale({187.0f/367.0f, 1.1f}) ;
        m_ChoiceFrame->SetPosition({COL_VALUE_X-13.5f, ROW_Y_BG});
        break;
    case 2:  // BGM VOLUME
        m_ChoiceFrame->SetScale({187.0f/367.0f, 1.1f}) ;
        m_ChoiceFrame->SetPosition({COL_VALUE_X-13.5f, ROW_Y_BGM});
        break;
    case 3:  // SE VOLUME
        m_ChoiceFrame->SetScale({187.0f/367.0f, 1.1f}) ;
        m_ChoiceFrame->SetPosition({COL_VALUE_X-13.5f, ROW_Y_SE});
        break;
    case 4:  // DISP NUMBER
        m_ChoiceFrame->SetScale({187.0f/367.0f, 1.1f}) ;
        m_ChoiceFrame->SetPosition({COL_VALUE_X-13.5f, ROW_Y_DISP});
        break;
    case 5:  // OK
        m_ChoiceFrame->SetScale({207.0f/367.0f, 1.1f}) ;
        m_ChoiceFrame->SetPosition({COL_OK_X-15.0f, ROW_Y_BTN});
        break;
    case 6:  // CANCEL
        m_ChoiceFrame->SetScale({207.0f/367.0f, 1.1f}) ;
        m_ChoiceFrame->SetPosition({COL_CANCEL_X-15.0f, ROW_Y_BTN});
        break;
    default:
        break;
    }
}