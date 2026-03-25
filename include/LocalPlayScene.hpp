//
// Created by cody2 on 2026/3/17.
//

#ifndef PICOPART_LOCALPLAYSCENE_HPP
#define PICOPART_LOCALPLAYSCENE_HPP

#include "Scene.hpp"
#include "Character.hpp"
#include "GameText.hpp"

#include "UI_Triangle_Button.hpp"

class MenuScene;
class KeyboardConfigScene;
class LocalPlayGameScene;

class LocalPlayScene : public Scene {
public:
    LocalPlayScene(GameContext& ctx,
                   MenuScene* menuScene,
                   std::shared_ptr<Character>          menuFrame,
                   std::shared_ptr<Character>          exitGameButton,
                   std::shared_ptr<UI_Triangle_Button> leftTriButton,
                   std::shared_ptr<UI_Triangle_Button> rightTriButton,
                   std::shared_ptr<Character>          blueCatRunImg,
                   KeyboardConfigScene* kbConfigScene);
    ~LocalPlayScene() override = default;

    void   OnEnter() override;
    void   OnExit()  override;
    Scene* Update()  override;

    void SetMenuScene(MenuScene* s) { m_MenuScene = s; }
    void SetGameScene(LocalPlayGameScene* s) { m_GameScene = s; }

    [[nodiscard]]int GetPlayerCount() const { return m_PlayerCount; }

    static constexpr int MIN_PLAYERS = 2;
    static constexpr int MAX_PLAYERS = 8;

private:
    std::shared_ptr<Character>          m_MenuFrame;
    std::shared_ptr<Character>          m_ExitGameButton;
    std::shared_ptr<UI_Triangle_Button> m_LeftTriButton;
    std::shared_ptr<UI_Triangle_Button> m_RightTriButton;
    std::shared_ptr<Character>          m_BlueCatRunImg;

    std::shared_ptr<GameText> m_PlayerCountText;  // "nPLAYER GAME"
    std::shared_ptr<GameText> m_NoConfigText;     // "No keyboard config"

    int m_PlayerCount = MIN_PLAYERS;

    MenuScene*           m_MenuScene     = nullptr;
    KeyboardConfigScene* m_KbConfigScene = nullptr;
    LocalPlayGameScene*  m_GameScene     = nullptr;

    static const Util::Color k_Black;
    static const Util::Color k_Red;

    void UpdateDisplay() const ;
};

#endif // PICOPART_LOCALPLAYSCENE_HPP