# 元件互動與資料流文件

> 本文件說明各元件在**執行期**如何傳遞資料、呼叫彼此、協調行為。  
> 類別的靜態 OOP 結構（繼承、組合）請見 `CLASS_RELATIONSHIPS.md`。  
> 場景切換流程圖已在 `ARCHITECTURE.md § 10` 說明。  
> 物理系統各物件的 `PhysicsUpdate` 骨架已在 `PHYSICS_DESIGN.md` 說明。

---

## 目錄

1. [主迴圈資料流](#1-主迴圈資料流)
2. [Scene 生命週期協議](#2-scene-生命週期協議)
3. [輸入 → 物理 → 渲染 管線](#3-輸入--物理--渲染-管線)
4. [CharacterPhysicsSystem 內部呼叫順序](#4-characterphysicssystem-內部呼叫順序)
5. [GameContext 作為跨場景匯流排](#5-gamecontext-作為跨場景匯流排)
6. [選單系統的資料流（設定確認模式）](#6-選單系統的資料流設定確認模式)
7. [角色動畫狀態機](#7-角色動畫狀態機)

---

## 1. 主迴圈資料流

```
Core::Context（PTSD 引擎）
    │  每幀
    ▼
App::Update()                          AppUpdate.cpp
    │
    ├─→ m_CurrentScene->Update()       回傳 Scene* next
    │       │
    │       └─→ 若 next ≠ nullptr 且 ≠ current
    │               └─→ App::TransitionTo(next)
    │                       ├─ current->OnExit()
    │                       └─ next->OnEnter()
    │
    └─→ m_Ctx->Root.Update()           Util::Renderer，觸發所有已加入物件的渲染
```

**關鍵點**：
- `Scene::Update()` 的**回傳值**是唯一合法的場景切換觸發點，場景不能自己呼叫 `TransitionTo`。
- `Root.Update()` 在場景邏輯**之後**執行，確保本幀的位置/狀態更新完畢才渲染。

---

## 2. Scene 生命週期協議

每個 Scene 子類別必須遵守以下配對規則：

```
OnEnter()                         OnExit()
─────────────────────────────    ─────────────────────────────
m_Ctx.Root.AddChild(objA)    ↔   m_Ctx.Root.RemoveChild(objA)
m_Ctx.Root.AddChild(objB)    ↔   m_Ctx.Root.RemoveChild(objB)
重置內部狀態（計數器、選項）      （不需要額外重置，下次 OnEnter 會再初始化）
```

違反配對會導致：
- **漏 AddChild**：物件不可見。
- **漏 RemoveChild**：同一 `shared_ptr` 在渲染清單重複出現，造成圖層疊加 Bug。
- **多次 AddChild**：同上，且物件更新邏輯被呼叫兩次。

**借用物件的特例**（`MenuScene` 的 UI 元件）：  
借用者在 `OnEnter` 加入、`OnExit` 移除，但在 `OnExit` 後需還原 position / scale，  
避免下次借用時繼承到修改後的值。

---

## 3. 輸入 → 物理 → 渲染 管線

以 `LocalPlayGameScene::Update()` 為例，完整的一幀處理流程：

```
Step 1  讀取輸入（LocalPlayGameScene::Update）
        ─────────────────────────────────────
        Util::Input::IsKeyPressed(key.left/right)  → agent.state.moveDir
        Util::Input::IsKeyDown(key.jump)
            + state.grounded && !state.beingStoodOn → 設定 state.velocityY, grounded=false

Step 2  攤平成 vector<PhysicsAgent>（複製，不是引用）
        ─────────────────────────────────────
        for pb in m_Players → agents.push_back(pb.agent)

Step 3  委託 CharacterPhysicsSystem::Update(agents, floor)
        ─────────────────────────────────────
        （詳見第 4 節）

Step 4  把更新後的 state 寫回 m_Players
        ─────────────────────────────────────
        m_Players[i].agent.state = agents[i].state
        （actor 是 shared_ptr，System 直接修改 actor->SetPosition()，不需要寫回）

Step 5  Root.Update()（由 App 呼叫，非 Scene）
        ─────────────────────────────────────
        所有已加入 Root 的 PlayerCat 依 ZIndex 排序後渲染到螢幕
```

**為什麼複製而非引用？**  
`CharacterPhysicsSystem` 需要在同一幀中同時讀取「所有角色的舊位置」並寫入新位置。  
若直接傳引用，當 i=1 的角色移動後，i=2 讀到的 i=1 位置已是「新位置」，導致碰撞判斷不一致。  
複製一份快照可保證全幀的碰撞判斷基準相同。

---

## 4. CharacterPhysicsSystem 內部呼叫順序

`CharacterPhysicsSystem::Update()` 對每個 agent 依序執行：

```
For each agent i:
│
├─ Step 1  攜帶（Carry）
│          若 grounded 且 supportIndex >= 0
│          → 將 agents[sup].state.lastDeltaX 加到自身 X
│          → 實現「站在移動角色頭上被帶著走」
│
├─ Step 2  水平移動 + 碰撞解析
│          ResolveHorizontal(i, rawX, agents)
│          → 跳過支撐關係（supportIndex）的角色
│          → 對同水平面（vertTolerance ≈ 32.2f）的其他角色做 AABB 擠壓
│          → 偵測 isPushing（想移動但被阻擋）
│
├─ Step 3  垂直物理
│          ResolveVertical(i, agents, floor)
│          → 施加重力（velocityY -= kGravity）
│          → 找最高的有效落點（地板 or 其他角色頭頂）
│          → 更新 grounded、supportIndex
│
├─ Step 4  更新 lastDeltaX
│          記錄本幀實際水平位移（供下幀攜帶計算）
│
├─ Step 5  偵測 beingPushed
│          IsBeingPushed(i, agents, floor)
│          → 旁邊是否有人正在推自己（不再用於動畫，保留供未來擴充）
│
└─ Step 6  動畫狀態切換
           UpdateAnimState(i, agents, isPushing, beingPushed)
           → 只有 isPushing 才切換成 PUSH（beingPushed 不影響動畫）

After loop:
└─ Step 7  計算 beingStoodOn
           掃描所有 supportIndex → 標記被踩者的 beingStoodOn = true
           → 供下一幀 Scene 的跳躍判斷使用
```

**Step 7 為何在迴圈外？**  
若放在 Step 3 之後、同一迴圈內，當 agent 0 尚未處理時，agent 1 的 supportIndex 還是舊值，  
導致 beingStoodOn 判斷遺漏。放在全部 agent 的垂直物理完成後，所有 supportIndex 都是本幀最新值。

---

## 5. GameContext 作為跨場景匯流排

`GameContext` 是所有場景共享資料的唯一合法管道，**禁止場景之間直接讀取對方的成員**。

```
LocalPlayScene                     LocalPlayGameScene
      │                                    │
      │  寫入                              │  讀取
      ▼                                    ▼
GameContext::SelectedPlayerCount ──────────→ SpawnPlayers(playerCount)

LocalPlayGameScene                 （下一個關卡場景）
      │  寫入                              │  讀取
      ▼                                    ▼
GameContext::CooperativePushPower ─────────→ 關卡設計用途（待擴充）
```

`GameContext` 中的**永久物件**（`Floor`、`Header`、`StartupCats`）由 `AppStart` 建立並加入渲染樹，  
場景只呼叫 `SetVisible()` 或 `SetInputEnabled()` 控制顯示，不移除也不重新建立。

此部分的結構說明已在 `ARCHITECTURE.md § 3` 說明。

---

## 6. 選單系統的資料流（設定確認模式）

`KeyboardConfigScene` 和 `OptionMenuScene` 都採用**Pending / Applied 兩層資料**的確認模式：

```
使用者操作
    │
    ▼
m_Pending（暫存）     ← 每次按鍵 / 滑鼠都直接修改這裡
    │
    ├─ 按下 OK / ENTER  → CommitPending() → m_Applied（生效值）
    │                                            │
    │                                            └─→ 外部讀取（GetAppliedConfig）
    │
    └─ 按下 CANCEL / ESC → 捨棄 m_Pending，還原為 m_Applied 的值
```

**`LocalPlayGameScene` 讀取按鍵設定的時機：**

```
LocalPlayGameScene::OnEnter()
    └── SpawnPlayers(count)
            └── For each i:
                    pb.key = m_KbConfigScene->GetAppliedConfig(i)
                    //                         ↑ 只讀取 m_Applied，不受 m_Pending 影響
```

這確保即使玩家在 `KeyboardConfigScene` 中途修改了 `m_Pending` 但未確認，  
進入遊戲後讀到的仍是上次確認的設定。

---

## 7. 角色動畫狀態機

`PlayerCat` 的動畫狀態由 `CharacterPhysicsSystem::UpdateAnimState` 每幀驅動。  
狀態切換規則（優先順序由高到低）：

```
優先級  條件                              → 目標狀態
──────  ─────────────────────────────   ──────────
  1     !grounded && velocityY > 0      → JUMP_RISE
  2     !grounded && velocityY <= 0     → JUMP_FALL
  3     grounded && !prevGrounded       → LAND（justLanded）
  4     cur == LAND && !IfAnimationEnds → LAND（播放完才結束）
  5     isPushing                       → PUSH
  6     moveDir != 0                    → RUN
  7     （其他）                        → STAND
```

**轉換圖：**

```
          JUMP 鍵（grounded && !beingStoodOn）
STAND ─────────────────────────────────────→ JUMP_RISE
  ↑   ←─── LAND（播完）                          │ velocityY 轉負
  │                                          JUMP_FALL
  │   ←─────────────── 落地（LAND 播完）────────┘
  │
  ├── moveDir ≠ 0 → RUN ──→ STAND（moveDir = 0）
  │
  └── isPushing → PUSH ──→ STAND / RUN（不再推動）
```

**關鍵約束**：
- `LAND` 狀態在動畫播完前（`IfAnimationEnds() == false`）不可被 PUSH / RUN / STAND 打斷（優先級 4）。
- `beingPushed`（被推方）**不會**觸發 PUSH 動畫；只有主動推動的一方（`isPushing`）才切換（優先級 5）。

`PlayerCat::SetCatAnimState()` 負責在狀態真正改變時才切換 `Drawable`，  
避免每幀重複設定相同動畫導致動畫重置。此細節在 `PlayerCat.hpp` 內部處理，Scene 與 System 不需感知。
