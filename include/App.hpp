#ifndef APP_HPP
#define APP_HPP

#include <memory>

#include "AudioService.hpp"
#include "BGMPlayer.hpp"
#include "GlobalActors.hpp"
#include "SessionState.hpp"
#include "Util/Renderer.hpp"
#include "VisualThemeService.hpp"
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

    std::shared_ptr<BGMPlayer> m_BgmPlayer;
    std::unique_ptr<GlobalActors> m_GlobalActors;
    std::unique_ptr<SessionState> m_SessionState;
    std::unique_ptr<AudioService> m_AudioService;
    std::unique_ptr<VisualThemeService> m_ThemeService;
    std::unique_ptr<SceneManager> m_SceneManager;
};

#endif // APP_HPP