# `include/*.hpp` 用途總覽（重點版）

## 核心流程與場景框架

- `app/App.hpp`：`App` 主流程（`START/UPDATE/END`）與核心服務持有者。
- `app/AppUtil.hpp`：UI 對齊、鍵碼文字化、矩形滑鼠命中工具。
- `core/Scene.hpp`：場景抽象基底（`OnEnter/OnExit/Update` + `SceneOp` 發送/消費）。
- `core/SceneId.hpp`：場景列舉（已實作到 `Level03`，`Level04~10` 為保留 ID）。
- `core/SceneOp.hpp`：場景切換意圖資料（Overlay push/pop、重開底層、清堆疊切換）。
- `core/SceneManager.hpp`：場景註冊、堆疊管理與切換執行。
- `core/SceneServices.hpp`：場景依賴注入封裝（Audio/Theme/Session/Actors 介面）。

## 服務與共享狀態

- `services/IAudioService.hpp`、`services/AudioService.hpp`：BGM 控制介面與實作。
- `services/BGMPlayer.hpp`：BGM 播放器（曲目輪播、音量、播放狀態更新）。
- `services/IVisualThemeService.hpp`、`services/VisualThemeService.hpp`：背景主題套用與回復。
- `services/ISessionState.hpp`、`services/SessionState.hpp`：跨場景執行期狀態（玩家數、合作推力、鍵位、離開旗標）。
- `services/IGlobalActors.hpp`、`services/GlobalActors.hpp`：共用 Actor 容器（背景、地板、門、啟動貓等）。
- `services/SaveManager.hpp`：持久化（`settings.json`、`save_data.json`）與時間格式化。

## 遊戲物件與 UI

- `gameplay/Character.hpp`：靜態圖片物件（換圖、定位、滑鼠命中）。
- `gameplay/AnimatedCharacter.hpp`：動畫角色基底（位置/縮放/朝向與動畫存取）。
- `gameplay/PlayerCat.hpp`：玩家貓（動畫狀態機 + `IPhysicsBody`）。
- `gameplay/PushableBox.hpp`：可推箱（`IPhysicsBody` + `IPushable`，顯示推力缺口文字）。
- `gameplay/IPushable.hpp`：可推物件需求介面（需要幾人推）。
- `gameplay/PlayerKeyConfig.hpp`：玩家鍵位資料與 `AllKeys()` 聚合。
- `gameplay/CatAssets.hpp`：貓咪動畫資源路徑工具。
- `gameplay/BoundaryFactory.hpp`：通用房間邊界預設與靜態邊界建立。
- `ui/GameText.hpp`：文字物件（內容/顏色更新、滑鼠命中）。
- `ui/UITriangleButton.hpp`：三角按鈕（按下貼圖與回彈計時）。

## 物理系統（核心）

- `physics/IPhysicsBody.hpp`：物理主介面（由 Transform/Material/Motion/Collision/Lifecycle/PushReactive 組成）。
- `physics/IPhysicsTransform.hpp`、`IPhysicsMaterial.hpp`、`IPhysicsMotion.hpp`、`IPhysicsCollisionListener.hpp`、`IPhysicsLifecycle.hpp`、`IPhysicsPushReactive.hpp`：物理行為分層介面。
- `physics/PhysicsBodyTraits.hpp`：`BodyType` 與行為旗標。
- `physics/PhysicsWorld.hpp`：場景物理中樞（更新排程、解算、碰撞回呼、推力查詢、凍結控制）。
- `physics/PhysicsUpdateScheduler.hpp`：兩段式 `PhysicsUpdate`（先角色/平台，後箱子）。
- `physics/PhysicsSnapshot.hpp`：每幀解算快照資料結構。
- `physics/SupportResolver.hpp`：站立支撐關係偵測（含平台偏好）。
- `physics/CollisionSolver.hpp`：碰撞解算與位移套用、碰撞事件派送。
- `physics/PushForceResolver.hpp`：遞迴推力鏈計算。
- `physics/IPushQueryService.hpp`：箱子查詢推力所需服務介面。
- `physics/StaticBody.hpp`：世界靜態碰撞體。
- `physics/LevelThreeScenePhysicsBodies.hpp`：第三關專用動態物件（升降台、巡邏敵、管道敵）。
- `physics/CharacterPhysicsSystem.hpp`：舊版常數相容層（實際運算已移至 `PlayerCat` + `PhysicsWorld`）。

## 場景檔案

- `scenes/TitleScene.hpp`：標題畫面（閃爍提示、進入主選單）。
- `scenes/MenuScene.hpp`：主選單導流（Exit / Option / Local Play）。
- `scenes/ExitConfirmScene.hpp`：離開確認（YES 設定 quit flag）。
- `scenes/OptionMenuScene.hpp`：選項（背景/音量預覽、提交或還原）。
- `scenes/KeyboardConfigScene.hpp`：鍵位設定（1P~8P、衝突檢查、預設鍵、同步 Session+Save）。
- `scenes/LocalPlayScene.hpp`：玩家數選擇與配置數量檢查。
- `scenes/LocalPlayGameScene.hpp`：多人暖身區（玩家生成、進門統計、合作推力更新）。
- `scenes/LevelSelectScene.hpp`：選關（10 格、目前前 3 關可進入、最佳時間/皇冠顯示）。
- `scenes/LevelExitScene.hpp`：關卡暫停 Overlay（返回/重試/回選關/回標題）。
- `scenes/LevelOneScene.hpp`：第一關（箱子推力、拿鑰匙開門、多人進門通關）。
- `scenes/LevelTwoScene.hpp`：第二關（按鈕伸展木板、掉落傳送、鑰匙開門通關）。
- `scenes/LevelThreeScene.hpp`：第三關（共識輸入、升降台與敵人、檢查點/死亡重生）。
