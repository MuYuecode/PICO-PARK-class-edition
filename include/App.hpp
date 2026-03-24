#ifndef APP_HPP
#define APP_HPP

#include "Util/Renderer.hpp"
#include "GameContext.hpp"
#include "Scene.hpp"

// 前向宣告只保留「AppStart.cpp 需要呼叫 setter 時」用到的型別
class TitleScene;
class MenuScene;
class ExitConfirmScene;
class OptionMenuScene;
class KeyboardConfigScene;
class LocalPlayScene;
class LocalPlayGameScene;
class LevelSelectScene;

class App {
public:
    enum class State { START, UPDATE, END };

    [[nodiscard]] State GetCurrentState() const { return m_CurrentState; }

    void Start();
    void Update();
    void End();

private:
    void TransitionTo(Scene* next);

    State          m_CurrentState = State::START;
    Util::Renderer m_Root;

    std::unique_ptr<GameContext> m_Ctx;

    std::unique_ptr<Scene> m_TitleScene;
    std::unique_ptr<Scene> m_MenuScene;
    std::unique_ptr<Scene> m_ExitConfirmScene;
    std::unique_ptr<Scene> m_OptionMenuScene;
    std::unique_ptr<Scene> m_KeyboardConfigScene;
    std::unique_ptr<Scene> m_LocalPlayScene;
    std::unique_ptr<Scene> m_LocalPlayGameScene;
    std::unique_ptr<Scene> m_LevelSelectScene;

    Scene* m_CurrentScene = nullptr;
};

#endif // APP_HPP