#ifndef MENU_SCENE_HPP
#define MENU_SCENE_HPP

#include "Scene.hpp"
#include "Character.hpp"
#include "GameText.hpp"
#include "UI_Triangle_Button.hpp"
#include "CharacterPhysicsSystem.hpp"

class ExitConfirmScene;
class OptionMenuScene;
class LocalPlayScene;

class MenuScene : public Scene {
public:
    MenuScene(GameContext& ctx,
              Scene* titleScene,
              ExitConfirmScene* exitConfirmScene,
              OptionMenuScene* optionScene,
              LocalPlayScene* playerSelectScene);
    ~MenuScene() override = default;

    void   OnEnter() override;
    void   OnExit()  override;
    Scene* Update()  override;

    void SetTitleScene(Scene* s)                   { m_TitleScene = s; }
    void SetExitConfirmScene(ExitConfirmScene* s)  { m_ExitConfirmScene = s; }
    void SetOptionScene(OptionMenuScene* s)       { m_OptionScene = s; }
    void SetLocalPlayScene(LocalPlayScene* s)     { m_LocalPlayScene = s; }

    // [新增] ExitConfirmScene 需要借用這兩個物件（修改 scale / position），
    // 而且需要知道它們是「已在渲染樹中的那組」，所以透過 getter 共享。
    // AppStart 先建立 MenuScene，再把這兩個物件傳給 ExitConfirmScene。
    std::shared_ptr<Character> GetMenuFrame()      const { return m_MenuFrame; }
    std::shared_ptr<Character> GetExitGameButton() const { return m_ExitGameButton; }
    std::shared_ptr<UI_Triangle_Button> GetLeftTriButton()  const { return m_LeftTriButton;  }  // ← 新增
    std::shared_ptr<UI_Triangle_Button> GetRightTriButton() const { return m_RightTriButton; }  // ← 新增

private:
    std::shared_ptr<Character>          m_MenuFrame;
    std::shared_ptr<Character>          m_ExitGameButton;
    std::shared_ptr<Character>          m_blue_cat_run_img;
    std::shared_ptr<UI_Triangle_Button> m_LeftTriButton;
    std::shared_ptr<UI_Triangle_Button> m_RightTriButton;

    std::shared_ptr<GameText> m_ExitGameText;
    std::shared_ptr<GameText> m_OptionText;
    std::shared_ptr<GameText> m_LocalPlayText;

    CharacterPhysicsSystem    m_Physics;
    std::vector<PhysicsAgent> m_Agents;

    int m_SelectedIndex = 0;

    Scene*             m_TitleScene        = nullptr;
    ExitConfirmScene*  m_ExitConfirmScene  = nullptr;
    OptionMenuScene*   m_OptionScene       = nullptr;
    LocalPlayScene*    m_LocalPlayScene    = nullptr;  // ← 取代 PlayerSelectScene

    void ShowCurrentOption();
    void HideAllOptions();
};

#endif // MENU_SCENE_HPP