//
// Created by cody2 on 2026/3/15.
//

#include "TitleScene.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"
#include "Menuscene.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
//
// 只做「物件建立」，不呼叫 AddChild。
// 原因：物件建立的時機（App::Start）和「進入場景」的時機通常不同。
// 若在 constructor 就 AddChild，那麼遊戲一啟動所有場景的物件就全部進渲染樹，
// 這會造成看不見但仍然存在於渲染清單的物件，浪費效能。
// ─────────────────────────────────────────────────────────────────────────────
TitleScene::TitleScene(GameContext& ctx, MenuScene* menuScene)
    : Scene(ctx), m_MenuScene(menuScene) {
    const Util::Color orange(254, 133, 78, 255);

    m_Header = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/header.png");
    m_Header->SetZIndex(30);
    m_Header->SetPosition({0.0f, 135.0f});

    m_TitleSub = std::make_shared<GameText>("- CLASSIC EDITED -", 46, orange);
    m_TitleSub->SetPosition({0.0f, -22.5f});

    m_PressEnterText = std::make_shared<GameText>("PRESS ENTER KEY", 66, orange);
    m_PressEnterText->SetPosition({0.0f, -155.0f});
}

// ─────────────────────────────────────────────────────────────────────────────
// OnEnter：加入渲染樹 + 重置狀態
//
// 每次進入此場景（包含從 MenuScene 返回）都會呼叫一次。
// ─────────────────────────────────────────────────────────────────────────────
void TitleScene::OnEnter() {
    LOG_INFO("TitleScene::OnEnter");

    // 加入渲染樹
    m_Ctx.Root.AddChild(m_Header);
    m_Ctx.Root.AddChild(m_TitleSub);
    m_Ctx.Root.AddChild(m_PressEnterText);

    // 重置狀態（確保從 MenuScene 返回時閃爍動畫從頭開始）
    m_FlashTimer = 0.0f;
    m_PressEnterText->SetVisible(true);
    m_TitleSub->SetVisible(true);

    // 允許貓咪接受玩家輸入
    m_Ctx.BlueCat->SetInputEnabled(true);
    m_Ctx.RedCat->SetInputEnabled(true);
}

// ─────────────────────────────────────────────────────────────────────────────
// OnExit：從渲染樹移除
//
// 必須與 OnEnter 裡的 AddChild 完全配對，
// 否則每次切換回來就多加一份，同一張圖會被渲染 N 次。
// ─────────────────────────────────────────────────────────────────────────────
void TitleScene::OnExit() {
    LOG_INFO("TitleScene::OnExit");

    // RemoveChild 而非 SetVisible(false)：
    // SetVisible 只是隱藏，物件仍在渲染清單佔用空間。
    // RemoveChild 才是真正離開渲染樹，等下次 OnEnter 再加回來。
    m_Ctx.Root.RemoveChild(m_Header);
    m_Ctx.Root.RemoveChild(m_TitleSub);
    m_Ctx.Root.RemoveChild(m_PressEnterText);

    // 禁用貓咪輸入（進入選單後貓咪不應該被操控）
    m_Ctx.BlueCat->SetInputEnabled(false);
    m_Ctx.RedCat->SetInputEnabled(false);
}

// ─────────────────────────────────────────────────────────────────────────────
// Update：每 frame 的邏輯
// ─────────────────────────────────────────────────────────────────────────────
Scene* TitleScene::Update() {
    // 閃爍邏輯：完全封裝在場景內部，App 完全不知道這件事
    m_FlashTimer += Util::Time::GetDeltaTimeMs();
    if (m_FlashTimer >= 1000.0f) {
        m_PressEnterText->SetVisible(!m_PressEnterText->GetVisibility());
        m_FlashTimer = 0.0f;
    }

    // 貓咪物理更新（貓咪在這個場景是裝飾，但仍然有重力/碰撞）
    m_Ctx.BlueCat->Update(m_Ctx.Floor);
    m_Ctx.RedCat->Update(m_Ctx.Floor);

    // 按下 Enter → 切換到 MenuScene
    if (Util::Input::IsKeyDown(Util::Keycode::RETURN)) {
        LOG_INFO("TitleScene: ENTER pressed, switching to MenuScene");
        return m_MenuScene; // App 的 TransitionTo 會處理 OnExit / OnEnter
    }

    return nullptr; // 繼續待在這個場景
}