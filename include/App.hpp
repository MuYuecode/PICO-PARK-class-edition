#ifndef APP_HPP
#define APP_HPP

#include "pch.hpp" // IWYU pragma: export
#include "Util/Renderer.hpp"
#include "Character.hpp"
#include "AnimatedCharacter.hpp"
#include "PlayerCat.hpp"
#include "PhaseResourceManger.hpp"
#include "GameText.hpp"

class App {
public:
    enum class State {
        START,
        UPDATE,
        END
    };
    enum class GameState {
        STATE_00,   // header screen
        STATE_01_1, // Menu screen - EXIT
        STATE_01_2, // Menu screen - OPTION
        STATE_01_3  // Menu screen - LOCAL PLAY MODE
    };

    State GetCurrentState() const { return m_CurrentState; }
    GameState GetGameState() const {return m_GameState;}

    void Start();
    void Update();
    void End(); // NOLINT(readability-convert-member-functions-to-static)

private:
    // void ValidTask();

    void UpdateState00() ;
    void UpdateState01() ;
    void UpdateState01_1() ;
    void UpdateState01_2() ;
    void UpdateState01_3() ;

    State m_CurrentState = State::START;
    GameState m_GameState = GameState::STATE_00;

    Util::Renderer m_Root;

    // --- 背景與共用物件 ---
    std::shared_ptr<Character> m_WhiteBackground;
    std::shared_ptr<Character> m_Floor;

    // --- 00.png 所需物件 ---
    std::shared_ptr<Character> m_Header ;
    std::shared_ptr<GameText> m_TitleSub;
    std::shared_ptr<GameText> m_PressEnterText; // 新增: PRESS ENTER KEY
    float m_FlashTimer = 0.0f;                  // 新增: 用於控制文字閃爍的計時器

    std::shared_ptr<PlayerCat> m_BlueCat;
    std::shared_ptr<PlayerCat> m_RedCat;

    // --- 01-X.png 所需物件 ---
    std::shared_ptr<Character> m_MenuFrame;
    std::shared_ptr<GameText> m_ExitGameText;
    std::shared_ptr<GameText> m_OptionText;
    std::shared_ptr<GameText> m_LocalPlayText;

    std::shared_ptr<Character> m_ExitGameButton; // 小黑色 X
    std::shared_ptr<Character> m_Left_Tri_Button;
    std::shared_ptr<Character> m_Right_Tri_Button;
    // float m_ButtonTimer = 0.0f; // 用於控制 01-X 左右按鈕動畫停留 0.5 秒
};

#endif
