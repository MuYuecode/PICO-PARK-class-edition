//
// Created by cody2 on 2026/3/24.
//

//
// LevelSelectScene.hpp
//
// 關卡選擇場景
//
// 畫面結構(由上至下，座標以畫面中心為原點)：
//
//   LEVEL n(標題，置中，y ≈ +240)
//
//   關卡列表 2 × 5 網格(共 10 關)：
//     上排(y ≈ +60)  ：Level 1 | Level 2 | Level 3 | Level 4 | Level 5
//     下排(y ≈ -80)  ：Level 6 | Level 7 | Level 8 | Level 9 | Level 10
//     橫向格距 128 px，最左格中心 x = -256
//
//   m PLAYERS BEST TIME: mm:ss.cs(置中，y ≈ -238)
//
// 操作方式：
//   WASD / ↑↓←→       ：移動橘色選擇框
//   IsMouseHovering    ：滑鼠懸停自動切換選中關卡
//   ENTER / 滑鼠左鍵   ：進入選中關卡(對應 LevelOneScene…，尚未實作時回傳 nullptr)
//   ESC                ：返回 LocalPlayGameScene(重新進入門前準備)
//
// 皇冠與最佳時間：
//   completed = true 的關卡左上方顯示 "★"(橘色)
//   最佳時間依 m_Ctx.SelectedPlayerCount(前一場景設定)分類顯示
//   資料來源：GA_RESOURCE_DIR/Save/save_data.json(透過 SaveManager 存取)
//

#ifndef LEVEL_SELECT_SCENE_HPP
#define LEVEL_SELECT_SCENE_HPP

#include <array>
#include <memory>

#include "Scene.hpp"
#include "Character.hpp"
#include "GameText.hpp"
#include "SaveManager.hpp"

class LocalPlayGameScene;

class LevelSelectScene : public Scene {
public:
    LevelSelectScene(GameContext& ctx,
                     LocalPlayGameScene* localPlayGameScene);
    ~LevelSelectScene() override = default;

    void   OnEnter() override;
    void   OnExit()  override;
    Scene* Update()  override;

    /**
     * 設定對應關卡的場景指標(由 AppStart 呼叫)
     * @param levelIdx 0-indexed(0 = Level 1)
     * @param scene    關卡場景指標；尚未實作時傳 nullptr
     */
    // void SetLevelScene(int levelIdx, Scene* scene) {
    //     if (levelIdx >= 0 && levelIdx < LEVEL_COUNT)
    //         m_LevelScenes[levelIdx] = scene;
    // }

    // 關卡格子座標(供外部參考)
    static constexpr int   LEVEL_COUNT = 10;
    static constexpr int   COLS        = 5;
    static constexpr int   ROWS        = 2;
    static constexpr float CELL_W      = 200.0f;   // 格子橫向間距
    static constexpr float CELL_H      = 220.0f;   // 格子縱向間距
    static constexpr float GRID_MID_X  = 0.0f;  // 第 1 欄中心 X
    static constexpr float GRID_TOP_Y  = 60.0f;  // 第 1 列中心 Y

    /** 回傳第 idx 個格子的中心座標(0-indexed) */
    static glm::vec2 CellPos(int idx);

    std::vector<std::string> imagePaths = {
        "/Image/Level_Cover/LevelOne.png",
        "/Image/Level_Cover/LevelTwo.png",
        "/Image/Level_Cover/LevelThree.png",
        "/Image/Level_Cover/LevelFour.png",
        // "/Image/Level_Cover/LevelFive.png",
        // "/Image/Level_Cover/LevelSix.png",
        // "/Image/Level_Cover/LevelSeven.png",
        // "/Image/Level_Cover/LevelEight.png",
        // "/Image/Level_Cover/LevelNine.png",
        // "/Image/Level_Cover/LevelTen.png"
        "/Image/Level_Cover/LevelOne.png",
        "/Image/Level_Cover/LevelOne.png",
        "/Image/Level_Cover/LevelOne.png",
        "/Image/Level_Cover/LevelOne.png",
        "/Image/Level_Cover/LevelOne.png",
        "/Image/Level_Cover/LevelOne.png"
    };

private:
    // UI 物件
    std::shared_ptr<Character> m_SelectorFrame;  // 橘色粗框(隨選擇移動)

    std::shared_ptr<GameText>  m_TitleText;      // "LEVEL N"
    std::shared_ptr<GameText>  m_BestTimeText;   // "m PLAYERS BEST TIME: mm:ss.cs"

    std::array<std::shared_ptr<Character>, LEVEL_COUNT> m_LevelCover;  // "1"~"10"
    std::array<std::shared_ptr<Character>, LEVEL_COUNT> m_Crown;  // "★"(完成標記)

    // 場景狀態
    int m_SelectedIdx = 0;

    std::array<LevelSaveData, LEVEL_COUNT> m_LevelData;   // 從 save_data.json 載入

    //  場景連結(raw non-owning pointer)
    LocalPlayGameScene*             m_LocalPlayGameScene = nullptr;
    // std::array<Scene*, LEVEL_COUNT> m_LevelScenes;   // 各關卡場景；nullptr = 尚未實作

    //  顏色常數
    static const Util::Color kBlack;
    static const Util::Color kOrange;
    static const Util::Color kGray;

    //  輔助：UI 更新
    void UpdateSelectorPos()  const;
    void UpdateTitleText()    const;
    void UpdateBestTimeText() const;
    void UpdateCrowns()       const;
};

#endif // LEVEL_SELECT_SCENE_HPP