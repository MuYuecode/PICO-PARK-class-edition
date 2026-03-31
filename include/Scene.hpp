//
// Created by cody2 on 2026/3/14.
//

#ifndef PICOPART_SCENE_HPP
#define PICOPART_SCENE_HPP

#include <optional>

#include "SceneId.hpp"
#include "SceneOp.hpp"
#include "SceneServices.hpp"
class Scene {
public:
    explicit Scene(SceneServices services)
        : m_Audio(services.Audio)
        , m_Theme(services.Theme)
        , m_Session(services.Session)
        , m_Actors(services.Actors) {}

    virtual ~Scene() = default;

    Scene(const Scene&)            = delete;
    Scene& operator=(const Scene&) = delete;
    Scene(Scene&&)                 = delete;
    Scene& operator=(Scene&&)      = delete;

    virtual void OnEnter() = 0;
    virtual void OnExit()  = 0;
    virtual void Update() = 0;

    virtual void PauseGameplay() {}
    virtual void ResumeGameplay() {}

    std::optional<SceneOp> ConsumeSceneOp() {
        const auto op = m_PendingSceneOp;
        m_PendingSceneOp.reset();
        return op;
    }

protected:
    IAudioService& m_Audio;
    IVisualThemeService& m_Theme;
    ISessionState& m_Session;
    IGlobalActors& m_Actors;

    void RequestSceneOp(const SceneOp op) {
        m_PendingSceneOp = op;
    }

private:
    std::optional<SceneOp> m_PendingSceneOp;
};

#endif //PICOPART_SCENE_HPP