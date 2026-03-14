#include "App.hpp"
#include "TitleScene.hpp"
#include "MenuScene.hpp"
#include "ExitConfirmScene.hpp"
#include "Util/Logger.hpp"

void App::Start() {
    LOG_TRACE("Start");

    // ── Step 1：GameContext（跨場景共用物件）────────────────────────────────
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

    // ── Step 2：建立場景 ──────────────────────────────────────────────────────
    //
    // 注意相依順序：
    //   ExitConfirmScene 需要 MenuScene 指標
    //   MenuScene        需要 ExitConfirmScene 指標
    //
    // 解法：先建立 MenuScene（此時 ExitConfirmScene 還是 nullptr），
    //       再建立 ExitConfirmScene 並把兩個共用物件傳入，
    //       最後再把 ExitConfirmScene 的指標告訴 MenuScene。
    //
    // 因此 MenuScene 需要一個 SetExitConfirmScene() setter，
    // 或者在 constructor 用 nullptr 占位，之後再 Set。

    // MenuScene 傳入的 MenuFrame 與 ExitGameButton 需要在 ExitConfirmScene
    // 裡修改外觀，所以要先建立這兩個物件並「借」給雙方。
    // 最乾淨的做法是讓 MenuScene 建立這些物件，再把 shared_ptr 傳給
    // ExitConfirmScene。這裡示範直接由 App 建立並傳遞。

    auto menuFrame = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/Menu_Frame.png");
    menuFrame->SetZIndex(10);
    menuFrame->SetPosition({0.0f, -105.0f});

    auto exitButton = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Button/ExitGameButton.png");
    exitButton->SetZIndex(20);
    exitButton->SetPosition({331.0f, -14.0f});

    // 建立時用 make_unique<具體型別>，指派給 unique_ptr<Scene> 完全合法
    auto exitConfirmScene = std::make_unique<ExitConfirmScene>(
        *m_Ctx, nullptr, menuFrame, exitButton);
    auto menuScene = std::make_unique<MenuScene>(
        *m_Ctx, nullptr, exitConfirmScene.get(), nullptr, nullptr);
    auto titleScene = std::make_unique<TitleScene>(
        *m_Ctx, menuScene.get());

    // setter 在 move 進 unique_ptr<Scene> 之前呼叫，此時還持有具體型別指標
    menuScene->SetTitleScene(titleScene.get());
    exitConfirmScene->SetMenuScene(menuScene.get());

    // move 進成員（upcast 自動發生）
    m_TitleScene       = std::move(titleScene);
    m_MenuScene        = std::move(menuScene);
    m_ExitConfirmScene = std::move(exitConfirmScene);

    TransitionTo(m_TitleScene.get());
    m_CurrentState = State::UPDATE;
}