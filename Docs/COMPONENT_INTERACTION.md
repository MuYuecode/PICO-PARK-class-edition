# Component Interaction & Data Flow

> Static OOP structure → `CLASS_RELATIONSHIPS.md`.
> Scene transition diagram → `ARCHITECTURE.md §11`.
> Physics object interface and chain-push algorithm → `PHYSICS_DESIGN.md`.
> This document covers **runtime** data flow, call sequences, and coordination.

## Contents
1. [Main Loop Data Flow](#1-main-loop-data-flow)
2. [Scene Lifecycle Protocol](#2-scene-lifecycle-protocol)
3. [Input → Physics → Render Pipeline](#3-input--physics--render-pipeline)
4. [PhysicsWorld Internal Pipeline](#4-physicsworld-internal-pipeline)
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

The complete per-frame flow in a physics-enabled scene (shown for
`LocalPlayGameScene::Update()`):

```
Step 1  Read input → set move directions on each cat
        IsKeyPressed(key.left / key.right) → cat->SetMoveDir(dir)
        IsKeyDown(key.jump) && cat->IsGrounded() → cat->Jump()
            Jump() sets: m_VelocityY = kJumpForce, m_Grounded = false

Step 2  Door-entry check (positional logic; runs before physics)
        If cat within door AABB && IsKeyDown(key.up):
            pb.entered = true, cat->SetActive(false), ++m_EnteredCount
            if all entered → return m_LevelSelectScene

Step 3  Unified physics pipeline
        m_World.Update()
        → Phase 1 : all cats + boxes self-drive (PhysicsUpdate)
        → Resolve  : collision resolution (AABB separation, stacking carry)
        → Apply    : ApplyResolvedDelta → positions updated
        → Callbacks: OnCollision → grounded flags, velocityY clamps, m_IsPushing
        → PostUpdate: animation state machine

Step 4  Game statistics (pure read, no displacement)
        UpdateCooperativePower() → writes m_Ctx.CooperativePushPower
```

**Ordering rule:** `SetMoveDir()` and `Jump()` must be called **before**
`m_World.Update()` because `PhysicsWorld::StepPhysicsUpdate()` reads
`GetMoveDir()` and integrates velocity during `PhysicsUpdate()`. Any input
applied after `m_World.Update()` would silently take effect one frame late.

**Why no agent snapshot?** The old architecture used a per-frame `vector<PhysicsAgent>` copy to give `CharacterPhysicsSystem` a consistent baseline. The new architecture achieves the same guarantee via the two-pass ordering inside `StepPhysicsUpdate()` (characters commit desired deltas before boxes query them) and the topological resolution order in `StepResolveAndApply()` (supports resolved before riders). No external snapshot is required.

---

## 4. PhysicsWorld Internal Pipeline

`PhysicsWorld::Update()` internally orchestrates four sub-steps. This section
documents what happens inside each one.

### 4.1 Phase 1 — `StepPhysicsUpdate()`

```
Pass A (non-PUSHABLE_BOX bodies):
    for each active, non-frozen body except PUSHABLE_BOX:
        body->PhysicsUpdate()

    PlayerCat::PhysicsUpdate():
        m_Grounded = false          // will be restored by OnCollision
        m_IsPushing = false         // will be restored by NotifyAdjacentPushers
        m_VelocityY -= kGravity
        read own keys (if m_InputEnabled && key bindings are not UNKNOWN)
        m_DesiredDelta = {moveDir * kGroundMoveSpeed, m_VelocityY}
        flip m_Transform.scale.x to face movement direction

Pass B (PUSHABLE_BOX bodies):
    for each active, non-frozen PUSHABLE_BOX:
        box->PhysicsUpdate()

    PushableBox::PhysicsUpdate():
        m_VelocityY -= kGravity
        netRight = m_World->CountCharactersPushing(this, +1)
        netLeft  = m_World->CountCharactersPushing(this, -1)
        net = netRight - netLeft
        update m_CountText (deficit = max(0, required - |net|))
        vx = kPushSpeed * sign(net)  if |net| >= requiredPushers, else 0
        m_DesiredDelta = {vx, m_VelocityY}
        if moving: NotifyAdjacentPushers(activeDir)
            → calls ch->NotifyPush() on adjacent characters on the push side
```

**Why does Pass B come second?** `CountCharactersPushing()` reads `GetMoveDir()`
on CHARACTER bodies. If boxes ran first, they would see stale `moveDir` values
from the previous frame. The two-pass ordering guarantees all `moveDir` values
are current-frame before any box queries them.

### 4.2 Resolve — `StepResolveAndApply()`

```
1. Snapshot:  build vector<BodyInfo> from all active bodies
              each entry: body, desired delta, resolved=desired, resolvedFlag=false, supportIdx=-1

2. DetectRiding():
   for each non-kinematic body A:
     for each solid body B:
       if A's bottom edge is within 5 px of B's top edge
          AND horizontal overlap > 85% of combined half-widths:
           A.supportIdx = B's index  (A is riding B)

3. Immediate resolution for kinematic / frozen bodies:
   StaticBody: resolvedFlag=true, resolved={0,0}
   Frozen body: resolvedFlag=true, resolved={0,0}

4. Iterative topological loop (repeat until no progress):
   for each body i that is NOT yet resolved:
     if its supportIdx j is still unresolved → skip (wait for j first)
     effective_delta = i.desired + (j.resolved.x if j is non-kinematic)  (stacking carry)
     i.resolved = ResolveBody(i, effective_delta, infos)
     i.resolvedFlag = true

   ResolveBody():
     Horizontal pass: sweep body rightward/leftward by resolved.x;
       for each solid body (using resolved position if already resolved,
       pre-move position otherwise):
         if AABB overlap → push self out horizontally; record collidedH + normalH
     Vertical pass (with horizontal already applied):
       same logic in Y; record collidedV + normalV

5. Safety pass: resolve any remaining orphaned bodies without carry

6. Apply: body->ApplyResolvedDelta(resolved) for every body
   PlayerCat:  position += resolved
   PushableBox: position += resolved; reposition m_CountText label
   StaticBody: no-op
```

The topological ordering ensures that a character standing on a moving platform
(or on another character's head) inherits the support's horizontal displacement
before their own collision is resolved — producing correct platform-following
and stacking behaviour without extra special-case code.

### 4.3 Callbacks — `StepCollisionCallbacks()`

```
for each body that recorded a horizontal contact during resolution:
    body->OnCollision({other=collidedH, normal=normalH})

for each body that recorded a vertical contact:
    body->OnCollision({other=collidedV, normal=normalV})

PlayerCat::OnCollision:
    normal.y > +0.5  → m_Grounded = true, m_VelocityY = 0
    normal.y < -0.5  → ceiling hit: clamp m_VelocityY to 0 if rising
    |normal.x| > 0.5 && moveDir != 0 && blocked in move direction
                     → m_IsPushing = true

PushableBox::OnCollision:
    normal.y > +0.5  → m_VelocityY = 0  (landed on floor)
    normal.y < -0.5  → m_VelocityY = 0  (hit ceiling)
```

### 4.4 PostUpdate

```
for each active non-frozen body:
    body->PostUpdate()

PlayerCat::PostUpdate():
    → UpdateAnimState()
        priority (high to low):
          airborne rising        → JUMP_RISE
          airborne falling       → JUMP_FALL
          just landed            → LAND (hold until clip ends)
          m_IsPushing            → PUSH
          moveDir != 0           → RUN
          (default)              → STAND
```

### 4.5 Chain-Pushing Query — `CountCharactersPushing(target, dir)`

This is called by `PushableBox::PhysicsUpdate()` during Pass B of Phase 1.
The algorithm walks the adjacency graph recursively:

```
CountCharactersPushingImpl(target, dir, visited):
    add target to visited
    for each active body sp adjacent to target on the correct side:
        if sp is CHARACTER:
            if sp->GetMoveDir() == dir → ++count       (active pusher)
            always recurse into sp                      (passive chain transmission)
        if sp is PUSHABLE_BOX:
            recurse into sp                             (box transmits passively)
    return count
```

The unconditional recursion for `CHARACTER` bodies (regardless of their own
`moveDir`) is essential for scenario 3: a passive character standing between an
active pusher and the box still transmits the upstream force. See `PHYSICS_DESIGN.md §4`
for the full scenario verification table.

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

LocalPlayGameScene      reads  m_Ctx.Floor (for boundary positioning)
                               m_Ctx.Door  (swaps open/close image on enter/exit)
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

`PlayerCat::UpdateAnimState()` runs inside `PostUpdate()` every frame.
Priority order (high → low):

```
Condition                              → Target State
───────────────────────────────────    ─────────────
!grounded && velocityY > 0             JUMP_RISE
!grounded && velocityY <= 0            JUMP_FALL
grounded && !prevGrounded              LAND  (justLanded)
cur == LAND && !IfAnimationEnds()      LAND  (hold until clip ends)
m_IsPushing                            PUSH
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
  └── m_IsPushing → PUSH → STAND / RUN
```

`m_IsPushing` is reset to `false` at the top of every `PhysicsUpdate()` call
and is re-set either via `OnCollision` (when the cat is blocked while moving)
or via `PushableBox::NotifyAdjacentPushers()` (when the box is actually moving
and notifies its contributing pushers). This ensures the PUSH animation
activates only when a push is genuinely occurring.

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

`completed` is set to `true` by `SaveManager::UpdateBestTime()` when a level is
first cleared and persisted. The next `LevelSelectScene::OnEnter()` reloads and
displays it correctly.
