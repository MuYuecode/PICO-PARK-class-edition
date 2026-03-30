#ifndef APP_HPP
#define APP_HPP

#include "Util/Renderer.hpp"
#include "GameContext.hpp"
#include "SceneManager.hpp"

class App {
public:
    enum class State { START, UPDATE, END };

    [[nodiscard]] State GetCurrentState() const { return m_CurrentState; }

    void Start();
    void Update();
    void End();

private:
    State          m_CurrentState = State::START;
    Util::Renderer m_Root;

    std::unique_ptr<GameContext> m_Ctx;
    std::unique_ptr<SceneManager> m_SceneManager;
};

#endif // APP_HPP