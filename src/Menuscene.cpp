#include "MenuScene.hpp"
#include "ExitConfirmScene.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

// OptionScene / PlayerSelectScene 目前是 stub，只有前向宣告，
// 不需要 include 完整 header，等到實作時再加上。

// =============================================================================
// Constructor
// =============================================================================
MenuScene::MenuScene(GameContext& ctx,
                     Scene* titleScene,
                     ExitConfirmScene* exitConfirmScene,
                     OptionScene* optionScene,
                     PlayerSelectScene* playerSelectScene)
    : Scene(ctx)
    , m_TitleScene(titleScene)
    , m_ExitConfirmScene(exitConfirmScene)
    , m_OptionScene(optionScene)
    , m_PlayerSelectScene(playerSelectScene)
{
    const Util::Color black = Util::Color::FromRGB(0, 0, 0, 255);

    // 選單框
    m_MenuFrame = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/Menu_Frame.png");
    m_MenuFrame->SetZIndex(10);
    m_MenuFrame->SetPosition({0.0f, -105.0f});

    // 小黑色 X 關閉按鈕
    m_ExitGameButton = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Button/ExitGameButton.png");
    m_ExitGameButton->SetZIndex(20);
    m_ExitGameButton->SetPosition({331.0f, -14.0f});

    // 左右三角按鈕
    m_LeftTriButton = std::make_shared<UI_Triangle_Button>(
        GA_RESOURCE_DIR "/Image/Button/Left_Tri_Button.png",
        GA_RESOURCE_DIR "/Image/Button/Left_Tri_Button_Full.png");
    m_LeftTriButton->SetZIndex(10);
    m_LeftTriButton->SetPosition({-305.0f, -153.0f});

    m_RightTriButton = std::make_shared<UI_Triangle_Button>(
        GA_RESOURCE_DIR "/Image/Button/Right_Tri_Button.png",
        GA_RESOURCE_DIR "/Image/Button/Right_Tri_Button_Full.png");
    m_RightTriButton->SetZIndex(10);
    m_RightTriButton->SetPosition({305.0f, -153.0f});

    // 選項文字（同一位置，切換時只顯示其中一個）
    m_ExitGameText = std::make_shared<GameText>("EXIT GAME", 65, black);
    m_ExitGameText->SetPosition({0.0f, -153.0f});

    m_OptionText = std::make_shared<GameText>("OPTION", 65, black);
    m_OptionText->SetPosition({0.0f, -153.0f});

    m_LocalPlayText = std::make_shared<GameText>(" LOCAL PLAY MODE", 65, black);
    m_LocalPlayText->SetPosition({0.0f, -153.0f});
}

// =============================================================================
// OnEnter
// =============================================================================
void MenuScene::OnEnter() {
    LOG_INFO("MenuScene::OnEnter  index={}", m_SelectedIndex);

    m_Ctx.Root.AddChild(m_MenuFrame);
    m_Ctx.Root.AddChild(m_ExitGameButton);
    m_Ctx.Root.AddChild(m_LeftTriButton);
    m_Ctx.Root.AddChild(m_RightTriButton);
    m_Ctx.Root.AddChild(m_ExitGameText);
    m_Ctx.Root.AddChild(m_OptionText);
    m_Ctx.Root.AddChild(m_LocalPlayText);

    // 重置到 EXIT GAME（index 0）
    // 每次從 TitleScene 進來都從頭開始；
    // 若希望從子場景回來時「記住上次位置」，移除這行即可。
    m_SelectedIndex = 0;

    // 確保按鈕狀態乾淨（不會殘留上次離開時的高亮）
    m_LeftTriButton->ResetState();
    m_RightTriButton->ResetState();

    // 貓咪在選單裡不受輸入控制，但仍然繼續物理更新（落地）
    m_Ctx.BlueCat->SetInputEnabled(false);
    m_Ctx.RedCat->SetInputEnabled(false);

    // 顯示正確的選項文字
    ShowCurrentOption();
}

// =============================================================================
// OnExit
// =============================================================================
void MenuScene::OnExit() {
    LOG_INFO("MenuScene::OnExit");

    m_Ctx.Root.RemoveChild(m_MenuFrame);
    m_Ctx.Root.RemoveChild(m_ExitGameButton);
    m_Ctx.Root.RemoveChild(m_LeftTriButton);
    m_Ctx.Root.RemoveChild(m_RightTriButton);
    m_Ctx.Root.RemoveChild(m_ExitGameText);
    m_Ctx.Root.RemoveChild(m_OptionText);
    m_Ctx.Root.RemoveChild(m_LocalPlayText);
}

// =============================================================================
// Update
// =============================================================================
Scene* MenuScene::Update() {
    // 貓咪物理持續更新（讓貓咪在進入選單前若在空中，能繼續落地）
    m_Ctx.BlueCat->Update(m_Ctx.Floor);
    m_Ctx.RedCat->Update(m_Ctx.Floor);

    // 更新按鈕動畫（UI_Triangle_Button 自己管理 timer，外部只需呼叫 UpdateButton）
    m_LeftTriButton->UpdateButton();
    m_RightTriButton->UpdateButton();

    // ESC 或點擊小黑色 X → 返回 TitleScene
    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE) ||
        m_ExitGameButton->IsLeftClicked()) {
        LOG_INFO("MenuScene: back to TitleScene");
        return m_TitleScene;
    }

    // 左鍵（A）→ 向前循環
    if (Util::Input::IsKeyDown(Util::Keycode::A)) {
        HideAllOptions();
        m_SelectedIndex = (m_SelectedIndex + 2) % 3; // +2 等同 -1 在 mod 3
        m_LeftTriButton->Press(75.0f);
        ShowCurrentOption();
        LOG_INFO("MenuScene: left  index={}", m_SelectedIndex);
        return nullptr;
    }

    // 右鍵（D）→ 向後循環
    if (Util::Input::IsKeyDown(Util::Keycode::D)) {
        HideAllOptions();
        m_SelectedIndex = (m_SelectedIndex + 1) % 3;
        m_RightTriButton->Press(75.0f);
        ShowCurrentOption();
        LOG_INFO("MenuScene: right index={}", m_SelectedIndex);
        return nullptr;
    }

    // ENTER → 進入對應子場景
    if (Util::Input::IsKeyDown(Util::Keycode::RETURN)) {
        switch (m_SelectedIndex) {
        case 0: // EXIT GAME
            LOG_INFO("MenuScene: ENTER on EXIT GAME");
            if (m_ExitConfirmScene != nullptr) return m_ExitConfirmScene;
            break;
        case 1: // OPTION
            LOG_INFO("MenuScene: ENTER on OPTION (stub)");
            // if (m_OptionScene != nullptr) return m_OptionScene;
            break; // 尚未實作時不切換
        case 2: // LOCAL PLAY MODE
            LOG_INFO("MenuScene: ENTER on LOCAL PLAY (stub)");
            // if (m_PlayerSelectScene != nullptr) return m_PlayerSelectScene;
            break; // 同上
        default:
            break;
        }
    }

    return nullptr;
}

// =============================================================================
// Private helpers
// =============================================================================
void MenuScene::ShowCurrentOption() {
    m_ExitGameText->SetVisible(m_SelectedIndex == 0);
    m_OptionText->SetVisible(m_SelectedIndex == 1);
    m_LocalPlayText->SetVisible(m_SelectedIndex == 2);
}

void MenuScene::HideAllOptions() {
    m_ExitGameText->SetVisible(false);
    m_OptionText->SetVisible(false);
    m_LocalPlayText->SetVisible(false);
}