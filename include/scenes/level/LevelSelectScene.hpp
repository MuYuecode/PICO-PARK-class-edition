//
// Created by cody2 on 2026/3/24.
//

#ifndef LEVEL_SELECT_SCENE_HPP
#define LEVEL_SELECT_SCENE_HPP

#include <array>
#include <memory>

#include "core/scene/Scene.hpp"
#include "gameplay/actors/Character.hpp"
#include "ui/text/GameText.hpp"
#include "services/persistence/SaveManager.hpp"

class LevelSelectScene : public Scene {
public:
    explicit LevelSelectScene(SceneServices services);
    ~LevelSelectScene() override = default;

    void   OnEnter() override;
    void   OnExit()  override;
    void Update()  override;

    void SetLevelSceneId(int levelIdx, SceneId sceneId) {
        if (levelIdx >= 0 && levelIdx < LEVEL_COUNT)
            m_LevelSceneIds[levelIdx] = sceneId;
    }

    static constexpr int   LEVEL_COUNT = 10;
    static constexpr int   COLS        = 5;
    static constexpr int   ROWS        = 2;
    static constexpr float CELL_W      = 200.0f;
    static constexpr float CELL_H      = 220.0f;
    static constexpr float GRID_MID_X  = 0.0f;
    static constexpr float GRID_TOP_Y  = 60.0f;

    static glm::vec2 CellPos(int idx);

    std::vector<std::string> imagePaths = {
        "/Image/Level_Cover/LevelOne.png",
        "/Image/Level_Cover/LevelTwo.png",
        "/Image/Level_Cover/LevelThree.png",
        "/Image/Level_Cover/LevelFour.png",
        "/Image/Level_Cover/LevelOne.png", // ignore
        "/Image/Level_Cover/LevelOne.png", // ignore
        "/Image/Level_Cover/LevelOne.png", // ignore
        "/Image/Level_Cover/LevelOne.png", // ignore
        "/Image/Level_Cover/LevelOne.png", // ignore
        "/Image/Level_Cover/LevelOne.png"  // ignore
    };

private:
    std::shared_ptr<Character> m_SelectorFrame;

    std::shared_ptr<GameText>  m_TitleText;
    std::shared_ptr<GameText>  m_BestTimeText;

    std::array<std::shared_ptr<Character>, LEVEL_COUNT> m_LevelCover;
    std::array<std::shared_ptr<Character>, LEVEL_COUNT> m_Crown;

    int m_SelectedIdx = 0;

    std::array<LevelSaveData, LEVEL_COUNT> m_LevelData;

    std::array<SceneId, LEVEL_COUNT> m_LevelSceneIds{};

    void UpdateSelectorPos()  const;
    void UpdateTitleText()    const;
    void UpdateBestTimeText() const;
    void UpdateCrowns()       const;
};

#endif // LEVEL_SELECT_SCENE_HPP
