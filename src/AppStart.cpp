#include "App.hpp"
#include "Util/Logger.hpp"


void App::Start() {
    LOG_TRACE("Start");

    // -- 預設白色 background --
    m_WhiteBackground = std::make_shared<Character>(GA_RESOURCE_DIR"/Image/Background/white_background.jpg");
    m_WhiteBackground->SetZIndex(-10);
    m_WhiteBackground->SetScale({100.0f, 100.0f}); // Make it large enough to cover the screen
    m_Root.AddChild(m_WhiteBackground);

    m_Floor = std::make_shared<Character>(GA_RESOURCE_DIR"/Image/Background/background_floor.png");
    m_Floor->SetZIndex(0);
    m_Floor->SetPosition({0.0f, -340.0f}); // Place at the bottom
    m_Root.AddChild(m_Floor);

    // -- big title --
    m_Header = std::make_shared<Character>(GA_RESOURCE_DIR"/Image/Background/header.png");
    m_Header->SetZIndex(10);
    m_Header->SetPosition({0.0f, 135.0f});
    m_Root.AddChild(m_Header);


    // --- title ---
    Util::Color orange(254, 133, 78, 255);
    Util::Color black = Util::Color::FromRGB(0,0,0,255) ;

    // sub_title CLASSIC EDITED
    m_TitleSub = std::make_shared<GameText>("- CLASSIC EDITED -", 46, orange);
    m_TitleSub->SetPosition({0.0f, -22.5f});
    m_Root.AddChild(m_TitleSub);

    // PRESS ENTER KEY
    m_PressEnterText = std::make_shared<GameText>("PRESS ENTER KEY", 66, orange);
    m_PressEnterText->SetPosition({0.0f, -155.0f});
    m_Root.AddChild(m_PressEnterText);
    // -------------------

    // blue_cat
    std::vector<std::string> blueCatPaths = {GA_RESOURCE_DIR"/Image/Character/blue_cat.png"};
    m_BlueCat = std::make_shared<PlayerCat>(blueCatPaths, Util::Keycode::A, Util::Keycode::D, Util::Keycode::W);
    m_BlueCat->SetZIndex(20);
    m_BlueCat->SetPosition({-30.0f, -281.5f}); // Place on top of the floor
    m_BlueCat->SetScale({0.2f, 0.2f});
    m_Root.AddChild(m_BlueCat);

    // red_cat
    std::vector<std::string> redCatPaths = {GA_RESOURCE_DIR"/Image/Character/red_cat.png"};
    m_RedCat = std::make_shared<PlayerCat>(redCatPaths, Util::Keycode::LEFT, Util::Keycode::RIGHT, Util::Keycode::UP);
    m_RedCat->SetZIndex(20);
    m_RedCat->SetPosition({30.0f, -281.5f});
    m_RedCat->SetScale({0.2f, 0.2f});
    m_Root.AddChild(m_RedCat);

    // === 初始化 STATE_01 所需物件 (預設隱藏) ===
    m_ExitGameText = std::make_shared<GameText>("EXIT GAME", 65, black);
    m_ExitGameText->SetPosition({0.0f, -153.0f});
    m_ExitGameText->SetVisible(false);
    m_Root.AddChild(m_ExitGameText);
    m_OptionText = std::make_shared<GameText>("OPTION", 65, black);
    m_OptionText->SetPosition({0.0f, -153.0f});
    m_OptionText->SetVisible(false);
    m_Root.AddChild(m_OptionText);
    m_LocalPlayText = std::make_shared<GameText>(" LOCAL PLAY MODE", 65, black);
    m_LocalPlayText->SetPosition({0.0f, -153.0f});
    m_LocalPlayText->SetVisible(false);
    m_Root.AddChild(m_LocalPlayText);

    m_MenuFrame = std::make_shared<Character>(GA_RESOURCE_DIR"/Image/Background/Menu_Frame.png");
    m_MenuFrame->SetZIndex(10);
    m_MenuFrame->SetPosition({0.0f, -105.0f});
    m_MenuFrame->SetVisible(false);
    m_Root.AddChild(m_MenuFrame);

    m_ExitGameButton = std::make_shared<Character>(GA_RESOURCE_DIR"/Image/Button/ExitGameButton.png") ;
    m_ExitGameButton->SetZIndex(20);
    m_ExitGameButton->SetPosition({331.0f, -14.0f});
    m_ExitGameButton->SetVisible(false);
    m_Root.AddChild(m_ExitGameButton);

    // MenuFrame_x + 305 == right_tri_button
    m_Left_Tri_Button = std::make_shared<Character>(GA_RESOURCE_DIR"/Image/Button/Left_Tri_Button.png") ;
    m_Left_Tri_Button->SetZIndex(10);
    m_Left_Tri_Button->SetPosition({-305.0f, -153.0f});
    m_Left_Tri_Button->SetVisible(false);
    m_Root.AddChild(m_Left_Tri_Button);

    m_Right_Tri_Button = std::make_shared<Character>(GA_RESOURCE_DIR"/Image/Button/Right_Tri_Button.png") ;
    m_Right_Tri_Button->SetZIndex(10);
    m_Right_Tri_Button->SetPosition({305.0f, -153.0f});
    m_Right_Tri_Button->SetVisible(false);
    m_Root.AddChild(m_Right_Tri_Button);

    m_CurrentState = State::UPDATE; // don't delete this line
}
