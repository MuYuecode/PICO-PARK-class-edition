#include "SceneManager.hpp"
#include "Util/Logger.hpp"


void SceneManager::Register(SceneId id, std::unique_ptr<Scene> scene) {
    if (scene == nullptr) {
        LOG_ERROR("SceneManager::Register got nullptr for SceneId {}", static_cast<int>(id));
        return;
    }
    m_Scenes[id] = std::move(scene);
}

void SceneManager::GoTo(SceneId id) {
    const auto it = m_Scenes.find(id);
    if (it == m_Scenes.end() || it->second == nullptr) {
        LOG_ERROR("SceneManager::GoTo failed, SceneId {} not registered", static_cast<int>(id));
        return;
    }

    if (m_Current != nullptr) {
        m_Current->OnExit();
    }

    m_CurrentId = id;
    m_Current = it->second.get();
    m_Current->OnEnter();
}

SceneId SceneManager::UpdateCurrent() {
    if (m_Current == nullptr) {
        return SceneId::None;
    }

    const SceneId next = m_Current->Update();
    if (next != SceneId::None && next != m_CurrentId) {
        GoTo(next);
    }

    return next;
}

