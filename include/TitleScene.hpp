#ifndef PICOPART_TITLESCENE_HPP
#define PICOPART_TITLESCENE_HPP

#include <memory>
#include "Scene.hpp"
#include "GameText.hpp"
#include "PhysicsWorld.hpp"


class TitleScene : public Scene {
public:
    explicit TitleScene(SceneServices services);
    ~TitleScene() override = default;

    void   OnEnter() override;
    void   OnExit()  override;
    void Update()  override;

private:
    void SetupStaticBoundaries();

    std::shared_ptr<GameText> m_TitleSub;
    std::shared_ptr<GameText> m_PressEnterText;

    float      m_FlashTimer = 0.0f;

    PhysicsWorld m_World;
};

#endif // PICOPART_TITLESCENE_HPP