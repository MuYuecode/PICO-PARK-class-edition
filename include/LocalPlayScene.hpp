//
// Created by cody2 on 2026/3/17.
//

#ifndef PICOPART_LOCALPLAYSCENE_HPP
#define PICOPART_LOCALPLAYSCENE_HPP

#include "Scene.hpp"
#include "Character.hpp"
#include "GameText.hpp"

#include "UITriangleButton.hpp"

class KeyboardConfigScene;

class LocalPlayScene : public Scene {
public:
    LocalPlayScene(SceneServices services,
                   KeyboardConfigScene* kbConfigScene);
    ~LocalPlayScene() override = default;

    void   OnEnter() override;
    void   OnExit()  override;
    void Update()  override;

    [[nodiscard]]int GetPlayerCount() const { return m_PlayerCount; }

    static constexpr int MIN_PLAYERS = 2;
    static constexpr int MAX_PLAYERS = 8;

private:
    std::shared_ptr<Character>          m_MenuFrame;
    std::shared_ptr<Character>          m_ExitGameButton;
    std::shared_ptr<UITriangleButton> m_LeftTriButton;
    std::shared_ptr<UITriangleButton> m_RightTriButton;
    std::shared_ptr<Character>          m_BlueCatRunImg;

    std::shared_ptr<GameText> m_PlayerCountText;  // "nPLAYER GAME"
    std::shared_ptr<GameText> m_NoConfigText;     // "No keyboard config"

    int m_PlayerCount = MIN_PLAYERS;

    KeyboardConfigScene* m_KbConfigScene = nullptr;

    static const Util::Color k_Black;
    static const Util::Color k_Red;

    void UpdateDisplay() const ;
};

#endif // PICOPART_LOCALPLAYSCENE_HPP