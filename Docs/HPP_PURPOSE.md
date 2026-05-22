# Header 用途速查

本文件整理 `include/` 底下各公開 header 的用途。專案目前採用 `app / scenes / game / systems` 四分類。

## App

- `app/App.hpp`：擁有程式層級系統，驅動 `START/UPDATE/END` App 狀態。
- `app/AppUtil.hpp`：共用 UI 與 sprite 輔助函式，包含按鍵名稱轉換、碰撞測試與 `Character` 對齊輔助。
- `app/Scene.hpp`：場景基底介面，提供生命週期 hook 與 pending `SceneOp`。
- `app/SceneId.hpp`：已註冊與保留場景的 enum ID。
- `app/SceneManager.hpp`：擁有場景、維護場景堆疊並套用場景操作。
- `app/SceneOp.hpp`：描述場景堆疊操作的資料型別。
- `app/SceneServices.hpp`：傳入場景的依賴集合。

## Game

- `game/Character.hpp`：圖片型視覺物件。
- `game/AnimatedCharacter.hpp`：動畫圖片物件。
- `game/PlayerCat.hpp`：可控制的貓角色，同時實作 `IPhysicsBody`。
- `game/PushableBox.hpp`：可推動箱子，同時實作 `IPhysicsBody` 與 `IPushable`。
- `game/IPushable.hpp`：可推動遊戲物件的介面。
- `game/PlayerKeyConfig.hpp`：單一玩家鍵位設定資料型別。
- `game/CatAssets.hpp`：建立貓動畫素材路徑的輔助函式。
- `game/BoundaryFactory.hpp`：建立靜態邊界物件的輔助函式。
- `game/GameText.hpp`：可渲染文字物件。
- `game/UITriangleButton.hpp`：選單使用的三角按鈕 UI 元件。

## Systems

- `systems/IAudioService.hpp`：音訊服務介面。
- `systems/AudioService.hpp`：面向場景的 BGM 音訊服務實作。
- `systems/BGMPlayer.hpp`：較底層的 BGM 播放包裝。
- `systems/IVisualThemeService.hpp`：視覺主題服務介面。
- `systems/VisualThemeService.hpp`：背景主題服務實作。
- `systems/ISessionState.hpp`：Session 狀態介面。
- `systems/SessionState.hpp`：玩家數、鍵位設定、合作推力與離開遊戲狀態。
- `systems/IGlobalActors.hpp`：共用演員存取介面。
- `systems/GlobalActors.hpp`：root、背景、地板、標題列、門與起始貓等共用物件儲存。
- `systems/SaveManager.hpp`：設定、鍵位與關卡最佳時間的 JSON 持久化。
- `systems/IPhysicsBody.hpp`：整合型物理物件介面。
- `systems/IPhysicsTransform.hpp`：位置與 half-size 介面。
- `systems/IPhysicsMaterial.hpp`：實體與 kinematic 旗標介面。
- `systems/IPhysicsMotion.hpp`：每幀移動意圖與已解析位移介面。
- `systems/IPhysicsCollisionListener.hpp`：碰撞回呼介面。
- `systems/IPhysicsLifecycle.hpp`：啟用、凍結與 post-update 生命週期介面。
- `systems/IPhysicsPushReactive.hpp`：推動反應 hook。
- `systems/PhysicsBodyTraits.hpp`：`BodyType` 與行為旗標。
- `systems/PhysicsWorld.hpp`：本地物理世界，負責物件註冊、靜態邊界擁有、更新流程與推動查詢。
- `systems/PhysicsUpdateScheduler.hpp`：兩階段 `PhysicsUpdate()` 排程器。
- `systems/PhysicsSnapshot.hpp`：碰撞解析使用的每幀物件快照。
- `systems/CollisionSolver.hpp`：支撐感知的 AABB 碰撞解析與碰撞回呼發送。
- `systems/SupportResolver.hpp`：解析前的 riding/support 偵測。
- `systems/PushForceResolver.hpp`：遞迴計算合作推力。
- `systems/IPushQueryService.hpp`：`PushableBox` 使用的推動查詢介面。
- `systems/StaticBody.hpp`：由物理世界擁有的實體靜態碰撞物。
- `systems/BulletBody.hpp`：LevelFour 使用的移動子彈物件，會記錄碰撞命中類型。
- `systems/LevelThreeScenePhysicsBodies.hpp`：LevelThree 專用的升降平台與敵人物理物件定義。
- `systems/CharacterPhysicsSystem.hpp`：保留在專案中的舊版角色物理整合輔助。

## Scenes

- `scenes/TitleScene.hpp`：標題畫面。
- `scenes/MenuScene.hpp`：主選單，前往本地遊玩、選項與離開。
- `scenes/ExitConfirmScene.hpp`：離開遊戲確認覆蓋層。
- `scenes/OptionMenuScene.hpp`：主題與音量選項。
- `scenes/KeyboardConfigScene.hpp`：玩家 1 到 8 的鍵盤配置編輯器。
- `scenes/LocalPlayScene.hpp`：本地玩家人數選擇與設定檢查。
- `scenes/LocalPlayGameScene.hpp`：進入關卡選擇前的合作房間。
- `scenes/LevelSelectScene.hpp`：10 格關卡選擇畫面，顯示最佳時間與皇冠。
- `scenes/LevelExitScene.hpp`：關卡中的暫停覆蓋層。
- `scenes/LevelOneScene.hpp`：Level01 遊戲場景。
- `scenes/LevelTwoScene.hpp`：Level02 遊戲場景。
- `scenes/LevelThreeScene.hpp`：Level03 遊戲場景。
- `scenes/LevelFourScene.hpp`：Level04 遊戲場景，包含發射器、子彈、罐子狀態、鑰匙與進門過關。
