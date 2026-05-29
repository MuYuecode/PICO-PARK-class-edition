#ifndef MENU_SCENE_HPP
#define MENU_SCENE_HPP

#include "app/Scene.hpp"
#include "app/SceneActorGroup.hpp"
#include "game/Character.hpp"
#include "game/GameText.hpp"
#include "game/UITriangleButton.hpp"
#include "systems/PhysicsWorld.hpp"

class MenuScene : public Scene {
public:
    explicit MenuScene(SceneServices services);
    ~MenuScene() override = default;

    void   OnEnter() override;
    void   OnExit()  override;
    void Update()  override;


private:
    void SetupStaticBoundaries();
    void ShowCurrentOption() const;
    void HideAllOptions()    const;

    SceneActorGroup m_ActorGroup;

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
