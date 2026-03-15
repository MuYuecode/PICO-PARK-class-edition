#ifndef PICOPART_TITLESCENE_HPP
#define PICOPART_TITLESCENE_HPP

#include "Scene.hpp"
#include "GameText.hpp"

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
    // m_Header 已移至 GameContext，
    // 因為它們在 MenuScene 也要顯示，不屬於 TitleScene 私有。
    // TitleScene 只負責 m_PressEnterText（只在標題畫面閃爍）。
    std::shared_ptr<GameText> m_TitleSub;
    std::shared_ptr<GameText> m_PressEnterText;

    float m_FlashTimer = 0.0f;

    MenuScene* m_MenuScene = nullptr;
};

#endif //PICOPART_TITLESCENE_HPP