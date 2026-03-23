//
// Created by cody2 on 2026/3/17.
//

#include "LocalPlayScene.hpp"
#include "Menuscene.hpp"
#include "KeyboardConfigScene.hpp"
#include "LocalPlayGameScene.hpp"      // <--- ADD THIS LINE
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

const Util::Color LocalPlayScene::k_Black = Util::Color::FromRGB(0,   0,   0,   255);
const Util::Color LocalPlayScene::k_Red   = Util::Color::FromRGB(220, 50,  50,  255);

LocalPlayScene::LocalPlayScene(GameContext& ctx,
                               MenuScene* menuScene,
                               std::shared_ptr<Character>          menuFrame,
                               std::shared_ptr<Character>          exitGameButton,
                               std::shared_ptr<UI_Triangle_Button> leftTriButton,
                               std::shared_ptr<UI_Triangle_Button> rightTriButton,
                               std::shared_ptr<Character>          blueCatRunImg,
                               KeyboardConfigScene* kbConfigScene)
    : Scene(ctx)
    , m_MenuFrame(std::move(menuFrame))
    , m_ExitGameButton(std::move(exitGameButton))
    , m_LeftTriButton(std::move(leftTriButton))
    , m_RightTriButton(std::move(rightTriButton))
    , m_BlueCatRunImg(std::move(blueCatRunImg))
    , m_MenuScene(menuScene)
    , m_KbConfigScene(kbConfigScene)
{
    // 主文字："nPLAYER GAME"，與 MenuScene 選項文字同位置、同大小
    m_PlayerCountText = std::make_shared<GameText>("2PLAYER GAME", 65, k_Black);
    m_PlayerCountText->SetZIndex(20);
    m_PlayerCountText->SetPosition({0.0f, -153.0f}); // -118

    // 警告文字：略小、紅色，顯示於主文字正上方
    m_NoConfigText = std::make_shared<GameText>("No keyboard config", 30, k_Red);
    m_NoConfigText->SetZIndex(20);
    m_NoConfigText->SetPosition({0.0f, -188.0f});
    m_NoConfigText->SetVisible(false);
}

void LocalPlayScene::OnEnter() {
    LOG_INFO("LocalPlayScene::OnEnter  players={}", m_PlayerCount);

    // 借用的共用物件（從 MenuScene 拿到）
    m_Ctx.Root.AddChild(m_MenuFrame);
    m_Ctx.Root.AddChild(m_ExitGameButton);
    m_Ctx.Root.AddChild(m_LeftTriButton);
    m_Ctx.Root.AddChild(m_RightTriButton);
    m_Ctx.Root.AddChild(m_BlueCatRunImg);

    // 本場景私有物件
    m_Ctx.Root.AddChild(m_PlayerCountText);
    m_Ctx.Root.AddChild(m_NoConfigText);

    // 確保共用元件可見且位置/縮放與 MenuScene 完全相同
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

    // 初始顯示
    UpdateDisplay();
}

void LocalPlayScene::OnExit() {
    LOG_INFO("LocalPlayScene::OnExit");

    // 私有物件直接移除
    m_Ctx.Root.RemoveChild(m_PlayerCountText);
    m_Ctx.Root.RemoveChild(m_NoConfigText);

    // 還原共用物件狀態，再移除（與其他借用場景的慣例一致）
    m_MenuFrame->SetScale({1.0f, 1.0f});
    m_ExitGameButton->SetPosition({331.0f, -14.0f});

    m_Ctx.Root.RemoveChild(m_LeftTriButton);
    m_Ctx.Root.RemoveChild(m_RightTriButton);
    m_Ctx.Root.RemoveChild(m_BlueCatRunImg);
    m_Ctx.Root.RemoveChild(m_ExitGameButton);
    m_Ctx.Root.RemoveChild(m_MenuFrame);
}

Scene* LocalPlayScene::Update() {
    // 推進按鈕動畫計時器
    m_LeftTriButton->UpdateButton();
    m_RightTriButton->UpdateButton();

    // ESC / X 按鈕  返回 MenuScene
    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE) ||
        m_ExitGameButton->IsLeftClicked()) {
        LOG_INFO("LocalPlayScene: back to MenuScene");
        return m_MenuScene;
    }

    // 判斷左右輸入
    const bool pressedLeft  = Util::Input::IsKeyDown(Util::Keycode::A)    ||
                              Util::Input::IsKeyDown(Util::Keycode::LEFT)  ||
                              m_LeftTriButton->IsLeftClicked();

    const bool pressedRight = Util::Input::IsKeyDown(Util::Keycode::D)    ||
                              Util::Input::IsKeyDown(Util::Keycode::RIGHT) ||
                              m_RightTriButton->IsLeftClicked();

    if (pressedLeft) {
        // 向左循環：2 → 8 → 7 → … → 2
        m_PlayerCount = ((m_PlayerCount - MIN_PLAYERS - 1 + (MAX_PLAYERS - MIN_PLAYERS + 1))
                         % (MAX_PLAYERS - MIN_PLAYERS + 1))
                        + MIN_PLAYERS;
        m_LeftTriButton->Press(75.0f);
        UpdateDisplay();
    }
    else if (pressedRight) {
        // 向右循環：2 → 3 → … → 8 → 2
        m_PlayerCount = ((m_PlayerCount - MIN_PLAYERS + 1)
                         % (MAX_PLAYERS - MIN_PLAYERS + 1))
                        + MIN_PLAYERS;
        m_RightTriButton->Press(75.0f);
        UpdateDisplay();
    }

    // ENTER  確認人數(可在此接實際遊戲場景)
    if (Util::Input::IsKeyDown(Util::Keycode::RETURN)) {
        int configuredCount = 0;
        if (m_KbConfigScene != nullptr) {
            configuredCount = m_KbConfigScene->GetConfiguredPlayerCount();
        }

        if (m_PlayerCount <= configuredCount) {
            m_Ctx.SelectedPlayerCount = m_PlayerCount;
            LOG_INFO("LocalPlayScene: ENTER confirmed with {} players", m_PlayerCount);
            return m_GameScene;
        }

        LOG_INFO("LocalPlayScene: ENTER blocked, selected={} configured={}",
                 m_PlayerCount, configuredCount);
    }

    return nullptr;
}

// 根據目前人數更新文字顯示與顏色
void LocalPlayScene::UpdateDisplay() const {
    // 主文字
    m_PlayerCountText->SetText(std::to_string(m_PlayerCount) + "PLAYER GAME");

    // 計算已設定按鍵的玩家數
    int configuredCount = 0;
    if (m_KbConfigScene != nullptr) {
        configuredCount = m_KbConfigScene->GetConfiguredPlayerCount();
    }

    // 若所選人數超過已設定按鍵的人數 → 警告
    const bool hasWarning = (m_PlayerCount > configuredCount);

    m_PlayerCountText->SetColor(hasWarning ? k_Red : k_Black);
    m_NoConfigText->SetVisible(hasWarning);
}