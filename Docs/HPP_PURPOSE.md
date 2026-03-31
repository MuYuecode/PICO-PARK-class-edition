# include/*.hpp 用途一覽（重點版）

## 核心流程與場景框架

- `App.hpp`：應用程式生命週期（`START/UPDATE/END`）與核心服務持有。
- `Scene.hpp`：場景基底抽象（`OnEnter/OnExit/Update`、`SceneOp` 發送/消費）。
- `SceneId.hpp`：場景 ID 列舉（目前已實作到 `Level03`）。
- `SceneOp.hpp`：場景切換操作資料（Overlay push/pop、重開、清堆疊切換）。
- `SceneManager.hpp`：場景註冊、堆疊管理、切換執行。
- `SceneServices.hpp`：場景依賴注入包（Audio/Theme/Session/Actors）。

## 服務與共享狀態

- `IAudioService.hpp`：背景音樂控制介面。
- `AudioService.hpp`：音樂服務實作，封裝 `BGMPlayer`。
- `BGMPlayer.hpp`：BGM 播放、切歌、音量與結束回呼處理。
- `IVisualThemeService.hpp`：視覺主題（背景）服務介面。
- `VisualThemeService.hpp`：背景主題套用/還原實作。
- `ISessionState.hpp`：跨場景執行期狀態介面（玩家數、鍵位、合作值、離開旗標）。
- `SessionState.hpp`：`ISessionState` 具體實作。
- `IGlobalActors.hpp`：全域共用 Actor 存取介面。
- `GlobalActors.hpp`：全域 Actor 容器（背景、地板、門、啟動貓等）。
- `SaveManager.hpp`：持久化存取（`settings.json`、`save_data.json`）。

## 物理與角色

- `IPhysicsBody.hpp`：物理物件協定（意圖位移、解算套用、碰撞回呼、凍結/啟用）。
- `PhysicsWorld.hpp`：場景物理中樞（兩階段更新、碰撞解算、推力查詢）。
- `StaticBody.hpp`：靜態碰撞體。
- `IPushable.hpp`：可推物件介面（需求推動人數）。
- `PushableBox.hpp`：可推箱子（重力、群推判定、缺口文字）。
- `AnimatedCharacter.hpp`：動畫角色基底（位置、縮放、朝向與動畫存取）。
- `PlayerCat.hpp`：玩家貓（動畫狀態機 + `IPhysicsBody`）。
- `CatAssets.hpp`：貓咪動畫資源路徑工具。
- `BoundaryFactory.hpp`：房間邊界預設與靜態邊界建立。
- `CharacterPhysicsSystem.hpp`：舊版物理常數集合（目前以 `PlayerCat/PhysicsWorld` 為主）。

## UI 與互動元件

- `Character.hpp`：圖片物件（換圖、定位、滑鼠命中）。
- `GameText.hpp`：文字物件（內容/顏色更新、滑鼠命中）。
- `UITriangleButton.hpp`：左右三角按鈕（按壓貼圖與回彈）。
- `AppUtil.hpp`：UI 對齊、鍵碼轉字串、矩形命中工具。
- `PlayerKeyConfig.hpp`：玩家鍵位資料與 `AllKeys()` 聚合。

## 場景實作

- `TitleScene.hpp`：標題場景（提示閃爍、進入主選單）。
- `MenuScene.hpp`：主選單（Exit/Option/Local Play 導流）。
- `ExitConfirmScene.hpp`：離開確認（YES/NO）。
- `OptionMenuScene.hpp`：選項設定（背景、音量、顯示、儲存/還原）。
- `KeyboardConfigScene.hpp`：鍵位設定（1P~8P、衝突檢查、預設鍵）。
- `LocalPlayScene.hpp`：玩家數選擇與配置檢查。
- `LocalPlayGameScene.hpp`：多人暖身（玩家生成、進門計數、合作值更新）。
- `LevelSelectScene.hpp`：選關（封面、最佳時間、皇冠、關卡路由）。
- `LevelExitScene.hpp`：關卡暫停 Overlay（返回/重試/回選關/回標題）。
- `LevelOneScene.hpp`：第一關（鑰匙、箱子、開門與通關計時）。
- `LevelTwoScene.hpp`：第二關（按鈕啟動移動木板、掉落傳送、開門通關）。
- `LevelThreeScene.hpp`：第三關（共識輸入、升降台、敵人危險區、檢查點與重生）。
