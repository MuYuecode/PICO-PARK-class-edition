#ifndef MENU_SCENE_HPP
#define MENU_SCENE_HPP

#include "Scene.hpp"
#include "Character.hpp"
#include "GameText.hpp"
#include "UI_Triangle_Button.hpp"

class ExitConfirmScene;
class OptionScene;       // 未來實作，目前只需前向宣告
class PlayerSelectScene; // 同上

/**
 * @brief 主選單場景（原 STATE_01_1 / _01_2 / _01_3）
 *
 * 職責：
 *   - 顯示選單框（Menu_Frame）、左右三角按鈕
 *   - 管理三個選項的循環切換：EXIT GAME / OPTION / LOCAL PLAY MODE
 *   - 按下 ENTER 依選項切換到對應的子場景
 *   - 按下 ESC 回到 TitleScene
 *
 * 不再需要 UpdateState01_1/2/3 的 switch/case，
 * 內部只用一個 int m_SelectedIndex 追蹤目前選到哪個選項。
 */
class MenuScene : public Scene {
public:
    MenuScene(GameContext& ctx,
              Scene* titleScene,
              ExitConfirmScene* exitConfirmScene,
              OptionScene* optionScene,
              PlayerSelectScene* playerSelectScene);
    ~MenuScene() override = default;

    void   OnEnter() override;
    void   OnExit()  override;
    Scene* Update()  override;

    // ---- setter：解決 App::Start() 裡的循環指標問題 ----
    // MenuScene 與 TitleScene 互相持有對方指標，無法在 constructor 一次傳齊，
    // 所以先用 nullptr 占位，建立完所有場景後再透過 setter 補齊。
    void SetTitleScene(Scene* s) { m_TitleScene = s; }

private:
    // ---- 此場景私有的 UI 元件 ----
    std::shared_ptr<Character>          m_MenuFrame;
    std::shared_ptr<Character>          m_ExitGameButton;
    std::shared_ptr<UI_Triangle_Button> m_LeftTriButton;
    std::shared_ptr<UI_Triangle_Button> m_RightTriButton;

    // 三個選項文字（同一位置，每次只顯示一個）
    std::shared_ptr<GameText> m_ExitGameText;
    std::shared_ptr<GameText> m_OptionText;
    std::shared_ptr<GameText> m_LocalPlayText;

    // ---- 內部狀態 ----
    // 0 = EXIT GAME, 1 = OPTION, 2 = LOCAL PLAY MODE
    int m_SelectedIndex = 0;

    // ---- 切換目標（non-owning pointers，由 App 持有）----
    Scene*             m_TitleScene        = nullptr;
    ExitConfirmScene*  m_ExitConfirmScene  = nullptr;
    OptionScene*       m_OptionScene       = nullptr;
    PlayerSelectScene* m_PlayerSelectScene = nullptr;

    // ---- 輔助函式 ----
    void ShowCurrentOption();
    void HideAllOptions();
};

#endif // MENU_SCENE_HPP