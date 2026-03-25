#ifndef EXIT_CONFIRM_SCENE_HPP
#define EXIT_CONFIRM_SCENE_HPP

#include "Scene.hpp"
#include "Character.hpp"
#include "GameText.hpp"

class MenuScene;

class ExitConfirmScene : public Scene {
public:
    ExitConfirmScene(GameContext& ctx,
                     MenuScene* menuScene,
                     std::shared_ptr<Character> menuFrame,
                     std::shared_ptr<Character> exitGameButton);
    ~ExitConfirmScene() override = default;

    void   OnEnter() override;
    void   OnExit()  override;
    Scene* Update()  override;

    void SetMenuScene(MenuScene* s) { m_MenuScene = s; }

private:
    std::shared_ptr<GameText>  m_ExitGame1Text;
    std::shared_ptr<GameText>  m_YesText;
    std::shared_ptr<GameText>  m_NoText;
    std::shared_ptr<Character> m_ChoiceFrame;

    std::shared_ptr<Character> m_MenuFrame;
    std::shared_ptr<Character> m_ExitGameButton;

    MenuScene* m_MenuScene = nullptr;

    bool m_IsYesSelected = true;

    void UpdateChoiceFramePosition() const;
};

#endif // EXIT_CONFIRM_SCENE_HPP