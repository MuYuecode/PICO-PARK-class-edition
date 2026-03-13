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
    case GameState::STATE_01_1_1:
    case GameState::STATE_01_1_2:
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
        LOG_INFO("Change to STATE_01");
    }
}

void App::UpdateState01() {
    // 打開 MenuFrame
    m_MenuFrame->SetVisible(true);
    m_ExitGameButton->SetVisible(true);

    // 依然呼叫 Update，這樣貓咪就算在空中進入選單，還是會掉到地板上站好，而不會定格在半空
    m_BlueCat->Update(m_Floor);
    m_RedCat->Update(m_Floor);

    if (m_GameState == GameState::STATE_01_1 || m_GameState == GameState::STATE_01_2 || m_GameState == GameState::STATE_01_3) {
        LOG_INFO("STATE_01 detect in STATE_01_X");
        m_ExitGameButton->SetPosition({331.0f, -14.0f});
        m_Left_Tri_Button->SetVisible(true);
        m_Right_Tri_Button->SetVisible(true);

        // ================= 新增：按鈕動畫計時器邏輯 =================
        // 處理左按鈕
        if (m_LeftButtonTimer > 0.0f) {
            m_LeftButtonTimer -= Util::Time::GetDeltaTimeMs();
            if (m_LeftButtonTimer <= 0.0f) {
                m_LeftButtonTimer = 0.0f; // 防止負數
                m_Left_Tri_Button->SetImage(GA_RESOURCE_DIR"/Image/Button/Left_Tri_Button.png");
            }
        }
        // 處理右按鈕
        if (m_RightButtonTimer > 0.0f) {
            m_RightButtonTimer -= Util::Time::GetDeltaTimeMs();
            if (m_RightButtonTimer <= 0.0f) {
                m_RightButtonTimer = 0.0f; // 防止負數
                m_Right_Tri_Button->SetImage(GA_RESOURCE_DIR"/Image/Button/Right_Tri_Button.png");
            }
        }
        // ============================================================

        if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE) || m_ExitGameButton->IsLeftClicked()) {
            m_MenuFrame->SetVisible(false);

            m_ExitGameButton->SetVisible(false);
            m_Left_Tri_Button->SetVisible(false);
            m_Right_Tri_Button->SetVisible(false);

            m_ExitGameText->SetVisible(false);
            m_OptionText->SetVisible(false);
            m_LocalPlayText->SetVisible(false);

            m_GameState = GameState::STATE_00;
            return; // 提早返回，不執行下方邏輯
        }
    }
    else if (m_GameState == GameState::STATE_01_1_1 || m_GameState == GameState::STATE_01_1_2) {
        LOG_INFO("STATE_01 detect in STATE_01_1_X");
        m_ExitGameButton->SetPosition({192.3f, 9.0f});
        m_Left_Tri_Button->SetVisible(false);
        m_Right_Tri_Button->SetVisible(false);
    }

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
    case GameState::STATE_01_1_1:
        UpdateState01_1_1() ;
        break ;
    case GameState::STATE_01_1_2:
        UpdateState01_1_2() ;
        break ;
    default:
        LOG_ERROR("Unknown GameState: {}", static_cast<int>(m_GameState));
        break ;
    }
}

void App::UpdateState01_1() {
    m_ExitGameText->SetVisible(true);

    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
        m_MenuFrame->SetVisible(false);
        m_ExitGameButton->SetVisible(false);
        m_Left_Tri_Button->SetVisible(false);
        m_Right_Tri_Button->SetVisible(false);

        m_ExitGameText->SetVisible(false);

        // --- 新增：退出時強制還原按鈕狀態，避免下次開啟時出Bug ---
        m_LeftButtonTimer = 0.0f;
        m_Left_Tri_Button->SetImage(GA_RESOURCE_DIR"/Image/Button/Left_Tri_Button.png");
        m_RightButtonTimer = 0.0f;
        m_Right_Tri_Button->SetImage(GA_RESOURCE_DIR"/Image/Button/Right_Tri_Button.png");
        // --------------------------------------------------------

        m_GameState = GameState::STATE_00 ;
        return ; // 提早返回，不執行下方邏輯
    }
    if (Util::Input::IsKeyDown(Util::Keycode::RETURN)) {
        // --- 進入 01_1_1 子畫面 ---
        m_ExitGameText->SetVisible(false); // 隱藏原本的文字
        m_GameState = GameState::STATE_01_1_1;

        // 改變選單框的比例
        m_MenuFrame->SetScale({408.0f / 695.0f, 287.0f / 218.0f});

        // 更換小黑色 X 的位置

        return ;
    }
    if (Util::Input::IsKeyDown(Util::Keycode::A)) {
        m_ExitGameText->SetVisible(false) ;
        m_GameState = GameState::STATE_01_2 ;

        // --- 新增：觸發左按鈕動畫 ---
        m_Left_Tri_Button->SetImage(GA_RESOURCE_DIR"/Image/Button/Left_Tri_Button_Full.png");
        m_LeftButtonTimer = 200.0f;
    }
    else if (Util::Input::IsKeyDown(Util::Keycode::D)) {
        m_ExitGameText->SetVisible(false);
        m_GameState = GameState::STATE_01_3 ;

        // --- 新增：觸發右按鈕動畫 ---
        m_Right_Tri_Button->SetImage(GA_RESOURCE_DIR"/Image/Button/Right_Tri_Button_Full.png");
        m_RightButtonTimer = 200.0f;
    }
}

