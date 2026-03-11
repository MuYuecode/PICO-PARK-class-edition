#include "App.hpp"

#include "Util/Logger.hpp"

void App::Start() {
    LOG_TRACE("Start");

    m_WhiteBackground = std::make_shared<Character>(GA_RESOURCE_DIR"/Image/Background/white_background.jpg");
    m_WhiteBackground->SetZIndex(-10);
    m_WhiteBackground->SetScale({100.0f, 100.0f}); // Make it large enough to cover the screen
    m_Root.AddChild(m_WhiteBackground);

    m_Floor = std::make_shared<Character>(GA_RESOURCE_DIR"/Image/Background/background_floor.png");
    m_Floor->SetZIndex(0);
    m_Floor->SetPosition({0.0f, -340.0f}); // Place at the bottom
    m_Root.AddChild(m_Floor);

    // m_Header = std::make_shared<Character>(GA_RESOURCE_DIR"/Image/Background/header.png");
    // m_Header->SetZIndex(10);
    // m_Header->SetPosition({0.0f, 100.0f});
    // m_Header->SetVisible(true);
    // m_Root.AddChild(m_Header);

    // --- title ---
    Util::Color orange(254, 133, 78, 255);

    // 1. big title PICO PARK
    m_TitleMain = std::make_shared<GameText>("PICO PARK", 240, orange);
    m_TitleMain->SetPosition({0.0f, 155.0f});
    m_Root.AddChild(m_TitleMain);

    // 2. mid_line
    m_TitleLine = std::make_shared<GameText>("─────────", 240, orange);
    m_TitleLine->SetPosition({0.0f, 19.0f});
    m_Root.AddChild(m_TitleLine);

    // 3. sub title CLASSIC EDITED
    m_TitleSub = std::make_shared<GameText>("- CLASSIC EDITED -", 46, orange);
    m_TitleSub->SetPosition({0.0f, -22.5f});
    m_Root.AddChild(m_TitleSub);
    // -------------------

    m_BlueCat = std::make_shared<Character>(GA_RESOURCE_DIR"/Image/Character/blue_cat.png");
    m_BlueCat->SetZIndex(20);
    m_BlueCat->SetPosition({0.0f, -281.5f}); // Place on top of the floor
    m_BlueCat->SetScale({0.2f, 0.2f});
    m_Root.AddChild(m_BlueCat);

    m_CurrentState = State::UPDATE;
}
