# 元件互動與資料流文件

> 本文件說明各元件在**執行期**如何傳遞資料、呼叫彼此、協調行為   
> 類別的靜態 OOP 結構(繼承、組合)請見 `CLASS_RELATIONSHIPS.md`   
> 場景切換流程圖已在 `ARCHITECTURE.md § 11` 說明   
> 物理系統各物件的 `PhysicsUpdate` 骨架已在 `PHYSICS_DESIGN.md` 說明 

---

## 目錄

1. [主迴圈資料流](#1-主迴圈資料流)
2. [Scene 生命週期協議](#2-scene-生命週期協議)
3. [輸入 → 物理 → 渲染 管線](#3-輸入--物理--渲染-管線)
4. [CharacterPhysicsSystem 內部呼叫順序](#4-characterphysicssystem-內部呼叫順序)
5. [GameContext 作為跨場景匯流排](#5-gamecontext-作為跨場景匯流排)
6. [選單系統的資料流(設定確認模式)](#6-選單系統的資料流設定確認模式)
7. [角色動畫狀態機](#7-角色動畫狀態機)
8. [BGMPlayer 與音量控制](#8-bgmplayer-與音量控制)
9. [持久化系統資料流(SaveManager)](#9-持久化系統資料流savemanager)
10. [LevelSelectScene 的互動流程](#10-levelselectscene-的互動流程)

---

## 1. 主迴圈資料流

```
Core::Context(PTSD 引擎)
    │  每幀
    ▼
App::Update()                          AppUpdate.cpp
    │
    ├─→ m_Ctx->BGMPlayer->Update()     自動偵測歌曲結束並切下一首
    │
    ├─→ m_CurrentScene->Update()       回傳 Scene* next
    │       │
    │       └─→ 若 next ≠ nullptr 且 ≠ current
    │               └─→ App::TransitionTo(next)
    │                       ├─ current->OnExit()
    │                       └─ next->OnEnter()
    │
    ├─→ 偵測 m_Ctx->ShouldQuit         若為 true → State = END
    │
    └─→ m_Ctx->Root.Update()           Util::Renderer，觸發所有已加入物件的渲染
```

**關鍵點**：
- `Scene::Update()` 的**回傳值**是唯一合法的場景切換觸發點，場景不能自己呼叫 `TransitionTo` 
- `BGMPlayer::Update()` 必須在 `Root.Update()` 之前呼叫，確保音樂切換不因渲染延遲而跳幀 
- `Root.Update()` 在場景邏輯**之後**執行，確保本幀的位置/狀態更新完畢才渲染 

---

## 2. Scene 生命週期協議

每個 Scene 子類別必須遵守以下配對規則：

```
OnEnter()                         OnExit()
─────────────────────────────    ─────────────────────────────
m_Ctx.Root.AddChild(objA)    ↔   m_Ctx.Root.RemoveChild(objA)
m_Ctx.Root.AddChild(objB)    ↔   m_Ctx.Root.RemoveChild(objB)
重置內部狀態(計數器、選項)      (不需要額外重置，下次 OnEnter 會再初始化)
```

違反配對會導致：
- **漏 AddChild**：物件不可見 
- **漏 RemoveChild**：同一 `shared_ptr` 在渲染清單重複出現，造成圖層疊加 Bug 
- **多次 AddChild**：同上，且物件更新邏輯被呼叫兩次 

**借用物件的特例**(`MenuScene` 的 UI 元件)：  
借用者在 `OnEnter` 加入、`OnExit` 移除，但在 `OnExit` 後需還原 position / scale，  
避免下次借用時繼承到修改後的值 

**永久物件的隱藏特例**(`LevelSelectScene` 與 `Header`)：  
`LevelSelectScene` 的佈局不需要 Header，因此在 `OnEnter` 呼叫 `m_Ctx.Header->SetVisible(false)`，  
`OnExit` 呼叫 `m_Ctx.Header->SetVisible(true)` 還原 不呼叫 `RemoveChild`，只切換可見性 

---

## 3. 輸入 → 物理 → 渲染 管線

以 `LocalPlayGameScene::Update()` 為例，完整的一幀處理流程：

```
Step 1  讀取輸入(LocalPlayGameScene::Update)
        ─────────────────────────────────────
        Util::Input::IsKeyPressed(key.left/right)  → agent.state.moveDir
        Util::Input::IsKeyDown(key.jump)
            + state.grounded → CharacterPhysicsSystem::ApplyJump(state)
                → state.velocityY = kJumpForce, grounded = false

Step 2  門碰撞偵測(上方按鍵觸發)
        ─────────────────────────────────────
        IsKeyDown(key.up) && 角色在門的 AABB 範圍內
            → pb.entered = true, actor->SetVisible(false)
            → ++m_EnteredCount，更新 DoorCountText
            → 若全員進門 → return m_LevelSelectScene

Step 3  攤平成 vector<PhysicsAgent>(複製快照，不是引用)
        ─────────────────────────────────────
        for pb in m_Players → agents.push_back(pb.agent)

Step 4  委託 CharacterPhysicsSystem::Update(agents, floor)
        ─────────────────────────────────────
        (直接呼叫靜態方法處理物理運算)

Step 5  把更新後的 state 寫回 m_Players
        ─────────────────────────────────────
        m_Players[i].agent.state = agents[i].state

Step 6  UpdateCooperativePower()
        ─────────────────────────────────────
        計算同向緊貼的最大玩家群組數 → m_Ctx.CooperativePushPower

Step 7  Root.Update()(由 App 呼叫，非 Scene)
        ─────────────────────────────────────
        所有已加入 Root 的 PlayerCat 依 ZIndex 排序後渲染到螢幕
```

**為什麼複製而非引用(Step 3)？**  
`CharacterPhysicsSystem` 需要在同一幀中同時讀取「所有角色的舊位置」並寫入新位置   
若直接傳引用，當 i=1 的角色移動後，i=2 讀到的 i=1 位置已是「新位置」，導致碰撞判斷不一致   
複製一份快照可保證全幀的碰撞判斷基準相同 

---

## 4. CharacterPhysicsSystem 內部呼叫順序

`CharacterPhysicsSystem::Update()` 對每個 agent 依序執行：

```
For each agent i:
│
├─ Step 1  記錄 prevGrounded
│
├─ Step 2  攜帶(Carry)
│          若 grounded 且 supportIndex >= 0
│          → 將 agents[sup].state.lastDeltaX 加到自身 X
│
├─ Step 3  頭頂阻擋
│          若 beingStoodOn 且 velocityY > 0 → velocityY = 0
│
├─ Step 4  水平移動 + 碰撞解析
│          ResolveHorizontal(i, rawX, agents)
│
├─ Step 5  面向翻轉
│
├─ Step 6  垂直物理
│          ResolveVertical(i, agents, floor)
│
├─ Step 7  更新 lastDeltaX
│
└─ Step 8  動畫狀態切換
           UpdateAnimState(i, agents, isPushing)

After loop:
└─ Step 9  計算 beingStoodOn
```

---

## 5. GameContext 作為跨場景匯流排

`GameContext` 是所有場景共享資料的唯一合法管道，**禁止場景之間直接讀取對方的成員** 

```
LocalPlayScene                     LocalPlayGameScene
      │  寫入                              │  讀取
      ▼                                    ▼
GameContext::SelectedPlayerCount ──────────→ OnEnter: SpawnPlayers(playerCount)
                                             LevelSelectScene: UpdateBestTimeText()

LocalPlayGameScene                 (未來的關卡場景)
      │  寫入                              │  讀取
      ▼                                    ▼
GameContext::CooperativePushPower ─────────→ 關卡設計(待擴充)

OptionMenuScene                    BGMPlayer
      │  呼叫                              │
      ▼                                    ▼
m_Ctx.BGMPlayer->SetVolume(v * 6)  ← OK 或 CANCEL 確認時套用
m_Ctx.Background->SetImage(path)   ← 背景圖同步切換
```

---

## 6. 選單系統的資料流(設定確認模式)

`KeyboardConfigScene` 和 `OptionMenuScene` 都採用**Pending / Applied 兩層資料**的確認模式，並在確認時持久化：

```
使用者操作
    │
    ▼
m_Pending(暫存)     ← 每次按鍵 / 滑鼠都直接修改這裡
    │
    ├─ 按下 OK / ENTER  → CommitPending() / m_Applied = m_Pending → m_Applied(生效值)
    │                                                                     │
    │                                         ┌─────────────────────────┘
    │                                         ▼
    │                              SaveManager::SaveOptionSettings(...)
    │                              SaveManager::SaveKeyConfigs(...)
    │                                  └── 寫入 GA_RESOURCE_DIR/Save/settings.json
    │
    └─ 按下 CANCEL / ESC → 捨棄 m_Pending，還原為 m_Applied 的值(不寫檔)
```

**設定載入時機**(程式啟動 → constructor)：

```
OptionMenuScene 建構子
    └── SaveManager::LoadOptionSettings(saved)
            └── 若成功 → m_Applied.bgColorIndex / bgmVolume / ... = saved.*

KeyboardConfigScene 建構子
    └── SaveManager::LoadKeyConfigs(loaded)
            └── 對每個 0~7P，若 loaded[i] 非全 UNKNOWN → m_Applied[i] = toKeyConfig(loaded[i])
```

**`LocalPlayScene` 阻擋進入遊戲的判斷：**

```
LocalPlayScene::Update()
    └── ENTER 鍵按下
            └── configuredCount = m_KbConfigScene->GetConfiguredPlayerCount()
                    → 計算 m_Applied[p].AllKeys().size() >= 4 的玩家數
                    → 若 m_PlayerCount <= configuredCount → return m_GameScene
                    → 否則 → 僅 LOG，不切換(UI 顯示紅字警告)
```

---

## 7. 角色動畫狀態機

`PlayerCat` 的動畫狀態由 `CharacterPhysicsSystem::UpdateAnimState` 每幀驅動   
狀態切換規則(優先順序由高到低)：

```
優先級  條件                              → 目標狀態
──────  ─────────────────────────────   ──────────
  1     !grounded && velocityY > 0      → JUMP_RISE
  2     !grounded && velocityY <= 0     → JUMP_FALL
  3     grounded && !prevGrounded       → LAND(justLanded)
  4     cur == LAND && !IfAnimationEnds → LAND(播放完才結束)
  5     isPushing                       → PUSH
  6     moveDir != 0                    → RUN
  7     (其他)                        → STAND
```

**轉換圖：**

```
          JUMP 鍵(grounded)
STAND ─────────────────────────────────────→ JUMP_RISE
  ↑   ←─── LAND(播完)                          │ velocityY 轉負
  │                                          JUMP_FALL
  │   ←─────────────── 落地(LAND 播完)────────┘
  │
  ├── moveDir ≠ 0 → RUN ──→ STAND(moveDir = 0)
  │
  └── isPushing → PUSH ──→ STAND / RUN(不再推動)
```

---

## 8. BGMPlayer 與音量控制

`BGMPlayer` 由 `GameContext` 持有，採用**靜態回呼 + flag** 的方式處理 SDL_mixer 的跨執行緒通知：

```
SDL_mixer 音樂結束(音訊執行緒)
    └── BGMPlayer::MusicFinishedCallback()
            └── s_ShouldPlayNext = true

主執行緒(每幀)
    └── App::Update() → m_Ctx->BGMPlayer->Update()
            └── 若 s_ShouldPlayNext == true
                    → s_ShouldPlayNext = false
                    → Next()  → m_CurrentIndex++ % size → Play()
```

**音量設定時機**：
- `OptionMenuScene::AdjustLeft/Right`(row 2 BGM VOLUME)：**即時預覽**，直接呼叫 `m_Ctx.BGMPlayer->SetVolume(m_Pending.bgmVolume * 6)` 
- 按 OK(鍵盤 / 滑鼠)：`m_Applied = m_Pending`，並再次呼叫 `SetVolume` 確認生效，然後寫入 `settings.json` 
- 按 CANCEL / ESC：呼叫 `SetVolume(m_Applied.bgmVolume * 6)` 還原為上次確認的音量，**不寫檔** 

---

## 9. 持久化系統資料流(SaveManager)

### 9.1 settings.json 讀寫時序

```
程式啟動
    │
    ├─ OptionMenuScene constructor
    │       └── SaveManager::LoadOptionSettings()
    │               ← 讀 settings.json 的 bgColorIndex / bgmVolume / seVolume / dispNumber
    │               → 初始化 m_Applied
    │
    └─ KeyboardConfigScene constructor
            └── SaveManager::LoadKeyConfigs()
                    ← 讀 settings.json 的 keyConfigs[0..7]
                    → 初始化 m_Applied[0..7](覆蓋預設值)

遊戲中 OptionMenuScene OK 確認
    └── SaveManager::SaveOptionSettings(toSave)
            → read-modify-write settings.json(只改 Option 欄位，保留 keyConfigs)

遊戲中 KeyboardConfigScene CommitPending
    └── SaveManager::SaveKeyConfigs(saveData)
            → read-modify-write settings.json(只改 keyConfigs 欄位，保留 Option 設定)
```

### 9.2 save_data.json 讀寫時序

```
進入 LevelSelectScene::OnEnter()
    └── SaveManager::LoadLevelData(m_LevelData)
            ← 讀 save_data.json 的全部 10 關資料
            → 更新皇冠標記、最佳時間文字

關卡完成(由各 LevelNScene 呼叫)
    └── SaveManager::UpdateBestTime(levelIdx, playerCount, elapsed)
            ├── SaveManager::LoadLevelData(levels)   ← 讀取最新存檔
            ├── 若 elapsed < levels[idx].bestTimes[p](或尚無紀錄)
            │       → levels[idx].bestTimes[p] = elapsed
            │       → levels[idx].completed = true
            │       → SaveManager::SaveLevelData(levels)  ← 寫回
            └── 否則：不做任何操作
```

### 9.3 read-modify-write 的必要性

`settings.json` 同時存放 Option 設定和 8P 按鍵綁定   
若 `OptionMenuScene` 直接覆寫整個檔案，會把 `KeyboardConfigScene` 剛寫入的按鍵綁定清除，反之亦然   
透過 read-modify-write，每個場景只修改自己負責的 JSON 欄位：

```
OptionMenuScene::SaveOptionSettings()      KeyboardConfigScene::SaveKeyConfigs()
       │                                              │
       ▼                                              ▼
讀 settings.json → 修改 bgColorIndex 等欄位    讀 settings.json → 修改 keyConfigs 欄位
       │                                              │
       └─────────── 寫回 settings.json ──────────────┘
                   (彼此不干擾)
```

---

## 10. LevelSelectScene 的互動流程

### 10.1 進入流程

```
LocalPlayGameScene::Update()
    └── 全員進門(m_EnteredCount >= m_Ctx.SelectedPlayerCount)
            → return m_LevelSelectScene
                    ↓
        App::TransitionTo(LevelSelectScene)
                    ↓
        LevelSelectScene::OnEnter()
            ├── SaveManager::LoadLevelData(m_LevelData)
            ├── AddChild 全部 UI 物件
            ├── m_Ctx.Header->SetVisible(false)
            └── UpdateSelectorPos() / UpdateTitleText() / UpdateBestTimeText() / UpdateCrowns()
```

### 10.2 每幀互動流程

```
LevelSelectScene::Update()
    │
    ├─ ESC → return m_LocalPlayGameScene
    │
    ├─ WASD/方向鍵 → 移動 m_SelectedIdx
    │       └── UpdateSelectorPos() + UpdateTitleText() + UpdateBestTimeText()
    │
    ├─ 滑鼠懸停 → AppUtil::IsMouseHovering(*m_LevelTexts[i])
    │       └── 自動切換 m_SelectedIdx(同上)
    │
    └─ ENTER 或左鍵點擊格子 → return m_LevelScenes[m_SelectedIdx]
            ├── 若 != nullptr → 進入該關卡場景
            └── 若 == nullptr → 留在 LevelSelectScene(關卡尚未實作)
```

### 10.3 最佳時間顯示邏輯

```
UpdateBestTimeText()
    ├── p = m_Ctx.SelectedPlayerCount          (LocalPlayScene 選定的人數)
    ├── best = m_LevelData[m_SelectedIdx].bestTimes[p]
    └── m_BestTimeText->SetText(
            std::to_string(p) + " PLAYERS BEST TIME: " + SaveManager::FormatTime(best))
            │
            └── FormatTime：-1.0f → "--:--.--"
                           ≥0.0f → "mm:ss.cs"
```

### 10.4 皇冠(完成標記)顯示邏輯

```
UpdateCrowns()
    └── for i in 0..9:
            m_CrownTexts[i]->SetVisible(m_LevelData[i].completed)
```

`completed` 在關卡完成時由 `SaveManager::UpdateBestTime()` 自動設為 `true` 並持久化，  
下次進入 `LevelSelectScene` 時讀取即可正確顯示 
