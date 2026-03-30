#ifndef MENU_SCENE_HPP
#define MENU_SCENE_HPP

#include "Scene.hpp"
#include "Character.hpp"
#include "GameText.hpp"
#include "UITriangleButton.hpp"
#include "PhysicsWorld.hpp"

class MenuScene : public Scene {
public:
    explicit MenuScene(GameContext& ctx);
    ~MenuScene() override = default;

    void   OnEnter() override;
    void   OnExit()  override;
    SceneId Update()  override;

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

};

#endif // MENU_SCENE_HPP