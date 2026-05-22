# 元件互動

## 每幀執行鏈

1. `AudioService::UpdateBgm()` 更新音樂播放狀態。
2. `SceneManager::UpdateCurrent()` 更新目前堆疊最上層的場景。
3. 場景可以提出一個 `SceneOp`。
4. `SceneManager` 最多套用一次場景堆疊變更。
5. `App` 檢查 `SessionState::ShouldQuit()`。
6. `Renderer` 繪製目前的演員樹。

## 場景切換合約

場景之間不直接互相呼叫。場景需要切換畫面時，透過 `RequestSceneOp(...)` 送出請求。

支援的操作：

- `PushOverlay`
- `PopOverlay`
- `RestartUnderlying`
- `ClearToAndGoTo`

只有 `SceneManager` 可以修改場景堆疊。

## 共用系統

- `IGlobalActors`：共用 root 與共用視覺物件，包含背景、地板、標題列、門與起始貓。
- `ISessionState`：玩家人數、合作推力、已套用鍵位與離開遊戲旗標。
- `IAudioService`：BGM 播放、BGM 更新與音量設定。
- `IVisualThemeService`：背景主題選擇。
- `SaveManager`：選項、鍵盤配置與關卡最佳時間持久化。

上述檔案位於 `include/systems/` 與 `src/systems/`，場景透過 `include/app/SceneServices.hpp` 取得服務參考。

## 輸入與設定流程

- `KeyboardConfigScene` 編輯每位玩家的鍵位，並寫入 `SaveManager`。
- `SessionState` 保存目前已套用的鍵位設定。
- `LocalPlayScene` 檢查目前選擇的玩家數是否都有足夠鍵位設定。
- 關卡場景在進入時讀取 `SessionState::GetSelectedPlayerCount()` 與 `GetAppliedKeyConfigs()`。

## 本地遊玩流程

`LocalPlayGameScene` 是進入關卡選擇前的合作房間：

- 依玩家數生成玩家。
- 使用全域門物件。
- 玩家需要用自己的 `up` 鍵進入開啟的門。
- 完成後切換到 `LevelSelect`。

## 關卡流程

已實作關卡大致遵循以下生命週期：

- `OnEnter`：隱藏或重設共用物件、建立本地視覺物件、清空物理世界、註冊物理物件並重設狀態。
- `Update`：處理輸入意圖、更新本地物理世界、更新遊戲狀態、更新計時文字並提出場景切換。
- `OnExit`：移除本地物件、清空物理世界，必要時恢復共用物件。

各關卡特色：

- `LevelOneScene`：合作推箱與進門過關。
- `LevelTwoScene`：按鈕啟動、移動木板、撿鑰匙、開門與全員進門。
- `LevelThreeScene`：多人共識控制、升降平台、敵人、檢查點、撿鑰匙、開門與過關。
- `LevelFourScene`：發射器生成 `BulletBody`，子彈碰撞由物理模型回報，罐子經過三段狀態後開放鑰匙與過關流程。

## 暫停覆蓋層

已實作關卡按下 `ESC` 時會透過 `PushOverlay` 開啟 `LevelExitScene`。

覆蓋層操作：

- 返回：`PopOverlay`
- 重試：`RestartUnderlying`
- 關卡選擇：`ClearToAndGoTo(LevelSelect)`
- 標題畫面：`ClearToAndGoTo(Title)`

暫停與恢復會委派給底下的場景。使用物理的關卡會呼叫 `PhysicsWorld::FreezeAll()` 與 `PhysicsWorld::UnfreezeAll()`。

## 關卡完成

關卡完成時：

1. 關卡透過 `SaveManager::UpdateBestTime(...)` 寫入最佳時間。
2. 關卡提出 `ClearToAndGoTo(LevelSelect)`。
3. `LevelSelectScene` 重新載入存檔，更新皇冠與最佳時間顯示。
