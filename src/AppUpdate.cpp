#include "App.hpp"
#include "Util/Logger.hpp"
 
void App::Update() {
    if (m_SceneManager == nullptr || m_SceneManager->GetCurrentScene() == nullptr) {
        LOG_ERROR("SceneManager current scene is nullptr!");
        m_CurrentState = State::END;
        return;
    }

    if (m_AudioService != nullptr) {
        m_AudioService->UpdateBgm();
    }
    m_SceneManager->UpdateCurrent();

    if (m_SessionState != nullptr && m_SessionState->ShouldQuit()) {
        m_CurrentState = State::END;
    }

    m_Root.Update();
}