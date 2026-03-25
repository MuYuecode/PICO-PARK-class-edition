# Component Interaction & Data Flow

> Static OOP structure → `CLASS_RELATIONSHIPS.md`.
> Scene transition diagram → `ARCHITECTURE.md §11`.
> Physics object skeletons → `PHYSICS_DESIGN.md`.
> This document covers **runtime** data flow, call sequences, and coordination.

## Contents
1. [Main Loop Data Flow](#1-main-loop-data-flow)
2. [Scene Lifecycle Protocol](#2-scene-lifecycle-protocol)
3. [Input → Physics → Render Pipeline](#3-input--physics--render-pipeline)
4. [CharacterPhysicsSystem Call Order](#4-characterphysicssystem-call-order)
5. [GameContext as Cross-scene Bus](#5-gamecontext-as-cross-scene-bus)
6. [Settings Confirmation Pattern](#6-settings-confirmation-pattern)
7. [Character Animation State Machine](#7-character-animation-state-machine)
8. [BGMPlayer & Volume Control](#8-bgmplayer--volume-control)
9. [SaveManager Data Flow](#9-savemanager-data-flow)
10. [LevelSelectScene Interaction Flow](#10-levelselectscene-interaction-flow)

---

## 1. Main Loop Data Flow

```
Core::Context (PTSD engine)
    │  each frame
    ▼
App::Update()                              AppUpdate.cpp
    ├─→ m_Ctx->BGMPlayer->Update()         detect track end, advance to next
    ├─→ m_CurrentScene->Update()           returns Scene* next
    │       └─→ if next ≠ nullptr && ≠ current
    │               └─→ App::TransitionTo(next)
    │                       ├─ current->OnExit()
    │                       └─- next->OnEnter()
    ├─→ check m_Ctx->ShouldQuit            if true → State = END
    └─→ m_Ctx->Root.Update()              Util::Renderer: render all children
```

**Key points:**
- `Scene::Update()` return value is the only legal scene-switch trigger.
- `BGMPlayer::Update()` runs before `Root.Update()` to avoid skipped-frame music transitions.
- `Root.Update()` runs after scene logic so this frame's position/state changes are reflected immediately.

---

## 2. Scene Lifecycle Protocol

```
OnEnter()                          OnExit()
──────────────────────────────     ──────────────────────────────
m_Ctx.Root.AddChild(objA)     ↔    m_Ctx.Root.RemoveChild(objA)
reset internal state               (re-initialization deferred to next OnEnter)
```

Violations:
- **Missing AddChild** → object invisible.
- **Missing RemoveChild** → duplicate render-list entry → layer overlap bug.
- **Double AddChild** → same as above, plus update logic called twice.

**Borrowed-object exception (MenuScene UI):** borrower adds in `OnEnter`, removes in `OnExit`, and restores position/scale before exit.

**Permanent-object hide exception (LevelSelectScene + Header):** `LevelSelectScene::OnEnter` calls `m_Ctx.Header->SetVisible(false)`; `OnExit` calls `SetVisible(true)`. No `RemoveChild`.

---

## 3. Input → Physics → Render Pipeline

Full per-frame flow in `LocalPlayGameScene::Update()`:

```
Step 1  Read input
        Util::Input::IsKeyPressed(key.left/right)  → agent.state.moveDir
        Util::Input::IsKeyDown(key.jump) + state.grounded
            → CharacterPhysicsSystem::ApplyJump(state)
               sets velocityY = kJumpForce, grounded = false

Step 2  Door collision check (UP key)
        IsKeyDown(key.up) && actor within door AABB
            → pb.entered = true, actor->SetVisible(false)
            → ++m_EnteredCount, UpdateDoorCountText()
            → if all entered → return m_LevelSelectScene

Step 3  Snapshot agents into vector<PhysicsAgent> (copy, not reference)

Step 4  CharacterPhysicsSystem::Update(agents, floor)

Step 5  Write updated state back to m_Players[i].agent.state

Step 6  UpdateCooperativePower()
        → compute largest same-direction touching group
        → write to m_Ctx.CooperativePushPower

Step 7  Root.Update() (called by App, not Scene)
```

**Why copy in Step 3?**  
`CharacterPhysicsSystem` needs all characters' *old* positions as the consistent baseline for this frame's collision resolution. Without a snapshot, when character i=1 moves, character i=2 would see the *new* position of i=1, causing asymmetric collision results.

---

## 4. CharacterPhysicsSystem Call Order

Per agent `i` in `Update()`:

```
1. record prevGrounded
2. Carry: if grounded && supportIndex >= 0
          → add agents[sup].lastDeltaX to own X (rider inherits platform motion)
3. Head-block: if beingStoodOn && velocityY > 0 → velocityY = 0
4. Horizontal move + collision: ResolveHorizontal(i, rawX, agents)
5. Face direction flip (scale.x sign)
6. Vertical physics: ResolveVertical(i, agents, floor)
7. Update lastDeltaX
8. Animation state: UpdateAnimState(i, agents, isPushing)

After loop:
9. Recompute beingStoodOn for all agents
```

---

## 5. GameContext as Cross-scene Bus

`GameContext` is the only legal channel for sharing state between scenes.

```
LocalPlayScene          writes SelectedPlayerCount
LocalPlayGameScene      reads  SelectedPlayerCount → SpawnPlayers(count)
LevelSelectScene        reads  SelectedPlayerCount → UpdateBestTimeText()

LocalPlayGameScene      writes CooperativePushPower
(future level scenes)   reads  CooperativePushPower

OptionMenuScene         calls  m_Ctx.BGMPlayer->SetVolume(v * 6)   on adjust/OK/CANCEL
                               m_Ctx.Background->SetImage(path)    on bg color change
```

---

## 6. Settings Confirmation Pattern

Both `KeyboardConfigScene` and `OptionMenuScene` use a **Pending / Applied** pattern:

```
User interaction
    ↓
m_Pending (buffer)     ← all UI edits go here immediately
    │
    ├─ OK / ENTER  → CommitPending() or m_Applied = m_Pending
    │                    → SaveManager::SaveOptionSettings / SaveKeyConfigs
    │                        → write to GA_RESOURCE_DIR/Save/settings.json
    │
    └─ CANCEL / ESC → discard m_Pending, revert to m_Applied (no file write)
```

**Load timing (constructor):**
```
OptionMenuScene()       → SaveManager::LoadOptionSettings()  → init m_Applied
KeyboardConfigScene()   → SaveManager::LoadKeyConfigs()      → init m_Applied[0..7]
```

**LocalPlayScene entry gate:**
```
ENTER pressed
    → configuredCount = m_KbConfigScene->GetConfiguredPlayerCount()
         (players with AllKeys().size() >= 4)
    → if m_PlayerCount <= configuredCount → return m_GameScene
    → else → LOG only, show red warning text, do not transition
```

---

## 7. Character Animation State Machine

`CharacterPhysicsSystem::UpdateAnimState` drives `PlayerCat` each frame. Priority order (high → low):

```
Condition                              → Target State
───────────────────────────────────    ─────────────
!grounded && velocityY > 0             JUMP_RISE
!grounded && velocityY <= 0            JUMP_FALL
grounded && !prevGrounded              LAND  (justLanded)
cur == LAND && !IfAnimationEnds()      LAND  (hold until clip ends)
isPushing                              PUSH
moveDir != 0                           RUN
(otherwise)                            STAND
```

**Transition diagram:**

```
         JUMP key (grounded)
STAND ──────────────────────────────→ JUMP_RISE
  ↑  ←── LAND (clip ends)                │ velocityY turns negative
  │                                   JUMP_FALL
  │  ←──────────── land (LAND ends) ─────┘
  ├── moveDir ≠ 0 → RUN → STAND (moveDir = 0)
  └── isPushing → PUSH → STAND / RUN
```

**LAND guard in `PlayerCat::SetCatAnimState`:** while in LAND and the clip hasn't ended, any state request other than LAND is silently ignored — ensuring the landing animation always plays in full.

---

## 8. BGMPlayer & Volume Control

Uses a **static callback + flag** pattern to bridge SDL_mixer's audio thread to the main thread:

```
SDL_mixer music-end (audio thread)
    └── BGMPlayer::MusicFinishedCallback()
            └── s_ShouldPlayNext = true

Main thread (each frame)
    └── BGMPlayer::Update()
            └── if s_ShouldPlayNext → Next() → advance index → Play()
```

**Volume control timing:**
- `OptionMenuScene::AdjustLeft/Right` (row 2, BGM VOLUME) → **live preview**: calls `SetVolume(pending * 6)` immediately.
- OK confirmed → `SetVolume(applied * 6)` + write to `settings.json`.
- CANCEL / ESC → `SetVolume(applied * 6)` to revert preview; no file write.

---

## 9. SaveManager Data Flow

### 9.1 settings.json Read/Write Sequence

```
App startup
    ├─ OptionMenuScene()   → LoadOptionSettings()  → init m_Applied
    └─ KeyboardConfigScene() → LoadKeyConfigs()    → init m_Applied[0..7]

OptionMenuScene OK   → SaveOptionSettings(toSave)
    read-modify-write settings.json (only option fields; keyConfigs untouched)

KeyboardConfigScene CommitPending → SaveKeyConfigs(saveData)
    read-modify-write settings.json (only keyConfigs; option fields untouched)
```

### 9.2 save_data.json Read/Write Sequence

```
LevelSelectScene::OnEnter()
    → LoadLevelData(m_LevelData) → update crown visibility, best time text

Level scene completion
    → UpdateBestTime(levelIdx, playerCount, elapsed)
        ├─ LoadLevelData(levels)
        ├─ if elapsed < bestTimes[p] (or no record)
        │       → bestTimes[p] = elapsed, completed = true
        │       → SaveLevelData(levels)
        └─ else: no-op
```

### 9.3 Why Read-Modify-Write?

`settings.json` is shared by `OptionMenuScene` and `KeyboardConfigScene`. A direct overwrite by either would erase the other's data. Each scene reads the current file, modifies only its own fields, and writes the merged result back.

---

## 10. LevelSelectScene Interaction Flow

### 10.1 Entry

```
LocalPlayGameScene::Update()
    └─ all players entered door (m_EnteredCount >= SelectedPlayerCount)
           → return m_LevelSelectScene
               → App::TransitionTo(LevelSelectScene)
                   → LevelSelectScene::OnEnter()
                       ├─ SaveManager::LoadLevelData(m_LevelData)
                       ├─ AddChild all UI objects
                       ├─ m_Ctx.Header->SetVisible(false)
                       └─ UpdateSelectorPos / UpdateTitleText / UpdateBestTimeText / UpdateCrowns
```

### 10.2 Per-frame Update

```
LevelSelectScene::Update()
    ├─ ESC → return m_LocalPlayGameScene
    ├─ WASD / arrow keys → update m_SelectedIdx
    │       → UpdateSelectorPos + UpdateTitleText + UpdateBestTimeText
    ├─ Mouse hover → AppUtil::IsMouseHovering(*m_LevelCover[i])
    │       → auto-switch m_SelectedIdx (same updates as above)
    └─ ENTER or left-click on cover → confirmed
            ├─ if m_LevelScenes[i] != nullptr → return that scene  (not yet implemented)
            └─ else: stay in LevelSelectScene
```

### 10.3 Best Time Display

```
UpdateBestTimeText()
    ├─ p    = m_Ctx.SelectedPlayerCount
    ├─ best = m_LevelData[m_SelectedIdx].bestTimes[p]
    └─ m_BestTimeText->SetText(
           to_string(p) + " PLAYERS BEST TIME: " + SaveManager::FormatTime(best))
               FormatTime: -1.0f → "--:--.--"
                           ≥0.0f → "mm:ss.cs"
```

### 10.4 Crown (Completion) Display

```
UpdateCrowns()
    └─ for i in 0..LEVEL_COUNT-1:
           m_Crown[i]->SetVisible(m_LevelData[i].completed)
```

`completed` is set to `true` by `SaveManager::UpdateBestTime()` when a level is first cleared and persisted. The next `LevelSelectScene::OnEnter()` reloads and displays it correctly.
