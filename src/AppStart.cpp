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
    m_Floor->SetPosition({0.0f, -330.0f}); // Place at the bottom
    m_Root.AddChild(m_Floor);

    m_Header = std::make_shared<Character>(GA_RESOURCE_DIR"/Image/Background/header.png");
    m_Header->SetZIndex(10);
    m_Header->SetPosition({0.0f, 100.0f});
    m_Header->SetVisible(true);
    m_Root.AddChild(m_Header);

    m_BlueCat = std::make_shared<Character>(GA_RESOURCE_DIR"/Image/Character/blue_cat.png");
    m_BlueCat->SetZIndex(20);
    m_BlueCat->SetPosition({0.0f, -200.0f}); // Place on top of the floor
    m_BlueCat->SetScale({0.2f, 0.2f});
    m_Root.AddChild(m_BlueCat);

    m_CurrentState = State::UPDATE;
}
