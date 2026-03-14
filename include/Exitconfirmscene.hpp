#ifndef EXIT_CONFIRM_SCENE_HPP
#define EXIT_CONFIRM_SCENE_HPP

#include "Scene.hpp"
#include "Character.hpp"
#include "GameText.hpp"

class MenuScene;

/**
 * @brief 退出確認對話框場景（原 STATE_01_1_1 / _01_1_2）
 *
 * 職責：
 *   - 顯示「EXIT GAME ?」文字、YES / NO 選項與選擇框
 *   - 管理游標在 YES / NO 之間切換（A/D 鍵或滑鼠 hover）
 *   - 確認離開（YES + ENTER）→ 設定 m_Ctx.ShouldQuit = true
 *   - 取消（NO + ENTER 或 ESC 或 X 按鈕）→ 返回 MenuScene
 *
 * 設計重點：
 *   - 選單框的縮放（SetScale）只在 OnEnter / OnExit 做，
 *     不再散落在各個 if/else 裡。
 *   - 唯一的狀態是 m_IsYesSelected（bool），比原本兩個 sub-state 更清楚。
 */
class ExitConfirmScene : public Scene {
public:
    ExitConfirmScene(GameContext& ctx,
                     MenuScene* menuScene,
                     std::shared_ptr<Character> menuFrame,
                     std::shared_ptr<Character> exitGameButton);
    ~ExitConfirmScene() override = default;

    void   OnEnter() override;
    void   OnExit()  override;
    Scene* Update()  override;

    // ---- setter：解決 App::Start() 裡的循環指標問題 ----
    // ExitConfirmScene 需要 MenuScene 指標，但 MenuScene 也需要
    // ExitConfirmScene 指標，建立順序上無法兩邊同時傳齊。
    void SetMenuScene(MenuScene* s) { m_MenuScene = s; }

private:
    // ---- 此場景私有的 UI 元件 ----
    std::shared_ptr<GameText>  m_ExitGame1Text;
    std::shared_ptr<GameText>  m_YesText;
    std::shared_ptr<GameText>  m_NoText;
    std::shared_ptr<Character> m_ChoiceFrame;

    // ---- 與 MenuScene 共用的元件（借用，不持有所有權）----
    // 原本的程式碼會修改 MenuFrame 的 scale 和 ExitGameButton 的 position，
    // 這裡繼續借用這兩個物件來做同樣的事。
    std::shared_ptr<Character> m_MenuFrame;
    std::shared_ptr<Character> m_ExitGameButton;

    // ---- 切換目標 ----
    // 宣告順序必須與 constructor 初始化清單順序一致，否則產生 -Wreorder 警告
    MenuScene* m_MenuScene = nullptr;

    // ---- 內部狀態 ----
    // true = 游標在 YES，false = 游標在 NO
    bool m_IsYesSelected = true;

    // ---- 輔助函式 ----
    void UpdateChoiceFramePosition();
};

#endif // EXIT_CONFIRM_SCENE_HPP