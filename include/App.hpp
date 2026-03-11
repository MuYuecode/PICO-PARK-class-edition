#ifndef APP_HPP
#define APP_HPP

#include "pch.hpp" // IWYU pragma: export

#include "Util/Renderer.hpp"
#include "Character.hpp"
#include "Util/Text.hpp"
#include "AnimatedCharacter.hpp"
#include "PhaseResourceManger.hpp"
#include "GameText.hpp"

class App {
public:
    enum class State {
        START,
        UPDATE,
        END,
    };

    State GetCurrentState() const { return m_CurrentState; }

    void Start();

    void Update();

    void End(); // NOLINT(readability-convert-member-functions-to-static)

private:
    void ValidTask();

    State m_CurrentState = State::START;

    Util::Renderer m_Root;

    std::shared_ptr<Character> m_WhiteBackground;
    std::shared_ptr<Character> m_Floor;
    // std::shared_ptr<AnimatedCharacter> m_BlueCat;
    std::shared_ptr<Character> m_BlueCat ;
    // std::shared_ptr<Character> m_Header; // use image
    std::shared_ptr<GameText> m_TitleMain;
    std::shared_ptr<GameText> m_TitleLine;
    std::shared_ptr<GameText> m_TitleSub;

    bool m_EnterDown = false;
};

#endif
