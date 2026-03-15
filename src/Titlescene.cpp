#include "Titlescene.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"
#include "Menuscene.hpp"

TitleScene::TitleScene(GameContext& ctx, MenuScene* menuScene)
    : Scene(ctx), m_MenuScene(menuScene)
{
    const Util::Color orange(254, 133, 78, 255);

    // m_Header 和 m_TitleSub 已移至 GameContext，不在這裡建立。
    // TitleScene 只建立自己私有的 m_PressEnterText。
    m_TitleSub = std::make_shared<GameText>("- CLASSIC EDITED -", 46, orange);
    m_TitleSub->SetZIndex(0);
    m_TitleSub->SetPosition({0.0f, -22.5f});

    m_PressEnterText = std::make_shared<GameText>("PRESS ENTER KEY", 66, orange);
    m_TitleSub->SetZIndex(0);
    m_PressEnterText->SetPosition({0.0f, -155.0f});
}

void TitleScene::OnEnter() {
    LOG_INFO("TitleScene::OnEnter");

    // Header 和 TitleSub 已永久在渲染樹，只需確保可見
    m_Ctx.Header->SetVisible(true);

    // m_TitleSub 和 m_PressEnterText 是 TitleScene 私有的，需要 AddChild
    m_Ctx.Root.AddChild(m_TitleSub);
    m_Ctx.Root.AddChild(m_PressEnterText);

    m_FlashTimer = 0.0f;
    m_PressEnterText->SetVisible(true);

    m_Ctx.BlueCat->SetInputEnabled(true);
    m_Ctx.RedCat->SetInputEnabled(true);
}

void TitleScene::OnExit() {
    LOG_INFO("TitleScene::OnExit");

    // Header 和 TitleSub 不移除，讓 MenuScene 也能看到它們。
    // 只移除 TitleScene 私有的 m_PressEnterText 和 m_TitleSub。
    m_Ctx.Root.RemoveChild(m_TitleSub);
    m_Ctx.Root.RemoveChild(m_PressEnterText);

    m_Ctx.BlueCat->SetInputEnabled(false);
    m_Ctx.RedCat->SetInputEnabled(false);
}

Scene* TitleScene::Update() {
    m_FlashTimer += Util::Time::GetDeltaTimeMs();
    if (m_FlashTimer >= 1000.0f) {
        m_PressEnterText->SetVisible(!m_PressEnterText->GetVisibility());
        m_FlashTimer = 0.0f;
    }

    m_Ctx.BlueCat->Update(m_Ctx.Floor);
    m_Ctx.RedCat->Update(m_Ctx.Floor);

    if (Util::Input::IsKeyDown(Util::Keycode::RETURN)) {
        LOG_INFO("TitleScene: ENTER pressed, switching to MenuScene");
        return m_MenuScene;
    }

    return nullptr;
}