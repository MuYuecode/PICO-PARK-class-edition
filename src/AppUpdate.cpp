#include "App.hpp"
#include "Util/Logger.hpp"
 
void App::Update() {
    if (m_SceneManager == nullptr || m_SceneManager->GetCurrentScene() == nullptr) {
        LOG_ERROR("SceneManager current scene is nullptr!");
        return;
    }

    m_Ctx->BGMPlayer->Update();
    m_SceneManager->UpdateCurrent();

    if (m_Ctx->ShouldQuit) {
        m_CurrentState = State::END;
    }

    m_Root.Update();
}

