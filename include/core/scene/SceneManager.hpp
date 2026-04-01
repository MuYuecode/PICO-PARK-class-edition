#ifndef SCENE_MANAGER_HPP
#define SCENE_MANAGER_HPP

#include <memory>
#include <unordered_map>
#include <vector>

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
    void UpdateCurrent();

    [[nodiscard]] SceneId GetCurrentId() const;
    [[nodiscard]] Scene* GetCurrentScene() const;

private:
    [[nodiscard]] Scene* ResolveScene(SceneId id) const;

    void PushOverlay(SceneId id);
    void PopOverlay();
    void RestartUnderlying();
    void ClearToAndGoTo(SceneId id);

    std::unordered_map<SceneId, std::unique_ptr<Scene>, SceneIdHash> m_Scenes;
    std::vector<SceneId> m_StackIds;
};

#endif // SCENE_MANAGER_HPP


