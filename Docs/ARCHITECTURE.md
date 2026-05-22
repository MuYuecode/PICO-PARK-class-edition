# 架構總覽

## 專案分類

目前專案採用較簡化的小型遊戲結構，`include/` 與 `src/` 都維持相同的四個主要分類：

- `app`：程式進入點、App 生命週期、場景系統與場景切換資料型別。
- `scenes`：所有畫面與關卡，例如標題畫面、選單、設定、關卡選擇與 Level01 到 Level04。
- `game`：遊戲物件與 UI 元件，例如角色、玩家貓、箱子、文字與三角按鈕。
- `systems`：跨場景系統與物理系統，例如音樂、存檔、全域演員、Session 狀態、PhysicsWorld 與碰撞解算。

這個分類的目標是降低跳檔成本。專案已完成主要遊戲內容，因此現在優先讓檔案位置直覺、容易維護，而不是保留過細的技術分層。

## 執行流程

- `src/app/main.cpp` 是程式進入點，負責建立並執行 `App`。
- `App` 是一個簡單狀態機，流程為 `START -> UPDATE -> END`。
- `App::Update()` 每一幀的順序為：
  1. `AudioService::UpdateBgm()`
  2. `SceneManager::UpdateCurrent()`
  3. 透過 `SessionState::ShouldQuit()` 檢查是否離開遊戲
  4. `Renderer::Update()`

場景切換採用意圖式設計。場景只會提出一個 `SceneOp`，由 `SceneManager` 在場景更新後統一消化並套用。

## 組合根

`App::Start()` 會建立整個遊戲生命週期共用的系統：

- `GlobalActors`
- `SessionState`
- `SceneManager`
- `BGMPlayer` 與 `AudioService`
- `VisualThemeService`

它也會建立共用視覺物件並交給 `GlobalActors` 管理：

- 背景
- 全域地板
- 標題列
- 門
- 起始畫面的貓

場景透過 `SceneServices` 取得這些依賴，因此各場景不需要自行建立全域系統。

## 已註冊場景

`App::Start()` 目前註冊：

- `Title`
- `Menu`
- `ExitConfirm`
- `OptionMenu`
- `KeyboardConfig`
- `LocalPlay`
- `LocalPlayGame`
- `LevelSelect`
- `LevelExit`
- `Level01`
- `Level02`
- `Level03`
- `Level04`

`SceneId::Level05` 到 `SceneId::Level10` 保留給未來擴充，目前沒有註冊。

## 關卡選擇

`LevelSelectScene` 顯示 10 個關卡格：

- 第 0 格對應 `Level01`。
- 第 1 格對應 `Level02`。
- 第 2 格對應 `Level03`。
- 第 3 格對應 `Level04`。
- 其餘格子預設為 `SceneId::None`，除非之後明確指定。

目前封面圖包含 `LevelOne.png`、`LevelTwo.png`、`LevelThree.png` 與 `LevelFour.png`。

## 場景堆疊模型

`SceneManager` 擁有所有場景實例，並用 `SceneId` 堆疊管理目前流程。

- `GoTo`：離開整個目前堆疊，進入目標場景。
- `ClearToAndGoTo`：清空堆疊後前往目標場景，常用於過關與回選單。
- `PushOverlay`：暫停目前場景，推入覆蓋場景。
- `PopOverlay`：離開覆蓋場景，恢復底下場景。
- `RestartUnderlying`：離開覆蓋場景，重啟底下場景。

場景不會直接改動堆疊，只能透過 `RequestSceneOp(...)` 提出切換需求。

## 權責邊界

- `App` 擁有長生命週期系統。
- `SceneManager` 擁有場景實例與場景堆疊。
- 每個關卡場景擁有自己的 `PhysicsWorld`。
- `GlobalActors` 擁有跨場景共用的視覺物件。
- `SaveManager` 集中處理設定、鍵位與最佳時間的 JSON 存取。

## 遊戲路線

主要流程：

`Title -> Menu -> LocalPlay -> LocalPlayGame -> LevelSelect -> Level01/Level02/Level03/Level04`

已實作關卡：

- `LevelOneScene`：合作推箱關卡。
- `LevelTwoScene`：按鈕、移動木板、鑰匙、門與多人進門。
- `LevelThreeScene`：多人共識操作、升降平台、敵人、檢查點、鑰匙與過關。
- `LevelFourScene`：子彈發射器、罐子狀態、鑰匙解鎖、進門與最佳時間保存。

所有已實作關卡都能用 `ESC` 開啟 `LevelExitScene`。

## 存檔

`SaveManager` 寫入：

- `Resources/Save/settings.json`：選項設定與鍵盤配置。
- `Resources/Save/save_data.json`：2P 到 8P 的各關最佳時間。

關卡完成時會先呼叫 `SaveManager::UpdateBestTime(levelIndex, playerCount, elapsedSeconds)`，再返回 `LevelSelect`。
