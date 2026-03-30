# Component Interaction & Data Flow

> Static OOP structure → `CLASS_RELATIONSHIPS.md`.
> Scene transition diagram → `ARCHITECTURE.md §11`.
> Physics object interface and chain-push algorithm → `PHYSICS_DESIGN.md`.
> This document covers **runtime** data flow, call sequences, and coordination.

## Contents
1. [Main Loop Data Flow](#1-main-loop-data-flow)
2. [Scene Lifecycle Protocol](#2-scene-lifecycle-protocol)
3. [Input → Physics → Render Pipeline (LocalPlayGameScene)](#3-input--physics--render-pipeline-localplaygamescene)
4. [Input → Physics → Render Pipeline (LevelOneScene)](#4-input--physics--render-pipeline-levelonescene)
5. [PhysicsWorld Internal Pipeline](#5-physicsworld-internal-pipeline)
6. [GameContext as Cross-scene Bus](#6-gamecontext-as-cross-scene-bus)
7. [Settings Confirmation Pattern](#7-settings-confirmation-pattern)
8. [Character Animation State Machine](#8-character-animation-state-machine)
9. [BGMPlayer & Volume Control](#9-bgmplayer--volume-control)
10. [SaveManager Data Flow](#10-savemanager-data-flow)
11. [LevelSelectScene Interaction Flow](#11-levelselectscene-interaction-flow)
12. [LevelOneScene Gameplay Mechanics](#12-levelonescene-gameplay-mechanics)

---

## 1. Main Loop Data Flow

```
Core::Context (PTSD engine)
    │  each frame
    ▼
App::Update()                              AppUpdate.cpp
    ├─→ m_Ctx->BGMPlayer->Update()         detect track end, advance to next
    ├─→ m_SceneManager->UpdateCurrent()    Scene::Update() returns SceneId
    │       └─→ if next != SceneId::None && next != currentId
    │               └─→ SceneManager::GoTo(next)
    │                       ├─ current->OnExit()
    │                       └─ next->OnEnter()
    ├─→ check m_Ctx->ShouldQuit            if true → State = END
    └─→ m_Ctx->Root.Update()              Util::Renderer: render all children
```

**Key points:**
- `Scene::Update()` returns `SceneId`; this is the only legal scene-switch trigger.
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

**Permanent-object hide exception (LevelSelectScene + LevelOneScene):**
these scenes use `SetVisible(...)` for permanent objects owned by `App`
instead of `RemoveChild(...)`.

- `LevelSelectScene`: hides `Header/Floor/Door/TestBox` on `OnEnter`, restores
  `Header/Floor/Door` on `OnExit` (TestBox visibility is not restored here).
- `LevelOneScene`: hides `Header/Floor/TestBox` on `OnEnter` and currently keeps
  them hidden on `OnExit` in the current implementation.

---

## 3. Input → Physics → Render Pipeline (LocalPlayGameScene)

The complete per-frame flow in `LocalPlayGameScene::Update()`:

```
Step 1  Read input → set move directions on each cat
        IsKeyPressed(key.left / key.right) → cat->SetMoveDir(dir)
        IsKeyDown(key.jump) && cat->IsGrounded() → cat->Jump()
            Jump() sets: m_VelocityY = kJumpForce, m_Grounded = false

Step 2  Door-entry check (positional logic; runs before physics)
        For each non-entered player:
            if cat within door AABB (door half + 10px margin) && IsKeyDown(key.up):
                pb.entered = true, cat->SetActive(false), ++m_EnteredCount
                UpdateDoorCountText()
                if m_EnteredCount == SelectedPlayerCount → return SceneId::LevelSelect

Step 3  Unified physics pipeline
        m_World.Update()
        → Phase 1 : all cats + boxes self-drive (PhysicsUpdate)
        → Resolve  : collision resolution (AABB separation, stacking carry)
        → Apply    : ApplyResolvedDelta → positions updated
        → Callbacks: OnCollision → grounded flags, velocityY clamps, m_IsPushing
        → PostUpdate: animation state machine

Step 4  Game statistics (pure read, no displacement)
        UpdateCooperativePower() → writes m_Ctx.CooperativePushPower
            Counts players that share the same moveDir and are within
            halfWidth * 1.7 * 1.02 × 40px AABB of each other;
            the largest co-moving group size is stored.
```

**Ordering rule:** `SetMoveDir()` and `Jump()` must be called **before**
`m_World.Update()` because `PhysicsWorld::StepPhysicsUpdate()` reads
`GetMoveDir()` and integrates velocity during `PhysicsUpdate()`. Input applied
after `m_World.Update()` silently takes effect one frame late.

---

## 4. Input → Physics → Render Pipeline (LevelOneScene)

`LevelOneScene::Update()` follows a similar but extended flow to handle the
key/door puzzle mechanics:

```
Step 1  ESC check (returns SceneId::LevelSelect)

Step 2  Read input → set move directions on each player
        HandlePlayerInput():
            IsKeyPressed(key.left / key.right) → cat->SetMoveDir(dir)
            IsKeyDown(key.jump) && cat->IsGrounded() → cat->Jump()

Step 3  Unified physics pipeline
        m_World.Update()    ← same two-phase pipeline as LocalPlayGameScene

Step 4  Timer update
        m_ElapsedSec += Util::Time::GetDeltaTimeMs() / 1000.0f
        UpdateTimerText() → "TIME mm:ss.cs" via SaveManager::FormatTime

Step 5  Key pickup check
        TryPickKey():
            if m_KeyCarrierIdx < 0:
                for each player: AABB(cat, keyHalf={20,26}) → first overlap wins
                m_KeyCarrierIdx = winning player index

Step 6  Key follow
        UpdateKeyFollow():
            if m_KeyCarrierIdx >= 0:
                key position = carrier position + offset{-28, +28}

Step 7  Door open check
        TryOpenDoorAndClear():
            if m_KeyCarrierIdx >= 0 && !m_DoorOpened:
                if AABB(carrier, door) → m_DoorOpened = true
                    door image → door_open.png, key → SetVisible(false)

Step 8  Door entry check
        UpdateDoorEntryAndClear():
            if m_DoorOpened:
                for each non-entered player:
                    if AABB(cat, door) && IsKeyDown(key.up):
                        m_PlayerEntered[i] = true, ++m_EnteredCount
                        cat->SetVisible(false) / SetInputEnabled(false) / SetActive(false)
                if m_EnteredCount == totalPlayers && !m_ClearDone:
                    SaveManager::UpdateBestTime(0, SelectedPlayerCount, m_ElapsedSec)
                    m_ClearDone = true

Step 9  Clear transition
        if m_ClearDone → return SceneId::LevelSelect
```

The key difference from `LocalPlayGameScene` is that all game-mechanic checks
(key pickup, door open, door entry, clear) run *after* `m_World.Update()`.
This guarantees that character positions are already at their resolved final
locations for this frame before overlap tests are performed.

---

## 5. PhysicsWorld Internal Pipeline

`PhysicsWorld::Update()` internally orchestrates four sub-steps. This section
documents what happens inside each one.

### 5.1 Phase 1 — `StepPhysicsUpdate()`

```
Pass A (non-PUSHABLE_BOX bodies):
    for each active, non-frozen body except PUSHABLE_BOX:
        body->PhysicsUpdate()

    PlayerCat::PhysicsUpdate():
        m_PrevGrounded = m_Grounded   // snapshot for "just landed" detection
        m_Grounded = false            // will be restored by OnCollision
        m_IsPushing = false           // will be restored by NotifyAdjacentPushers
        m_VelocityY -= kGravity (0.75)
        read own keys if (m_InputEnabled && key bindings are not UNKNOWN)
        m_DesiredDelta = {moveDir * kGroundMoveSpeed(5.0), m_VelocityY}
        flip m_Transform.scale.x to face movement direction

Pass B (PUSHABLE_BOX bodies):
    for each active, non-frozen PUSHABLE_BOX:
        box->PhysicsUpdate()

    PushableBox::PhysicsUpdate():
        m_VelocityY -= kGravity (0.5, different from PlayerCat)
        netRight = m_World->CountCharactersPushing(this, +1)
        netLeft  = m_World->CountCharactersPushing(this, -1)
        net = netRight - netLeft
        deficit = max(0, requiredPushers - |net|)
        update m_CountText to show deficit
        vx = kPushSpeed(2.0) * sign(net)  if |net| >= requiredPushers, else 0
        m_DesiredDelta = {vx, m_VelocityY}
        if net != 0: NotifyAdjacentPushers(activeDir)
            → calls ch->NotifyPush() on adjacent characters on the push side
```

**Why does Pass B come second?** `CountCharactersPushing()` reads `GetMoveDir()`
on CHARACTER bodies. If boxes ran first, they would see stale `moveDir` values
from the previous frame. The two-pass ordering guarantees all `moveDir` values
are current-frame before any box queries them.

### 5.2 Resolve — `StepResolveAndApply()`

```
1. Snapshot:  build vector<BodyInfo> from all active bodies
              each entry: body, desired delta, resolved=desired, resolvedFlag=false, supportIdx=-1

2. DetectRiding():
   for each non-kinematic body A:
     for each solid body B:
       if A's bottom edge is within 5 px (kRidingTolerance) of B's top edge
          AND horizontal overlap > 85% of combined half-widths:
           A.supportIdx = B's index  (A is riding B)

3. Immediate resolution for kinematic / frozen bodies:
   StaticBody / frozen body: resolvedFlag=true, resolved={0,0}

4. Iterative topological loop (repeat until no progress):
   for each body i that is NOT yet resolved:
     if its supportIdx j is still unresolved → skip (wait for j first)
     effective_delta = i.desired + (j.resolved.x if j is non-kinematic)  (stacking carry)
     i.resolved = ResolveBody(i, effective_delta, infos)
     i.resolvedFlag = true

   ResolveBody():
     Horizontal pass: sweep body by resolved.x;
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
before their own collision is resolved — producing correct stacking behaviour
without extra special-case code.

### 5.3 Callbacks — `StepCollisionCallbacks()`

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

### 5.4 PostUpdate

```
for each active non-frozen body:
    body->PostUpdate()

PlayerCat::PostUpdate():
    → UpdateAnimState()
        priority (high to low):
          airborne rising        → JUMP_RISE
          airborne falling       → JUMP_FALL
          just landed            → LAND (hold until clip ends — checked via IfAnimationEnds)
          m_IsPushing            → PUSH
          moveDir != 0           → RUN
          (default)              → STAND
```

### 5.5 Chain-Pushing Query — `CountCharactersPushing(target, dir)`

This is called by `PushableBox::PhysicsUpdate()` during Pass B of Phase 1.
The algorithm walks the adjacency graph recursively:

```
CountCharactersPushingImpl(target, dir, visited):
    add target to visited
    for each active body sp adjacent to target on the correct side:
        adjacent = within (tHalf.x + cHalf.x + 4px) horizontally
                   AND within 90% of combined heights vertically
                   AND positioned on the origin side of the push force
        if sp is CHARACTER:
            if sp->GetMoveDir() == dir → ++count       (active pusher)
            always recurse into sp                      (passive chain transmission)
        if sp is PUSHABLE_BOX:
            recurse into sp                             (box transmits passively)
    return count
```

The unconditional recursion for `CHARACTER` bodies (regardless of their own
`moveDir`) is essential for the passive chain scenario: a stationary character
standing between an active pusher and the box still transmits the upstream force.
See `PHYSICS_DESIGN.md §4` for the full scenario verification table.

---

## 6. GameContext as Cross-scene Bus

`GameContext` is the only legal channel for sharing state between scenes.

```
LocalPlayScene          writes SelectedPlayerCount (via m_Ctx.SelectedPlayerCount = m_PlayerCount)
LocalPlayGameScene      reads  SelectedPlayerCount → SpawnPlayers(count)
LevelOneScene           reads  SelectedPlayerCount → clamp active player count (2..8),
                                                      best-time index on clear
                       (Box required-pusher thresholds are currently fixed at
                        scene construction time, not recomputed each OnEnter)
LevelSelectScene        reads  SelectedPlayerCount → UpdateBestTimeText()


LocalPlayGameScene      writes CooperativePushPower (currently informational; not consumed)

KeyboardConfigScene     writes AppliedKeyConfigs[0..7] via SyncAppliedToContext()
LocalPlayGameScene      reads  AppliedKeyConfigs[i] → per-player key binding at spawn
LevelOneScene           reads  AppliedKeyConfigs[i] → per-player key binding at spawn

OptionMenuScene         calls  m_Ctx.BGMPlayer->SetVolume(pending * 6)   on adjust (live preview)
                               m_Ctx.BGMPlayer->SetVolume(applied * 6)   on OK/CANCEL
                               m_Ctx.Background->SetImage(path)          on bg color change

LevelOneScene           reads  m_Ctx.Door (swaps open/close image; sets position)
LevelOneScene           reads  m_Ctx.Header, m_Ctx.Floor, m_Ctx.TestBox
                               (hides on enter; currently remains hidden on exit)

LocalPlayGameScene      reads  m_Ctx.Floor (for boundary positioning)
                               m_Ctx.Door  (swaps open/close image; door count text position)
```

---

## 7. Settings Confirmation Pattern

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
                          (OptionMenuScene also calls SetVolume(applied*6) and
                           SetImage(appliedBgPath) to revert live previews)
```

**Load timing (constructor):**
```
OptionMenuScene()       → SaveManager::LoadOptionSettings()  → init m_Applied
KeyboardConfigScene()   → SaveManager::LoadKeyConfigs()      → init m_Applied[0..7]
                        → SyncAppliedToContext()             → init m_Ctx.AppliedKeyConfigs
```

**LocalPlayScene entry gate:**
```
ENTER pressed
    → configuredCount = m_KbConfigScene->GetConfiguredPlayerCount()
         (players with AllKeys().size() >= 4 non-UNKNOWN bindings)
    → if m_PlayerCount <= configuredCount → return SceneId::LocalPlayGame
    → else → show red warning text ("No keyboard config"), do not transition
```

---

## 8. Character Animation State Machine

`PlayerCat::UpdateAnimState()` runs inside `PostUpdate()` every frame.
Priority order (high → low):

```
Condition                              → Target State
───────────────────────────────────    ─────────────
cur == LAND && !IfAnimationEnds()      LAND  (hold until 80ms clip completes)
!grounded && velocityY > 0             JUMP_RISE
!grounded && velocityY <= 0            JUMP_FALL
grounded && !prevGrounded              LAND  (justLanded; triggers only once)
m_IsPushing                            PUSH
moveDir != 0                           RUN
(otherwise)                            STAND
```

Note: the LAND hold check runs at the top of `UpdateAnimState()` before all
other conditions — this ensures the landing clip plays to completion even if
the player immediately begins moving or pushing.

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

## 9. BGMPlayer & Volume Control

`BGMPlayer` manages three MP3 tracks (`ppc.mp3`, `pp1.mp3`, `pp2.mp3`) with
automatic sequential playback. It uses a **static callback + flag** pattern to
bridge SDL_mixer's audio thread to the main thread safely:

```
SDL_mixer music-end (audio thread)
    └── BGMPlayer::MusicFinishedCallback()
            └── s_ShouldPlayNext = true   (atomic-safe static flag)

Main thread (each frame via App::Update)
    └── BGMPlayer::Update()
            └── if s_ShouldPlayNext → s_ShouldPlayNext = false → Next() → Play()
```

**Volume control timing:**
- `OptionMenuScene::AdjustLeft/Right` (row 2, BGM VOLUME, range 0–20) → **live preview**: calls `BGMPlayer::SetVolume(pending * 6)` immediately. Multiplier 6 maps the 0–20 UI range to SDL_mixer's 0–120 internal range.
- OK confirmed → `SetVolume(applied * 6)` + write to `settings.json`.
- CANCEL / ESC → `SetVolume(applied * 6)` to revert preview; no file write.

---

## 10. SaveManager Data Flow

### 10.1 settings.json Read/Write Sequence

```
App startup
    ├─ OptionMenuScene()   → LoadOptionSettings()  → init m_Applied
    │                         applies: BGMPlayer volume + Background image
    └─ KeyboardConfigScene() → LoadKeyConfigs()    → init m_Applied[0..7]
                              → SyncAppliedToContext() → init m_Ctx.AppliedKeyConfigs

OptionMenuScene OK   → SaveOptionSettings(toSave)
    read-modify-write settings.json (only option fields; keyConfigs untouched)

KeyboardConfigScene CommitPending → SaveKeyConfigs(saveData)
    read-modify-write settings.json (only keyConfigs; option fields untouched)
```

### 10.2 save_data.json Read/Write Sequence

```
LevelSelectScene::OnEnter()
    → LoadLevelData(m_LevelData) → update crown visibility, best time text

LevelOneScene completion (all players entered door)
    → UpdateBestTime(0, SelectedPlayerCount, m_ElapsedSec)
        ├─ LoadLevelData(levels)
        ├─ if elapsed < bestTimes[playerCount] (or bestTimes[p] == −1.0)
        │       → bestTimes[p] = elapsed, completed = true
        │       → SaveLevelData(levels)
        └─ else: no-op (existing record stands)
```

### 10.3 Why Read-Modify-Write?

`settings.json` is shared by `OptionMenuScene` and `KeyboardConfigScene`. A direct overwrite by either would erase the other's data. Each scene reads the current file, modifies only its own fields, and writes the merged result back.

---

## 11. LevelSelectScene Interaction Flow

### 11.1 Entry

```
LocalPlayGameScene::Update()
    └─ all players entered door (m_EnteredCount >= SelectedPlayerCount)
           → return SceneId::LevelSelect
               → SceneManager::GoTo(SceneId::LevelSelect)
                   → LevelSelectScene::OnEnter()
                       ├─ SaveManager::LoadLevelData(m_LevelData)
                       ├─ AddChild all UI objects (selector, title, best-time, covers, crowns)
                       ├─ m_Ctx.Header->SetVisible(false)
                       ├─ m_Ctx.Floor / Door / TestBox → SetVisible(false)
                       ├─ StartupCats → SetVisible(false)
                       └─ UpdateSelectorPos / UpdateTitleText / UpdateBestTimeText / UpdateCrowns
```

### 11.2 Per-frame Update

```
LevelSelectScene::Update()
    ├─ ESC → return SceneId::LocalPlayGame
    ├─ WASD / arrow keys → update m_SelectedIdx (clamped, grid-aware)
    │       → UpdateSelectorPos + UpdateTitleText + UpdateBestTimeText
    ├─ Mouse hover → AppUtil::IsMouseHovering(*m_LevelCover[i])
    │       → auto-switch m_SelectedIdx (same updates as above)
    └─ ENTER or left-click on cover → confirmed
            ├─ if m_LevelSceneIds[i] != SceneId::None → return that SceneId
            └─ else: stay in LevelSelectScene (levels 2–10 not yet implemented)
```

### 11.3 Best Time Display

```
UpdateBestTimeText()
    ├─ p    = m_Ctx.SelectedPlayerCount
    ├─ best = m_LevelData[m_SelectedIdx].bestTimes[p]
    │         (index p into array<float,9>; indices 0–1 unused)
    └─ m_BestTimeText->SetText(
           to_string(p) + " PLAYERS BEST TIME: " + SaveManager::FormatTime(best))
               FormatTime: −1.0f → "--:--.--"
                           ≥0.0f → "mm:ss.cs"
```

### 11.4 Crown (Completion) Display

```
UpdateCrowns()
    └─ for i in 0..LEVEL_COUNT-1:
           m_Crown[i]->SetVisible(m_LevelData[i].completed)
```

`completed` is set to `true` by `SaveManager::UpdateBestTime()` when a level is
first cleared and persisted. The next `LevelSelectScene::OnEnter()` reloads and
displays the crown correctly.

---

## 12. LevelOneScene Gameplay Mechanics

This section documents the runtime interaction between the key, door, timer, and
player systems — all of which are unique to `LevelOneScene` and not present in
the lobby (`LocalPlayGameScene`).

### 12.1 Key Pickup and Follow

```
Key initial position: (−400, −220) in world space
Key half-size: {20, 26}

TryPickKey() — called every frame until m_KeyCarrierIdx >= 0:
    for i in 0..players.size():
        if AABB(players[i].cat, kKeyHalf) overlaps AABB(key, kKeyHalf):
            m_KeyCarrierIdx = i; return

UpdateKeyFollow() — called every frame after TryPickKey():
    if m_KeyCarrierIdx >= 0:
        key.SetPosition(carrier.GetPosition() + {−28, +28})
        (offset keeps the key visible above-left of the carrier)
```

### 12.2 Door Open Sequence

```
Door initial position: (420, kRoomFloorY + kDoorHalf.y)  = (420, −303)
Door half-size: {31, 34}  (derived from scaled sprite size in TryOpenDoorAndClear)

TryOpenDoorAndClear() — called every frame after UpdateKeyFollow():
    if m_KeyCarrierIdx >= 0 && !m_DoorOpened:
        if AABB(carrier, m_Ctx.Door): (exact scaled size * 0.5 used as half-size)
            m_DoorOpened = true
            m_Ctx.Door->SetImage(door_open.png)
            m_KeySprite->SetVisible(false)
```

Note: after `TryOpenDoorAndClear()` runs, the key sprite is hidden but
`m_KeyCarrierIdx` is not cleared. The carrier still "holds" the key logically;
this has no functional consequence since the door is already open.

### 12.3 Door Entry and Level Clear

```
UpdateDoorEntryAndClear() — called every frame after TryOpenDoorAndClear():
    if !m_DoorOpened: return

    doorPos  = m_Ctx.Door->GetPosition()
    doorHalf = |m_Ctx.Door->GetScaledSize()| * 0.5

    for i in 0..players.size():
        if already entered: skip
        if AABB(cat[i], doorHalf) && IsKeyDown(key.up):
            m_PlayerEntered[i] = true, ++m_EnteredCount
            cat→SetVisible(false), SetInputEnabled(false), SetActive(false)
            cat→SetPosition({640, −360})   (off-screen parking position)

    if !m_ClearDone && m_EnteredCount == totalPlayers:
        SaveManager::UpdateBestTime(0, SelectedPlayerCount, m_ElapsedSec)
        m_ClearDone = true

Then in Update(), after UpdateDoorEntryAndClear():
    if m_ClearDone → return SceneId::LevelSelect
```

All players must enter individually by pressing their own `up` key while
overlapping the open door. There is no time limit — players may enter in any
order. The timer continues running until the last player enters.

### 12.4 PushableBox Configuration

`LevelOneScene` creates two boxes in its constructor:
- `m_BoxA` requires `m_Ctx.SelectedPlayerCount − 1` pushers (constructor-time value)
- `m_BoxB` requires `m_Ctx.SelectedPlayerCount` pushers (constructor-time value)

Because `LevelOneScene` is instantiated once in `AppStart.cpp`, these required
counts are currently fixed after construction and are **not** recalculated on
each `OnEnter()`.

