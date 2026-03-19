#ifndef APP_HPP
#define APP_HPP

#include "pch.hpp"
#include "Util/Renderer.hpp"
#include "Character.hpp"
#include "PlayerCat.hpp"
#include "GameContext.hpp"
#include "Scene.hpp"  // Scene 是完整型別，unique_ptr<Scene> 不需要子類別定義

// 前向宣告只保留「AppStart.cpp 需要呼叫 setter 時」用到的型別
class TitleScene;
class MenuScene;
class ExitConfirmScene;
class OptionMenuScene;
class KeyboardConfigScene;
class LocalPlayScene;   // ← 取代 PlayerSelectScene
class LocalPlayGameScene;

class App {
public:
    enum class State { START, UPDATE, END };

    State GetCurrentState() const { return m_CurrentState; }

    void Start();
    void Update();
    void End();
    // ~App() 不再需要特別宣告，
    // 因為 unique_ptr<Scene> 使用完整型別，= default 在這裡就夠了

private:
    void TransitionTo(Scene* next);

    State          m_CurrentState = State::START;
    Util::Renderer m_Root;

    std::unique_ptr<GameContext> m_Ctx;

    // 改成 unique_ptr<Scene>：
    //   - Scene 已有 virtual destructor，多型 delete 正確
    //   - App.hpp 不再依賴子類別的完整定義
    //   - 消除 "incomplete type" 錯誤
    std::unique_ptr<Scene> m_TitleScene;
    std::unique_ptr<Scene> m_MenuScene;
    std::unique_ptr<Scene> m_ExitConfirmScene;
    std::unique_ptr<Scene> m_OptionMenuScene;
    std::unique_ptr<Scene> m_KeyboardConfigScene;
    std::unique_ptr<Scene> m_LocalPlayScene;   // ← 新增
    std::unique_ptr<Scene> m_LocalPlayGameScene;

    Scene* m_CurrentScene = nullptr;
};

#endif // APP_HPP