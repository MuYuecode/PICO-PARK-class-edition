#include "SceneManager.hpp"
#include "Util/Logger.hpp"

void SceneManager::Register(SceneId id, std::unique_ptr<Scene> scene) {
    if (scene == nullptr) {
        LOG_ERROR("SceneManager::Register got nullptr for SceneId {}", static_cast<int>(id));
        return;
    }
    m_Scenes[id] = std::move(scene);
}

Scene* SceneManager::ResolveScene(SceneId id) const {
    const auto it = m_Scenes.find(id);
    if (it == m_Scenes.end() || it->second == nullptr) {
        return nullptr;
    }
    return it->second.get();
}

Scene* SceneManager::GetCurrentScene() const {
    if (m_StackIds.empty()) {
        return nullptr;
    }
    return ResolveScene(m_StackIds.back());
}

SceneId SceneManager::GetCurrentId() const {
    if (m_StackIds.empty()) {
        return SceneId::None;
    }
    return m_StackIds.back();
}

void SceneManager::GoTo(SceneId id) {
    Scene* target = ResolveScene(id);
    if (target == nullptr) {
        LOG_ERROR("SceneManager::GoTo failed, SceneId {} not registered", static_cast<int>(id));
        return;
    }

    // Exit all stacked scenes from top to bottom for consistent lifecycle.
    while (!m_StackIds.empty()) {
        Scene* top = ResolveScene(m_StackIds.back());
        if (top != nullptr) {
            top->OnExit();
        }
        m_StackIds.pop_back();
    }

    m_StackIds.push_back(id);
    target->OnEnter();
}

void SceneManager::PushOverlay(SceneId id) {
    if (m_StackIds.empty()) {
        LOG_ERROR("SceneManager::PushOverlay requires at least one underlying scene");
        return;
    }

    Scene* top = ResolveScene(m_StackIds.back());
    Scene* overlay = ResolveScene(id);
    if (top == nullptr || overlay == nullptr) {
        LOG_ERROR("SceneManager::PushOverlay failed, invalid top/overlay id={}", static_cast<int>(id));
        return;
    }

    top->PauseGameplay();
    m_StackIds.push_back(id);
    overlay->OnEnter();
}

void SceneManager::PopOverlay() {
    if (m_StackIds.size() < 2) {
        LOG_ERROR("SceneManager::PopOverlay requires stack size >= 2, got {}", static_cast<int>(m_StackIds.size()));
        return;
    }

    Scene* overlay = ResolveScene(m_StackIds.back());
    if (overlay == nullptr) {
        LOG_ERROR("SceneManager::PopOverlay failed, overlay scene missing");
        return;
    }

    overlay->OnExit();
    m_StackIds.pop_back();

    Scene* newTop = ResolveScene(m_StackIds.back());
    if (newTop == nullptr) {
        LOG_ERROR("SceneManager::PopOverlay failed, new top scene missing after pop");
        return;
    }

    newTop->ResumeGameplay();
}

void SceneManager::RestartUnderlying() {
    if (m_StackIds.size() < 2) {
        LOG_ERROR("SceneManager::RestartUnderlying requires stack size >= 2, got {}", static_cast<int>(m_StackIds.size()));
        return;
    }

    Scene* overlay = ResolveScene(m_StackIds.back());
    if (overlay == nullptr) {
        LOG_ERROR("SceneManager::RestartUnderlying failed, overlay scene missing");
        return;
    }

    overlay->OnExit();
    m_StackIds.pop_back();

    Scene* underlying = ResolveScene(m_StackIds.back());
    if (underlying == nullptr) {
        LOG_ERROR("SceneManager::RestartUnderlying failed, underlying scene missing after pop");
        return;
    }

    underlying->OnExit();
    underlying->OnEnter();
    // Retry should leave the restarted scene immediately playable.
    underlying->ResumeGameplay();
}

void SceneManager::ClearToAndGoTo(SceneId id) {
    Scene* target = ResolveScene(id);
    if (target == nullptr) {
        LOG_ERROR("SceneManager::ClearToAndGoTo failed, target SceneId {} not registered", static_cast<int>(id));
        return;
    }

    while (!m_StackIds.empty()) {
        Scene* top = ResolveScene(m_StackIds.back());
        if (top != nullptr) {
            top->OnExit();
        }
        m_StackIds.pop_back();
    }

    m_StackIds.push_back(id);
    target->OnEnter();
}

void SceneManager::UpdateCurrent() {
    Scene* current = GetCurrentScene();
    if (current == nullptr) {
        return;
    }

    current->Update();
    const auto op = current->ConsumeSceneOp();

    if (!op.has_value()) {
        return;
    }

    switch (op->type) {
        case SceneOpType::PushOverlay:
            PushOverlay(op->target);
            break;
        case SceneOpType::PopOverlay:
            PopOverlay();
            break;
        case SceneOpType::RestartUnderlying:
            RestartUnderlying();
            break;
        case SceneOpType::ClearToAndGoTo:
            ClearToAndGoTo(op->target);
            break;
        case SceneOpType::None:
            break;
    }
}

