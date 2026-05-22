# 類別關係

## 遊戲物件階層

```text
Util::GameObject
  Character
    PushableBox (+ IPhysicsBody, + IPushable)
    UITriangleButton
  AnimatedCharacter
    PlayerCat (+ IPhysicsBody)
  GameText
```

說明：

- `Character` 是圖片型遊戲物件的基底。
- `AnimatedCharacter` 是動畫型 `GameObject` 基底，不繼承 `Character`。
- `PlayerCat` 同時是動畫角色與物理物件。
- `PushableBox` 同時是可渲染物件、`IPhysicsBody` 與目前唯一的 `IPushable` 實作。
- `GameText` 是文字顯示物件，用於 UI 與遊戲資訊。

相關檔案集中在 `include/game/` 與 `src/game/`。

## 場景階層

```text
Scene
  TitleScene
  MenuScene
  ExitConfirmScene
  OptionMenuScene
  KeyboardConfigScene
  LocalPlayScene
  LocalPlayGameScene
  LevelSelectScene
  LevelExitScene
  LevelOneScene
  LevelTwoScene
  LevelThreeScene
  LevelFourScene
```

每個場景至少實作 `OnEnter()`、`OnExit()` 與 `Update()`。會被 `LevelExitScene` 暫停的遊戲場景，也會覆寫 `PauseGameplay()` 與 `ResumeGameplay()`。

場景基底與場景管理器放在 `include/app/`，實際場景放在 `include/scenes/` 與 `src/scenes/`。

## 物理物件實作

```text
IPhysicsBody
  PlayerCat
  PushableBox
  StaticBody
  BulletBody
  LevelTwoScene::MovingPlankBody
  LevelThreeScene::MovingLiftBody
  LevelThreeScene::PatrolMobBody
  LevelThreeScene::PipeMobBody
```

`IPhysicsBody` 由數個較小的介面組成：

- `IPhysicsTransform`
- `IPhysicsMaterial`
- `IPhysicsMotion`
- `IPhysicsCollisionListener`
- `IPhysicsPushReactive`
- `IPhysicsLifecycle`

`PhysicsBodyTraits` 提供每個物理物件的 `BodyType` 與行為旗標。目前 `BodyType` 包含：

- `CHARACTER`
- `PUSHABLE_BOX`
- `PATROL_ENEMY`
- `MOVING_PLATFORM`
- `CONDITIONAL_PLATFORM`
- `ROPE_ENDPOINT`
- `BULLET`
- `JAR`
- `STATIC_BOUNDARY`

物理相關檔案集中在 `include/systems/` 與 `src/systems/`。

## 系統服務

```text
IAudioService        -> AudioService
IVisualThemeService  -> VisualThemeService
ISessionState        -> SessionState
IGlobalActors        -> GlobalActors
```

場景透過 `SceneServices` 取得這些服務。這樣場景只需要知道抽象介面，不需要直接建立或擁有長生命週期系統。

## 執行期擁有關係

- `SceneManager` 擁有所有場景物件與場景堆疊狀態。
- 每個使用物理的場景擁有一個 `PhysicsWorld`。
- `PhysicsWorld` 擁有透過 `AddStaticBoundary()` 建立的 `StaticBody`。
- 動態物理物件由場景或遊戲物件擁有，再註冊到該場景的 `PhysicsWorld`。
- 背景、地板、標題列、門與起始貓等共用物件由 `GlobalActors` 擁有。
- `SaveManager` 是靜態工具，集中處理 JSON 持久化。
