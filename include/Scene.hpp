//
// Created by cody2 on 2026/3/14.
//

#ifndef PICOPART_SCENE_HPP
#define PICOPART_SCENE_HPP

#include "GameContext.hpp"
#include "SceneId.hpp"
class Scene {
public:
    explicit Scene(GameContext& ctx) : m_Ctx(ctx) {}

    virtual ~Scene() = default;

    Scene(const Scene&)            = delete;
    Scene& operator=(const Scene&) = delete;
    Scene(Scene&&)                 = delete;
    Scene& operator=(Scene&&)      = delete;

    virtual void OnEnter() = 0;
    virtual void OnExit()  = 0;
    virtual SceneId Update() = 0;

protected:
    GameContext& m_Ctx;
};

#endif //PICOPART_SCENE_HPP