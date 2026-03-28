#ifndef PICOPART_TITLESCENE_HPP
#define PICOPART_TITLESCENE_HPP

#include <vector>
#include <memory>
#include "Scene.hpp"
#include "GameText.hpp"
#include "PhysicsWorld.hpp"

class MenuScene;

class TitleScene : public Scene {
public:
    TitleScene(GameContext& ctx, MenuScene* menuScene);
    ~TitleScene() override = default;

    void   OnEnter() override;
    void   OnExit()  override;
    Scene* Update()  override;

    void SetMenuScene(MenuScene* s) { m_MenuScene = s; }

private:
    void SetupStaticBoundaries();

    std::shared_ptr<GameText> m_TitleSub;
    std::shared_ptr<GameText> m_PressEnterText;

    float      m_FlashTimer = 0.0f;
    MenuScene* m_MenuScene  = nullptr;

    PhysicsWorld m_World;
};

#endif // PICOPART_TITLESCENE_HPP