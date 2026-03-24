#include "App.hpp"
#include "Util/Logger.hpp"
 
void App::Update() {
    if (m_CurrentScene == nullptr) {
        LOG_ERROR("m_CurrentScene is nullptr!");
        return;
    }

    m_Ctx->BGMPlayer->Update();
    Scene* next = m_CurrentScene->Update();


    if (next != nullptr && next != m_CurrentScene) {
        TransitionTo(next);
    }

    // ExitConfirmScene 設定 ShouldQuit 後，App 在這裡偵測並結束
    if (m_Ctx->ShouldQuit) {
        m_CurrentState = State::END;
    }

    m_Root.Update();
}

void App::TransitionTo(Scene* next) {
    if (m_CurrentScene != nullptr) {
        m_CurrentScene->OnExit();
    }
    m_CurrentScene = next;
    if (m_CurrentScene != nullptr) {
        LOG_INFO("now is {}", typeid(*m_CurrentScene).name());
        m_CurrentScene->OnEnter();
    }
}