void App::UpdateState01_1_1() {
    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE) || m_ExitGameButton->IsLeftClicked() || Util::Input::IsKeyDown(Util::Keycode::RETURN)) {
        m_YESText->SetVisible(false) ;
        m_NOText->SetVisible(false) ;
        m_ExitGame_1Text->SetVisible(false) ;
        m_Choice_Frame->SetVisible(false) ;
        m_MenuFrame->SetScale({1.0f, 1.0f});
        m_GameState = GameState::STATE_01_1 ;
        return ; // 提早返回，不執行下方邏輯
    }
    if (Util::Input::IsKeyDown(Util::Keycode::A) || Util::Input::IsKeyDown(Util::Keycode::D) || m_YESText->IsMouseHovering()) {
        // --- 進入 01_1_2  ---
        m_GameState = GameState::STATE_01_1_2 ;
        return ;
    }
    m_Choice_Frame->SetPosition({120.0f,-180.0f});
    m_Choice_Frame->SetVisible(true);

    m_YESText->SetVisible(true) ;
    m_NOText->SetVisible(true) ;
    m_ExitGame_1Text->SetVisible(true) ;
}

void App::UpdateState01_1_2() {
    if (Util::Input::IsKeyDown(Util::Keycode::RETURN)) {
        m_CurrentState = State::END;
        return ; // 遊戲結束，不執行下方邏輯
    }
    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE) || m_ExitGameButton->IsLeftClicked()) {
        m_YESText->SetVisible(false) ;
        m_NOText->SetVisible(false) ;
        m_ExitGame_1Text->SetVisible(false) ;
        m_Choice_Frame->SetVisible(false) ;
        m_MenuFrame->SetScale({1.0f, 1.0f});
        m_GameState = GameState::STATE_01_1 ;
        return ; // 提早返回，不執行下方邏輯
    }
    if (Util::Input::IsKeyDown(Util::Keycode::A) || Util::Input::IsKeyDown(Util::Keycode::D)  || m_NOText->IsMouseHovering()) {
        // --- 進入 01_1_1  ---
        m_GameState = GameState::STATE_01_1_1 ;
        return ;
    }

    m_Choice_Frame->SetPosition({-120.0f,-180.0f});
}



void App::UpdateState01_2() {
    m_OptionText->SetVisible(true);
    m_ExitGameText->SetVisible(false);
    m_LocalPlayText->SetVisible(false);

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

        // --- 新增：觸發左按鈕動畫 ---
        m_Left_Tri_Button->SetImage(GA_RESOURCE_DIR"/Image/Button/Left_Tri_Button_Full.png");
        m_LeftButtonTimer = 200.0f;
    }
    else if (Util::Input::IsKeyDown(Util::Keycode::D)) {
        m_OptionText->SetVisible(false);
        m_GameState = GameState::STATE_01_1 ;

        // --- 新增：觸發右按鈕動畫 ---
        m_Right_Tri_Button->SetImage(GA_RESOURCE_DIR"/Image/Button/Right_Tri_Button_Full.png");
        m_RightButtonTimer = 200.0f;
    }
}

void App::UpdateState01_3() {
    m_LocalPlayText->SetVisible(true);
    m_ExitGameText->SetVisible(false);
    m_OptionText->SetVisible(false);

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

        // --- 新增：觸發左按鈕動畫 ---
        m_Left_Tri_Button->SetImage(GA_RESOURCE_DIR"/Image/Button/Left_Tri_Button_Full.png");
        m_LeftButtonTimer = 200.0f;
    }
    else if (Util::Input::IsKeyDown(Util::Keycode::D)) {
        m_LocalPlayText->SetVisible(false);
        m_GameState = GameState::STATE_01_2 ;

        // --- 新增：觸發右按鈕動畫 ---
        m_Right_Tri_Button->SetImage(GA_RESOURCE_DIR"/Image/Button/Right_Tri_Button_Full.png");
        m_RightButtonTimer = 200.0f;
    }
}



