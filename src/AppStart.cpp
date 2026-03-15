#include "App.hpp"
#include "Titlescene.hpp"
#include "Menuscene.hpp"
#include "ExitConfirmScene.hpp"
#include "OptionMenuScene.hpp"
#include "Util/Logger.hpp"

void App::Start() {
    LOG_TRACE("Start");

    // ── Step 1：GameContext ───────────────────────────────────────────────────
    m_Ctx = std::make_unique<GameContext>(m_Root);

    m_Ctx->WhiteBackground = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/white_background.jpg");
    m_Ctx->WhiteBackground->SetZIndex(-10);
    m_Ctx->WhiteBackground->SetScale({100.0f, 100.0f});
    m_Root.AddChild(m_Ctx->WhiteBackground);

    m_Ctx->Floor = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/background_floor.png");
    m_Ctx->Floor->SetZIndex(0);
    m_Ctx->Floor->SetPosition({0.0f, -340.0f});
    m_Root.AddChild(m_Ctx->Floor);

    // const Util::Color orange(254, 133, 78, 255);

    m_Ctx->Header = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/header.png");
    m_Ctx->Header->SetZIndex(0);
    m_Ctx->Header->SetPosition({0.0f, 135.0f});
    m_Root.AddChild(m_Ctx->Header);

    m_Ctx->BlueCat = std::make_shared<PlayerCat>(
        std::vector<std::string>{GA_RESOURCE_DIR "/Image/Character/blue_cat.png"},
        Util::Keycode::A, Util::Keycode::D, Util::Keycode::W);
    m_Ctx->BlueCat->SetZIndex(20);
    m_Ctx->BlueCat->SetPosition({-30.0f, -281.5f});
    m_Ctx->BlueCat->SetScale({0.2f, 0.2f});
    m_Root.AddChild(m_Ctx->BlueCat);

    m_Ctx->RedCat = std::make_shared<PlayerCat>(
        std::vector<std::string>{GA_RESOURCE_DIR "/Image/Character/red_cat.png"},
        Util::Keycode::LEFT, Util::Keycode::RIGHT, Util::Keycode::UP);
    m_Ctx->RedCat->SetZIndex(20);
    m_Ctx->RedCat->SetPosition({30.0f, -281.5f});
    m_Ctx->RedCat->SetScale({0.2f, 0.2f});
    m_Root.AddChild(m_Ctx->RedCat);

    // ── Step 2：場景建立 ──────────────────────────────────────────────────────
    // TitleScene
    auto titleScene = std::make_unique<TitleScene>(
        *m_Ctx, nullptr);

    auto menuScene = std::make_unique<MenuScene>(
        *m_Ctx, titleScene.get(), nullptr, nullptr, nullptr);

    auto exitConfirmScene = std::make_unique<ExitConfirmScene>(
        *m_Ctx,
        menuScene.get(),
        menuScene->GetMenuFrame(),
        menuScene->GetExitGameButton()
    );

    // ── 新增 OptionMenuScene ──────────────────────────────────────
    auto optionMenuScene = std::make_unique<OptionMenuScene>(
        *m_Ctx,
        menuScene.get(),
        menuScene->GetExitGameButton()
    );

    menuScene->SetTitleScene(titleScene.get());
    menuScene->SetExitConfirmScene(exitConfirmScene.get());
    menuScene->SetOptionScene(optionMenuScene.get());     // ← 新增（見下方說明）
    titleScene->SetMenuScene(menuScene.get());
    // exitConfirmScene->SetMenuScene(menuScene.get()); // 已從建構子傳入
    // optionMenuScene->SetMenuScene(menuScene.get()); // 同上

    // unique_ptr<MenuScene> → unique_ptr<Scene> upcast
    m_TitleScene       = std::move(titleScene);
    m_MenuScene        = std::move(menuScene);
    m_ExitConfirmScene = std::move(exitConfirmScene);
    m_OptionMenuScene  = std::move(optionMenuScene);

    TransitionTo(m_TitleScene.get());
    m_CurrentState = State::UPDATE;
}