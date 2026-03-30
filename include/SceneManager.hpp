#ifndef SCENE_MANAGER_HPP
#define SCENE_MANAGER_HPP

#include <memory>
#include <unordered_map>

#include "Scene.hpp"
#include "SceneId.hpp"

struct SceneIdHash {
    std::size_t operator()(SceneId id) const noexcept {
        return static_cast<std::size_t>(id);
    }
};

class SceneManager {
public:
    SceneManager() = default;

    void Register(SceneId id, std::unique_ptr<Scene> scene);
    void GoTo(SceneId id);
    SceneId UpdateCurrent();

    [[nodiscard]] SceneId GetCurrentId() const { return m_CurrentId; }
    [[nodiscard]] Scene* GetCurrentScene() const { return m_Current; }

private:
    std::unordered_map<SceneId, std::unique_ptr<Scene>, SceneIdHash> m_Scenes;
    SceneId m_CurrentId = SceneId::None;
    Scene* m_Current = nullptr;
};

#endif // SCENE_MANAGER_HPP

