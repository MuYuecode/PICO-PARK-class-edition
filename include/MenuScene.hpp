#ifndef MENU_SCENE_HPP
#define MENU_SCENE_HPP

#include "Scene.hpp"
#include "Character.hpp"
#include "GameText.hpp"
#include "UITriangleButton.hpp"
#include "PhysicsWorld.hpp"

class ExitConfirmScene;
class OptionMenuScene;
class LocalPlayScene;

class MenuScene : public Scene {
public:
    MenuScene(GameContext& ctx,
              Scene*            titleScene,
              ExitConfirmScene* exitConfirmScene,
              OptionMenuScene*  optionScene,
              LocalPlayScene*   playerSelectScene);
    ~MenuScene() override = default;

    void   OnEnter() override;
    void   OnExit()  override;
    Scene* Update()  override;

    void SetTitleScene(Scene* s)                  { m_TitleScene       = s; }
    void SetExitConfirmScene(ExitConfirmScene* s)  { m_ExitConfirmScene = s; }
    void SetOptionScene(OptionMenuScene* s)        { m_OptionScene      = s; }
    void SetLocalPlayScene(LocalPlayScene* s)      { m_LocalPlayScene   = s; }

    [[nodiscard]] std::shared_ptr<Character>          GetMenuFrame()      const { return m_MenuFrame;      }
    [[nodiscard]] std::shared_ptr<Character>          GetExitGameButton() const { return m_ExitGameButton; }
    [[nodiscard]] std::shared_ptr<UITriangleButton>   GetLeftTriButton()  const { return m_LeftTriButton;  }
    [[nodiscard]] std::shared_ptr<UITriangleButton>   GetRightTriButton() const { return m_RightTriButton; }
    [[nodiscard]] std::shared_ptr<Character>          GetBlueCatRunImg()  const { return m_blue_cat_run_img; }

private:
    void SetupStaticBoundaries();
    void ShowCurrentOption() const;
    void HideAllOptions()    const;

    std::shared_ptr<Character>        m_MenuFrame;
    std::shared_ptr<Character>        m_ExitGameButton;
    std::shared_ptr<Character>        m_blue_cat_run_img;
    std::shared_ptr<UITriangleButton> m_LeftTriButton;
    std::shared_ptr<UITriangleButton> m_RightTriButton;

    std::shared_ptr<GameText> m_ExitGameText;
    std::shared_ptr<GameText> m_OptionText;
    std::shared_ptr<GameText> m_LocalPlayText;

    PhysicsWorld m_World;

    int m_SelectedIndex = 0;

    Scene*            m_TitleScene       = nullptr;
    ExitConfirmScene* m_ExitConfirmScene = nullptr;
    OptionMenuScene*  m_OptionScene      = nullptr;
    LocalPlayScene*   m_LocalPlayScene   = nullptr;
};

#endif // MENU_SCENE_HPP