#ifndef PICOPART_TITLESCENE_HPP
#define PICOPART_TITLESCENE_HPP

#include <vector>

#include "Scene.hpp"
#include "GameText.hpp"
#include "CharacterPhysicsSystem.hpp"

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
    // ── UI ───────────────────────────────────────────────────────────────
    std::shared_ptr<GameText> m_TitleSub;
    std::shared_ptr<GameText> m_PressEnterText;

    float      m_FlashTimer = 0.0f;
    MenuScene* m_MenuScene  = nullptr;

    // ── 物理系統（算法）與代理人（資料）─────────────────────────────────
    CharacterPhysicsSystem      m_Physics;
    std::vector<PhysicsAgent>   m_Agents;
};

#endif // PICOPART_TITLESCENE_HPP