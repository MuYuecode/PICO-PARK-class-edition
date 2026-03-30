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

    Scene* current = GetCurrentScene();
    if (current != nullptr) {
        current->OnExit();
    }

    m_StackIds.clear();
    m_StackIds.push_back(id);
    target->OnEnter();
}

void SceneManager::PushOverlay(SceneId id) {
    if (m_StackIds.size() != 1) {
        LOG_ERROR("SceneManager::PushOverlay requires one base scene, stack size={}", static_cast<int>(m_StackIds.size()));
        return;
    }

    Scene* base = ResolveScene(m_StackIds.front());
    Scene* overlay = ResolveScene(id);
    if (base == nullptr || overlay == nullptr) {
        LOG_ERROR("SceneManager::PushOverlay failed, invalid base/overlay id={}", static_cast<int>(id));
        return;
    }

    base->PauseGameplay();
    m_StackIds.push_back(id);
    overlay->OnEnter();
}

void SceneManager::PopOverlay() {
    if (m_StackIds.size() != 2) {
        LOG_ERROR("SceneManager::PopOverlay requires overlay stack size 2, got {}", static_cast<int>(m_StackIds.size()));
        return;
    }

    Scene* overlay = ResolveScene(m_StackIds.back());
    Scene* base = ResolveScene(m_StackIds.front());
    if (overlay == nullptr || base == nullptr) {
        LOG_ERROR("SceneManager::PopOverlay failed, base or overlay scene missing");
        return;
    }

    overlay->OnExit();
    m_StackIds.pop_back();
    base->ResumeGameplay();
}

void SceneManager::RestartUnderlying() {
    if (m_StackIds.size() != 2) {
        LOG_ERROR("SceneManager::RestartUnderlying requires overlay stack size 2, got {}", static_cast<int>(m_StackIds.size()));
        return;
    }

    Scene* overlay = ResolveScene(m_StackIds.back());
    Scene* base = ResolveScene(m_StackIds.front());
    if (overlay == nullptr || base == nullptr) {
        LOG_ERROR("SceneManager::RestartUnderlying failed, base or overlay scene missing");
        return;
    }

    overlay->OnExit();
    m_StackIds.pop_back();

    base->OnExit();
    base->OnEnter();
}

void SceneManager::ClearToAndGoTo(SceneId id) {
    Scene* target = ResolveScene(id);
    if (target == nullptr) {
        LOG_ERROR("SceneManager::ClearToAndGoTo failed, target SceneId {} not registered", static_cast<int>(id));
        return;
    }

    if (m_StackIds.size() == 2) {
        Scene* overlay = ResolveScene(m_StackIds.back());
        if (overlay != nullptr) {
            overlay->OnExit();
        }
        m_StackIds.pop_back();
    }

    if (m_StackIds.size() == 1) {
        Scene* base = ResolveScene(m_StackIds.back());
        if (base != nullptr) {
            base->OnExit();
        }
        m_StackIds.pop_back();
    }

    m_StackIds.push_back(id);
    target->OnEnter();
}

SceneId SceneManager::UpdateCurrent() {
    Scene* current = GetCurrentScene();
    if (current == nullptr) {
        return SceneId::None;
    }

    const SceneId next = current->Update();
    const auto op = current->ConsumeSceneOp();

    if (op.has_value()) {
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
        return SceneId::None;
    }

    if (next != SceneId::None && next != GetCurrentId()) {
        GoTo(next);
    }

    return next;
}

