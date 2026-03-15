#ifndef MENU_SCENE_HPP
#define MENU_SCENE_HPP

#include "Scene.hpp"
#include "Character.hpp"
#include "GameText.hpp"
#include "UI_Triangle_Button.hpp"

class ExitConfirmScene;
class OptionMenuScene;
class PlayerSelectScene;

class MenuScene : public Scene {
public:
    MenuScene(GameContext& ctx,
              Scene* titleScene,
              ExitConfirmScene* exitConfirmScene,
              OptionMenuScene* optionScene,
              PlayerSelectScene* playerSelectScene);
    ~MenuScene() override = default;

    void   OnEnter() override;
    void   OnExit()  override;
    Scene* Update()  override;

    void SetTitleScene(Scene* s)                   { m_TitleScene = s; }
    void SetExitConfirmScene(ExitConfirmScene* s)  { m_ExitConfirmScene = s; }
    void SetOptionScene(OptionMenuScene* s)       { m_OptionScene = s; }
    void SetPlayerSelectScene(PlayerSelectScene* s) { m_PlayerSelectScene = s; }

    // [新增] ExitConfirmScene 需要借用這兩個物件（修改 scale / position），
    // 而且需要知道它們是「已在渲染樹中的那組」，所以透過 getter 共享。
    // AppStart 先建立 MenuScene，再把這兩個物件傳給 ExitConfirmScene。
    std::shared_ptr<Character> GetMenuFrame()      const { return m_MenuFrame; }
    std::shared_ptr<Character> GetExitGameButton() const { return m_ExitGameButton; }

private:
    std::shared_ptr<Character>          m_MenuFrame;
    std::shared_ptr<Character>          m_ExitGameButton;
    std::shared_ptr<UI_Triangle_Button> m_LeftTriButton;
    std::shared_ptr<UI_Triangle_Button> m_RightTriButton;

    std::shared_ptr<GameText> m_ExitGameText;
    std::shared_ptr<GameText> m_OptionText;
    std::shared_ptr<GameText> m_LocalPlayText;

    int m_SelectedIndex = 0;

    Scene*             m_TitleScene        = nullptr;
    ExitConfirmScene*  m_ExitConfirmScene  = nullptr;
    OptionMenuScene*   m_OptionScene       = nullptr;
    PlayerSelectScene* m_PlayerSelectScene = nullptr;

    void ShowCurrentOption();
    void HideAllOptions();
};

#endif // MENU_SCENE_HPP