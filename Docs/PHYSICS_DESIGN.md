# 物理系統設計

## 範圍

物理系統是場景本地的。每個需要物理的場景都擁有自己的 `PhysicsWorld`，離開場景時會清空本地物理狀態。

目前有效的物理物件實作：

- `PlayerCat`
- `PushableBox`
- `StaticBody`
- `BulletBody`
- `LevelTwoScene::MovingPlankBody`
- `LevelThreeScene::MovingLiftBody`
- `LevelThreeScene::PatrolMobBody`
- `LevelThreeScene::PipeMobBody`

`PushableBox` 是目前唯一的 `IPushable` 實作。

物理相關檔案放在 `include/systems/` 與 `src/systems/`。

## Body Type

`PhysicsBodyTraits::type` 目前使用：

- `CHARACTER`：玩家貓。
- `PUSHABLE_BOX`：合作推箱。
- `PATROL_ENEMY`：LevelThree 敵人。
- `MOVING_PLATFORM`：移動木板與升降平台。
- `CONDITIONAL_PLATFORM`：保留。
- `ROPE_ENDPOINT`：保留。
- `BULLET`：LevelFour 子彈。
- `JAR`：LevelFour 罐子靜態碰撞物。
- `STATIC_BOUNDARY`：地板、牆、天花板、發射器碰撞物與一般靜態幾何。

`PhysicsBodyTraits` 中的 `supportsPush` 與 `emitsCollisionCallbacks` 是行為旗標，但目前碰撞解析主要依照 `type`、`IsSolid()` 與 `IsKinematic()` 分支。

## `IPhysicsBody` 合約

`IPhysicsBody` 整合下列能力：

- Transform：`GetPosition`、`SetPosition`、`GetHalfSize`。
- Material：`IsSolid`、`IsKinematic`。
- Motion：`PhysicsUpdate`、`GetDesiredDelta`、`ApplyResolvedDelta`。
- Collision callback：`OnCollision(CollisionInfo)`。
- Push reaction：`GetMoveDir` 與選用的 `NotifyPush`。
- Lifecycle：`IsActive`、`SetActive`、`IsFrozen`、`Freeze`、`Unfreeze`、`PostUpdate`。

這些小介面目前都集中在 `systems` 分類下，方便物理系統一起閱讀。

## `PhysicsWorld` 職責

- 註冊與解除註冊動態物理物件。
- 擁有透過 `AddStaticBoundary(...)` 建立的靜態碰撞物。
- 在場景離開或重設時清空所有物件與 rope metadata。
- 每次場景更新只執行一次物理 frame pipeline。
- 暫停覆蓋層開啟時凍結所有已註冊物件，恢復時解除凍結。
- 提供 `PushableBox` 需要的推動查詢服務。

Rope metadata 目前有 `AddRope`、`RemoveRope` 與 `GetRopesOf`，但尚未實作 rope force solving。

## 每幀物理流程

1. `PhysicsWorld::Update()` 會視需要清掉已失效的 weak body reference。
2. `PhysicsUpdateScheduler::StepPhysicsUpdate(...)` 分兩輪呼叫 `PhysicsUpdate()`：
   - active、未凍結、非 `PUSHABLE_BOX`
   - active、未凍結的 `PUSHABLE_BOX`
3. `CollisionSolver::ResolveAndApply(...)`：
   - 建立 active body 快照
   - 偵測 support/riding 關係
   - 將 kinematic 或 frozen body 標記為已解析
   - 解析動態物件對實體物件的碰撞
   - 套用解析後位移
   - 發送 `OnCollision(...)` 回呼
4. `PhysicsWorld` 對 active 且未凍結的物件呼叫 `PostUpdate()`。

兩階段更新讓箱子可以查詢同一幀中角色的推動意圖。

## 碰撞規則

- 碰撞形狀是 AABB，採用 strict overlap (`<`)，邊緣剛好接觸不算重疊。
- 解析順序是水平軸，再垂直軸。
- 物件會對其他 active 且 solid 的物件解析碰撞。
- 非 solid 物件仍可在自己的移動被 solid 物件解析時收到碰撞回呼。
- Kinematic 物件代表參考幾何，會提早標記為已解析。
- Frozen 物件不產生移動，也會提早標記為已解析。
- 移動平台的垂直 snap 邏輯處理升降平台或平台端點接觸。

## 支撐規則

- `SupportResolver` 在移動解析前計算支撐關係。
- 支撐候選接近時，移動平台會優先。
- 被支撐的物件會繼承支撐物的 X 位移。
- 只有當支撐物是 `MOVING_PLATFORM` 時，才會套用 Y 位移攜帶。

## 合作推動邏輯

- `PushableBox::PhysicsUpdate()` 透過 `IPushQueryService` 查詢推動狀態。
- `PushForceResolver` 會遞迴計算角色與箱子鏈中的 active pusher 數量。
- 當 `abs(netPushers) >= requiredPushers` 時，箱子開始水平移動。
- 箱子永遠會套用重力。
- 鄰近且 active 的推動者會收到 `NotifyPush()`，用於推動動畫回饋。

## 子彈邏輯

`BulletBody` 是正式物理物件，不是場景內臨時 AABB helper。

- `PhysicsUpdate()` 依照速度與 delta time 讓子彈往左移動。
- `IsSolid()` 回傳 false，因此子彈不會阻擋或推動其他物件。
- `IsKinematic()` 回傳 false，因此碰撞解算器會解析子彈與 solid 物件的接觸並發送碰撞回呼。
- `OnCollision(...)` 記錄 `HitType`：
  - `Character`
  - `Jar`
  - `Solid`
  - `None`
- `LevelFourScene` 在 `m_World.Update()` 後讀取命中類型並處理遊戲反應：
  - 命中角色：停用子彈
  - 命中罐子：推進罐子狀態，然後停用子彈
  - 命中實體：停用子彈

場景仍然擁有高層遊戲狀態，例如罐子階段、計時器、鑰匙持有者、門狀態與過關切換。

## 場景整合模式

使用物理的場景遵循以下模式：

- `OnEnter`：清空世界、重設狀態、建立並註冊動態物件、建立靜態邊界。
- `Update`：設定輸入與移動意圖，呼叫一次 `m_World.Update()`，同步視覺物件並處理遊戲狀態。
- `OnExit`：必要時解除凍結，清空世界並移除場景本地視覺物件。

目前使用物理的場景：

- `TitleScene`
- `MenuScene`
- `LocalPlayGameScene`
- `LevelOneScene`
- `LevelTwoScene`
- `LevelThreeScene`
- `LevelFourScene`

`LevelExitScene` 的暫停與恢復由場景覆寫實作，內部呼叫 `PhysicsWorld::FreezeAll()` 與 `PhysicsWorld::UnfreezeAll()`。
