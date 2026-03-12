#include "App.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"

void App::Update() {
    switch (m_GameState) {
    case GameState::STATE_00:
        UpdateState00();
        break;
    case GameState::STATE_01_1:
    case GameState::STATE_01_2:
    case GameState::STATE_01_3:
        UpdateState01();
        break;
    }

    m_Root.Update();
}

void App::UpdateState00() {
    // 從 01 回到 00 時要確保 sub_title 和 PRESS ENTER KEY 閃爍動畫回來
    m_TitleSub->SetVisible(true);
    m_FlashTimer += Util::Time::GetDeltaTimeMs();
    if (m_FlashTimer >= 1000.0f) { // 1000毫秒 = 1秒
        m_PressEnterText->SetVisible(!m_PressEnterText->GetVisibility()); // 切換可見度
        m_FlashTimer = 0.0f; // 重置計時器
    }

    // 進入狀態 00 時，確保貓咪可以動
    m_BlueCat->SetInputEnabled(true);
    m_RedCat->SetInputEnabled(true);

    // 呼叫貓咪的更新 (傳入地板做碰撞)
    m_BlueCat->Update(m_Floor);
    m_RedCat->Update(m_Floor);

    // 如果按下 Enter 切換到 01 狀態
    if (Util::Input::IsKeyDown(Util::Keycode::RETURN)) {
        m_TitleSub->SetVisible(false) ;
        m_PressEnterText->SetVisible(false) ;
        m_BlueCat->SetInputEnabled(false);
        m_RedCat->SetInputEnabled(false);
        m_GameState = GameState::STATE_01_1;
    }
}

void App::UpdateState01() {
    // 打開 MenuFrame
    m_MenuFrame->SetVisible(true);
    m_ExitGameButton->SetVisible(true);
    m_Left_Tri_Button->SetVisible(true);
    m_Right_Tri_Button->SetVisible(true);
    // 依然呼叫 Update，這樣貓咪就算在空中進入選單，還是會掉到地板上站好，而不會定格在半空
    m_BlueCat->Update(m_Floor);
    m_RedCat->Update(m_Floor);

    switch (m_GameState) {
    case GameState::STATE_01_1 :
        UpdateState01_1();
        break;
    case GameState::STATE_01_2 :
        UpdateState01_2() ;
        break ;
    case GameState::STATE_01_3 :
        UpdateState01_3() ;
        break ;
    default:
        LOG_ERROR("Unknown GameState: {}", static_cast<int>(m_GameState));
        break ;
    }
}

void App::UpdateState01_1() {
    m_ExitGameText->SetVisible(true);


    if (Util::Input::IsKeyDown(Util::Keycode::RETURN) || Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
        m_MenuFrame->SetVisible(false);
        m_ExitGameButton->SetVisible(false);
        m_Left_Tri_Button->SetVisible(false);
        m_Right_Tri_Button->SetVisible(false);

        m_ExitGameText->SetVisible(false);

        m_GameState = GameState::STATE_00 ;
    }
    else if (Util::Input::IsKeyDown(Util::Keycode::A)) {
        m_ExitGameText->SetVisible(false);
        m_GameState = GameState::STATE_01_2 ;
    }
    else if (Util::Input::IsKeyDown(Util::Keycode::D)) {
        m_ExitGameText->SetVisible(false);
        m_GameState = GameState::STATE_01_3 ;
    }
}

void App::UpdateState01_2() {
    m_OptionText->SetVisible(true);

    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
        m_MenuFrame->SetVisible(false);
        m_ExitGameButton->SetVisible(false);
        m_Left_Tri_Button->SetVisible(false);
        m_Right_Tri_Button->SetVisible(false);

        m_OptionText->SetVisible(false);

        m_GameState = GameState::STATE_00 ;
    }
    else if (Util::Input::IsKeyDown(Util::Keycode::RETURN)) {
        m_OptionText->SetVisible(false);
        // m_GameState = GameState::STATE_01_2_1) ; // enter OPTION
    }
    else if (Util::Input::IsKeyDown(Util::Keycode::A)) {
        m_OptionText->SetVisible(false);
        m_GameState = GameState::STATE_01_3 ;
    }
    else if (Util::Input::IsKeyDown(Util::Keycode::D)) {
        m_OptionText->SetVisible(false);
        m_GameState = GameState::STATE_01_1 ;
    }
}

void App::UpdateState01_3() {
    m_LocalPlayText->SetVisible(true);

    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
        m_MenuFrame->SetVisible(false);
        m_ExitGameButton->SetVisible(false);
        m_Left_Tri_Button->SetVisible(false);
        m_Right_Tri_Button->SetVisible(false);

        m_LocalPlayText->SetVisible(false);

        m_GameState = GameState::STATE_00 ;
    }
    else if (Util::Input::IsKeyDown(Util::Keycode::RETURN)) {
        m_LocalPlayText->SetVisible(false);
        // m_GameState = GameState::STATE_01_2_1) ; // enter OPTION
    }
    else if (Util::Input::IsKeyDown(Util::Keycode::A)) {
        m_LocalPlayText->SetVisible(false);
        m_GameState = GameState::STATE_01_1 ;
    }
    else if (Util::Input::IsKeyDown(Util::Keycode::D)) {
        m_LocalPlayText->SetVisible(false);
        m_GameState = GameState::STATE_01_2 ;
    }
}



