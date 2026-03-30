//
// Created by cody2 on 2026/3/14.
//

#ifndef PICOPART_SCENE_HPP
#define PICOPART_SCENE_HPP

#include "SceneId.hpp"
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
    virtual SceneId Update() = 0;

protected:
    IAudioService& m_Audio;
    IVisualThemeService& m_Theme;
    ISessionState& m_Session;
    IGlobalActors& m_Actors;
};

#endif //PICOPART_SCENE_HPP