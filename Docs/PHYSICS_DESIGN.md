# Physics System Design

> This document describes the **fully refactored** physics system. The former
> `CharacterPhysicsSystem::Update` / `PhysicsAgent` / `PhysicsState` approach
> has been retired; all runtime physics now flows through `PhysicsWorld` and
> the individual `IPhysicsBody` implementations.

---

## 1. Architecture Overview

```
IPhysicsBody  (pure interface — IPhysicsBody.hpp)
│
├── PlayerCat            → BodyType::CHARACTER          ✅ fully implemented
├── PushableBox          → BodyType::PUSHABLE_BOX       ✅ fully implemented
├── StaticBody           → BodyType::STATIC_BOUNDARY    ✅ fully implemented
├── PatrolEnemy          → BodyType::PATROL_ENEMY       ⬜ not yet implemented
├── MovingPlatform       → BodyType::MOVING_PLATFORM    ⬜ not yet implemented
├── ConditionalPlatform  → BodyType::CONDITIONAL_PLATFORM ⬜ not yet implemented
├── Rope endpoint        → BodyType::ROPE_ENDPOINT      ⬜ not yet implemented
└── Bullet               → BodyType::BULLET             ⬜ not yet implemented

IPushable  (secondary interface — IPushable.hpp)
└── PushableBox          (also implements IPhysicsBody)

PhysicsWorld  (pipeline coordinator — PhysicsWorld.hpp / .cpp)
├── Register / Unregister / Clear
├── AddStaticBoundary(center, halfSize)  → creates & owns a StaticBody
├── Update()                             → runs the full per-frame pipeline
├── CountCharactersPushing(target, dir)  → chain-push query
├── GetBodiesOfType(type)
├── FreezeAll / UnfreezeAll
└── AddRope / RemoveRope / GetRopesOf    → structural wiring (step not yet implemented)

CharacterPhysicsSystem  (constants header only — CharacterPhysicsSystem.hpp)
└── compile-time float constants shared by PlayerCat and documentation
    (all runtime logic has been moved to PlayerCat::PhysicsUpdate and PhysicsWorld)
```

---

## 2. The Two-Phase Per-Frame Pipeline

`PhysicsWorld::Update()` executes the following stages in order each frame:

```
PhysicsWorld::Update()
│
├── [Purge]  PurgeExpired()         every 60 frames — remove stale weak_ptrs
│
├── [Phase 1]  StepPhysicsUpdate()
│   ├── Pass A — all non-PUSHABLE_BOX active non-frozen bodies call PhysicsUpdate()
│   │            PlayerCat: applies gravity, reads moveDir, sets m_DesiredDelta
│   └── Pass B — all PUSHABLE_BOX active non-frozen bodies call PhysicsUpdate()
│                PushableBox: applies gravity, queries CountCharactersPushing(),
│                sets m_DesiredDelta (characters must run first so their
│                moveDir is already committed when boxes query)
│
├── [Resolve]  StepResolveAndApply()
│   ├── Snapshot all active bodies into vector<BodyInfo>
│   ├── DetectRiding() — record which body each dynamic body sits on top of
│   ├── Mark kinematic / frozen bodies as immediately resolved (desired = {0,0}
│   │   for StaticBody; no collision resolution needed)
│   ├── Iterative topological resolution loop:
│   │   for each unresolved body whose support IS already resolved:
│   │     effective = desired + support's horizontal resolved delta (carry)
│   │     resolved  = ResolveBody(i, effective, infos)
│   │         ResolveBody performs horizontal pass then vertical pass,
│   │         pushing self out of any solid overlap and recording contact normals
│   │   repeat until no progress (handles stacking chains of arbitrary depth)
│   ├── Safety pass — resolve any remaining orphaned bodies without carry
│   └── ApplyResolvedDelta() on every body
│
├── [Callbacks]  StepCollisionCallbacks()
│   └── for each body with recorded contacts: call OnCollision(CollisionInfo)
│       PlayerCat::OnCollision: sets m_Grounded, clears velocityY, sets m_IsPushing
│       PushableBox::OnCollision: clears velocityY on floor/ceiling contact
│
└── [PostUpdate]  calls PostUpdate() on every active non-frozen body
                  PlayerCat::PostUpdate: runs UpdateAnimState()
```

**Why two passes in Phase 1?** `PushableBox::PhysicsUpdate()` calls
`CountCharactersPushing()`, which reads `GetMoveDir()` on CHARACTER bodies.
Characters must commit their `m_MoveDir` (pass A) before boxes query it (pass B);
otherwise, a box might read stale move directions from the previous frame.

---

## 3. Body Types and Interface Contracts

### 3.1 PlayerCat (CHARACTER) — Fully Implemented

`PlayerCat` derives from both `AnimatedCharacter` and `IPhysicsBody`.

```cpp
class PlayerCat : public AnimatedCharacter, public IPhysicsBody {
    // GetBodyType()        → CHARACTER
    // GetPosition()        → m_Transform.translation
    // SetPosition(pos)     → m_Transform.translation = pos
    // GetHalfSize()        → {kHalfWidth=18, kHalfHeight=23}  (fixed, avoids jitter)
    // IsSolid()            → true
    // IsKinematic()        → false
    // GetMoveDir()         → m_MoveDir (-1 / 0 / +1), set by scene or own input
    // GetDesiredDelta()    → m_DesiredDelta = {moveDir * kGroundMoveSpeed, velocityY}
    // ApplyResolvedDelta() → SetPosition(pos + delta)
    // OnCollision()        → sets m_Grounded, clamps velocityY, sets m_IsPushing
    // PhysicsUpdate()      → gravity, optional direct-input read, desired delta
    // PostUpdate()         → UpdateAnimState()
    // NotifyPush()         → m_IsPushing = true  (called by PushableBox)
};
```

**Input sourcing:** if a cat has real key bindings and `m_InputEnabled == true`,
`PhysicsUpdate()` reads `Util::Input` directly. For cats spawned in
`LocalPlayGameScene` with `UNKNOWN` keys, the scene calls `SetMoveDir()` and
`Jump()` externally before `m_World.Update()`.

---

### 3.2 PushableBox (PUSHABLE_BOX) — Fully Implemented

`PushableBox` derives from `Character`, `IPhysicsBody`, and `IPushable`.

```cpp
class PushableBox : public Character, public IPhysicsBody, public IPushable {
    // GetBodyType()         → PUSHABLE_BOX
    // IsSolid()             → true
    // IsKinematic()         → false
    // GetRequiredPushers()  → m_RequiredPushers (set at construction)
    // PhysicsUpdate()       → gravity + CountCharactersPushing query → desired delta
    // ApplyResolvedDelta()  → move box + reposition m_CountText label
    // OnCollision()         → floor/ceiling velocityY clamp
};
```

`m_CountText` is an overlaid `GameText` showing how many more pushers are still
needed (`std::max(0, requiredPushers - |net|)`). Both the box and its text object
must be added to the renderer and registered with the world separately.

`SetWorld(PhysicsWorld*)` must be called before the box is registered, so
`PhysicsUpdate()` can reach `CountCharactersPushing()`.

---

### 3.3 StaticBody (STATIC_BOUNDARY) — Fully Implemented

`StaticBody` is the correct type for **all immovable solid geometry** — floor,
ceiling, left/right boundary walls, and any in-level wall that should never move.

```cpp
class StaticBody final : public IPhysicsBody {
    // GetBodyType()        → STATIC_BOUNDARY (or any BodyType passed at construction)
    // IsSolid()            → true   — blocks all dynamic bodies
    // IsKinematic()        → true   — never moved by the resolver
    // GetDesiredDelta()    → {0, 0} — stationary by definition
    // ApplyResolvedDelta() → no-op
    // SetPosition()        → no-op
};
```

`PhysicsWorld::AddStaticBoundary(center, halfSize)` creates, owns, and
immediately registers a `StaticBody`. The returned raw pointer is only for
identification; ownership stays with `PhysicsWorld::m_OwnedStatics`.

**Wall design rule:** all visual wall sprites (e.g. `background_lwall.png`,
`background_rwall.png`) are purely decorative. Their physics counterpart must be
a `StaticBody` registered via `AddStaticBoundary()`. The AABB should match the
visible surface of the wall, not the entire sprite bounding box.

---

### 3.4 Future Body Types (Not Yet Implemented)

The `BodyType` enum reserves values for `PATROL_ENEMY`, `MOVING_PLATFORM`,
`CONDITIONAL_PLATFORM`, `ROPE_ENDPOINT`, and `BULLET`. These are not required
for the current scope; their implementation can be deferred until the respective
level scenes are built. The rope constraint struct (`RopeConstraint`) and
`AddRope`/`RemoveRope` API already exist in `PhysicsWorld` for future use.

---

## 4. Chain Pushing Algorithm

### 4.1 API

```cpp
// PhysicsWorld public method
int CountCharactersPushing(const IPhysicsBody* target, int dir) const;
//   dir: +1 = querying right-side pushers, -1 = querying left-side pushers
//   Returns the net number of CHARACTER bodies directly or transitively
//   contributing force toward 'target' from the 'dir' origin side.
```

`PushableBox::PhysicsUpdate()` calls this twice:

```cpp
int netRight = m_World->CountCharactersPushing(this, +1);
int netLeft  = m_World->CountCharactersPushing(this, -1);
const int net = netRight - netLeft;
if (std::abs(net) >= m_RequiredPushers)
    vx = (net > 0) ? kPushSpeed : -kPushSpeed;
```

### 4.2 Recursive Traversal — `CountCharactersPushingImpl`

The algorithm walks the adjacency graph recursively, using a `visited` set to
prevent infinite loops.

**Proximity tests** (both must pass for a body to be considered adjacent):

```
vertical band  : |cPos.y - tPos.y| < (tHalf.y + cHalf.y) * 0.9f
horizontal gap : |dxToTarget|      ≤ tHalf.x + cHalf.x + 4.0f (touch tolerance)
direction side : dir=+1 → cPos.x < tPos.x   (pusher is to the left)
                 dir=-1 → cPos.x > tPos.x   (pusher is to the right)
```

**Per-neighbor logic:**

```
CHARACTER body adjacent to target:
    if moveDir == dir → ++count           (actively pushing: counts directly)
    always recurse(sp, dir, visited)      (passive characters still transmit)

PUSHABLE_BOX body adjacent to target:
    recurse(sp, dir, visited)             (box transmits passively, never counts itself)
```

The key insight is that a CHARACTER with `moveDir == 0` does **not** add 1 to
the count but still allows force from active pushers behind it to propagate
forward. This correctly handles all six chain scenarios below.

### 4.3 Verified Scenarios

All positions below are left-to-right. "→" means moveDir = +1. Box bodies are
capital letters in brackets.

---

**Scenario 1 — A pushes B, B pushes C. C is pushed by 2.**

```
A(→) adjacent to B(→) adjacent to [C]   requiredPushers = 2
```

`CountCharactersPushing([C], +1)`:
1. B found adjacent to C; B→: count++ (1). Recurse into B.
2. A found adjacent to B; A→: count++ (1). Recurse into A.
3. Nothing behind A.
4. **Total = 2 ≥ 2** → [C] moves right. ✅

---

**Scenario 2 — A pushes [C], [C] is adjacent to [D]. Both move with 1 pusher.**

```
A(→) adjacent to [C] adjacent to [D]   requiredPushers = 1 each
```

`CountCharactersPushing([C], +1)` = 1 (A found, A→).
`CountCharactersPushing([D], +1)`:
1. [C] found adjacent to D; PUSHABLE_BOX → recurse into [C].
2. A found adjacent to [C]; A→: count++ (1).
3. **Total = 1 ≥ 1** → [D] moves right. ✅

---

**Scenario 3 — A pushes passive B (no key), B is adjacent to [C]. C is pushed by 1.**

```
A(→) adjacent to B(0) adjacent to [C]   requiredPushers = 1
```

`CountCharactersPushing([C], +1)`:
1. B found adjacent to C; B has moveDir=0 → **do not count B**, but still recurse.
2. A found adjacent to B; A→: count++ (1).
3. **Total = 1 ≥ 1** → [C] moves right. ✅

> **This was the pre-fix failure point.** The old code used `continue` when
> `moveDir != dir`, silently skipping both the count and the recursion. The
> fix changes to an `if (moveDir == dir) ++count;` with an unconditional
> `count += recurse(sp, …)` below it.

---

**Scenario 4 — A pushes [C] from the left, B pushes [C] from the right. Forces cancel.**

```
A(→) | [C] | B(←)   requiredPushers = 1
```

`CountCharactersPushing([C], +1)` = 1 (A on left, A→).
`CountCharactersPushing([C], -1)` = 1 (B on right, B←).
`net = 1 − 1 = 0` → `|net| < 1` → [C] does not move. ✅

---

**Scenario 5 — A pushes [C] while both are airborne.**

```
A(→) airborne adjacent to [C] airborne   requiredPushers = 1
```

The vertical-band check allows objects at the same Y level (no ground
contact required). `CountCharactersPushing([C], +1)` = 1.
`ResolveBody` handles the horizontal separation exactly as on the ground.
[C] acquires a horizontal desired delta and is resolved normally. ✅

---

**Scenario 6 — A pushes [C], but [C] is blocked by a wall or IsKinematic object.**

```
A(→) adjacent to [C] | wall(StaticBody)   requiredPushers = 1
```

`CountCharactersPushing([C], +1)` = 1 ≥ 1 → [C]'s desired delta.x = kPushSpeed.
During `ResolveBody`, the wall is already marked `resolvedFlag = true` with
delta = {0,0}. The horizontal pass detects overlap and clamps [C]'s resolved.x
to zero. [C] does not move. The same result applies for any `IsKinematic()` body. ✅

---

## 5. Complete Workflow Example

This example traces a single frame where **PlayerCat A pushes PushableBox [C]**
(requiredPushers = 1) and both are on the floor.

### Frame start state

| Body | Position | VelocityY | MoveDir | DesiredDelta |
|------|----------|-----------|---------|--------------|
| A (PlayerCat) | (100, −294) | 0 | 0 | (0, 0) |
| [C] (PushableBox) | (155, −294) | 0 | — | (0, 0) |
| Floor (StaticBody) | (0, −357) | — | — | (0, 0) |

Player presses right key → scene sets `A->SetMoveDir(+1)` before `m_World.Update()`.

### Step 1 — Input (scene, before Update)

```
A->SetMoveDir(+1)   // scene reads key, informs cat
```

### Step 2 — Phase 1: StepPhysicsUpdate

**Pass A — PlayerCat:**
```
A::PhysicsUpdate():
    m_VelocityY -= kGravity (0.75)  →  velocityY = -0.75  (was 0, now slightly downward)
    m_IsPushing = false
    m_Grounded is reset to false (will be restored by OnCollision)
    vx = moveDir * kGroundMoveSpeed = +1 * 5.0 = +5.0
    m_DesiredDelta = {+5.0, -0.75}
```

**Pass B — PushableBox:**
```
[C]::PhysicsUpdate():
    m_VelocityY -= kGravity (0.5)   →  velocityY = -0.5
    netRight = CountCharactersPushing([C], +1)
        → A is adjacent (dxToTarget = 100-155 = -55; minDistX = 18+32 = 50;
           -55 within 50+4=54 → adjacent; dir=+1, dxToTarget<0 → left side ✓)
        → A->GetMoveDir() == +1 → count++ = 1; recurse into A → nothing behind A
        → returns 1
    netLeft  = CountCharactersPushing([C], -1) = 0
    net = 1 - 0 = 1
    deficit = max(0, 1 - 1) = 0    → m_CountText shows "0"
    |net| = 1 ≥ requiredPushers (1) → vx = +kPushSpeed = +2.0
    m_DesiredDelta = {+2.0, -0.5}
    NotifyAdjacentPushers(+1):
        A is on push side → A->NotifyPush() → A->m_IsPushing = true
```

### Step 3 — Resolve: StepResolveAndApply

Snapshot infos: [Floor (kinematic), A (desired={+5,-0.75}), C (desired={+2,-0.5})]

Floor is marked `resolvedFlag = true` immediately (kinematic).

**Topological pass — resolve A:**
- Floor is already resolved.
- A's supportIdx = Floor (A is standing on floor).
- effective = {+5,-0.75} + {floor.resolved.x=0} = {+5,-0.75}
- ResolveBody(A):
  - Horizontal: A moves to x=105. Check against [C] at x=155 (not yet resolved,
    pre-move). Overlap = (18+32) - |105-155| = 50-50 = 0 → no overlap.
  - Check against Floor: no horizontal overlap.
  - Vertical: A moves to y=-294.75. Check against Floor top edge (-357+40=-317).
    A bottom = -294.75-23 = -317.75; floor top = -317; overlap = -317-(-317.75) = 0.75 > 0
    → push A up: resolved.y = (-317 + 23) - (-294) = -317+23+294 = 0 → wait,
    resolved.y = (otherPos.y + (selfHalf.y + otherHalf.y) * sign) - selfPos.y
               = (-357 + (23+40)*1) - (-294) = -357+63+294 = 0
  - A.resolved = {+5, 0}   (horizontal fine, vertical blocked by floor → grounded)
  - A.collidedV = Floor, normalV = {0,+1}

**Topological pass — resolve [C]:**
- [C]'s supportIdx = Floor.
- effective = {+2,-0.5} + {0} = {+2,-0.5}
- ResolveBody([C]):
  - Horizontal: [C] moves to x=157. No wall adjacent. A is now at resolved
    position x=105 (resolvedFlag=true → otherPos = 100+5 = 105).
    Overlap = (32+18) - |157-105| = 50-52 = -2 → no overlap.
  - Vertical: same floor clamping → resolved.y = 0.
  - [C].resolved = {+2, 0}

**Apply deltas:**
```
A  →  position = (100+5, -294+0) = (105, -294)
[C] → position = (155+2, -294+0) = (157, -294)
```

### Step 4 — Callbacks: StepCollisionCallbacks

```
A.collidedV = Floor, normalV = {0,+1}
    → A::OnCollision: normal.y > 0.5 → m_Grounded = true, m_VelocityY = 0
[C].collidedV = Floor, normalV = {0,+1}
    → [C]::OnCollision: normal.y > 0.5 → m_VelocityY = 0
```

### Step 5 — PostUpdate

```
A::PostUpdate() → UpdateAnimState()
    m_IsPushing = true (set by NotifyAdjacentPushers above)
    → CatAnimState::PUSH
```

**Frame end state:**

| Body | Position | VelocityY | AnimState |
|------|----------|-----------|-----------|
| A | (105, −294) | 0 | PUSH |
| [C] | (157, −294) | 0 | — |

The box moved right 2 px; A moved right 5 px while staying flush against [C].
On the next frame, AABB separation keeps them adjacent at the `minDistX` boundary.

---

## 6. Walls — Design and Registration

All solid, immovable walls in any scene must be registered as `StaticBody` objects
via `PhysicsWorld::AddStaticBoundary()`. This includes:

- **Boundary walls** — left and right screen edges (already implemented in
  `TitleScene`, `MenuScene`, and `LocalPlayGameScene` via `SetupStaticBoundaries()`).
- **In-level platform edges and side walls** — any wall sprite whose physics
  surface should never move.

```cpp
// Pattern used in SetupStaticBoundaries() for left and right walls:
constexpr float kWallHalfW = 50.f;
constexpr float kWallHalfH = 400.f;

// Left wall: center is 50 px to the left of the play area edge
m_World.AddStaticBoundary({-(640.f + kWallHalfW), 0.f}, {kWallHalfW, kWallHalfH});

// Right wall: symmetric
m_World.AddStaticBoundary({ (640.f + kWallHalfW), 0.f}, {kWallHalfW, kWallHalfH});
```

The visual sprites `background_lwall.png` / `background_rwall.png` are purely
decorative `Character` objects. Their physics counterpart is the invisible
`StaticBody` AABB above. The `StaticBody` has `IsSolid() = true` and
`IsKinematic() = true`, so all dynamic bodies (players, boxes) are resolved
against it and can never pass through.

**Chain-pushing interaction with walls (Scenario 6):** when a `PushableBox`
has a `StaticBody` wall on the opposing side, the box correctly refuses to move.
`CountCharactersPushing()` may still return a positive number (the pushers are
present), but `ResolveBody` clamps the box's horizontal delta to zero because
the wall's resolved position leaves no room. The box stays put. ✅

---

## 7. Scene Registration Pattern

```cpp
// ── OnEnter ─────────────────────────────────────────────────────────────────
void LevelOneScene::OnEnter() {
    m_World.Clear();

    // 1. Spawn player cats and register them.
    for (auto& pb : m_Players) {
        m_World.Register(pb.cat);
        m_Ctx.Root.AddChild(pb.cat);
    }

    // 2. Spawn boxes: SetWorld first, then register.
    for (auto& box : m_Boxes) {
        box->SetWorld(&m_World);             // box needs world for CountCharactersPushing
        m_World.Register(box);
        m_Ctx.Root.AddChild(box);
        if (box->GetTextObject()) {
            m_Ctx.Root.AddChild(box->GetTextObject());
        }
    }

    // 3. Register immovable geometry last.
    SetupStaticBoundaries();                  // calls AddStaticBoundary internally
}

// ── Update ───────────────────────────────────────────────────────────────────
Scene* LevelOneScene::Update() {
    // Step 1: read input → set MoveDir / call Jump() on cats.
    for (auto& pb : m_Players) {
        // ... (key reading)
        pb.cat->SetMoveDir(moveDir);
        if (pb.cat->IsGrounded() && wantJump) pb.cat->Jump();
    }

    // Step 2: run unified physics pipeline.
    m_World.Update();

    // Step 3: read-only game logic (goal checks, timer, etc.)
    m_ElapsedSeconds += deltaTime;
    if (/* all players at goal */) {
        SaveManager::UpdateBestTime(0, m_Ctx.SelectedPlayerCount, m_ElapsedSeconds);
        return m_LevelExitScene;  // confirmation scene before returning to menu
    }
    return nullptr;
}

// ── OnExit ───────────────────────────────────────────────────────────────────
void LevelOneScene::OnExit() {
    for (auto& pb : m_Players) m_Ctx.Root.RemoveChild(pb.cat);
    for (auto& box : m_Boxes) {
        m_Ctx.Root.RemoveChild(box);
        if (box->GetTextObject()) m_Ctx.Root.RemoveChild(box->GetTextObject());
    }
    m_World.Clear();
}
```

**`LevelOneScene`** is the scene for the actual gameplay level.
**`LevelExitScene`** is the confirmation dialog shown before exiting any level
(analogous to `ExitConfirmScene` for the main menu). Neither is implemented yet;
both are empty stubs.

---

## 8. CharacterPhysicsSystem Constants

`CharacterPhysicsSystem.hpp` is now a **constants-only header**. All runtime
logic has moved to `PlayerCat::PhysicsUpdate()` and `PhysicsWorld`. The
constants are duplicated as `static constexpr` members of `PlayerCat` for
self-containment; the header exists only for backward-reference convenience.

| Constant | Value | Description |
|----------|-------|-------------|
| `kGroundMoveSpeed` | 5.0f | horizontal speed (px/frame) on ground |
| `kRunOnPlayerSpeed` | 6.2f | horizontal speed when standing on another character |
| `kJumpForce` | 11.0f | initial vertical velocity on jump |
| `kGravity` | 0.75f (PlayerCat) / 0.5f (PushableBox) | velocityY decrement per frame |
| `kScreenHalfW` | 640.0f | half of 1280×720 |
| `kScreenHalfH` | 360.0f | half of 1280×720 |
| `kHalfWidth` (PlayerCat) | 18.0f | collision box half-width (fixed to prevent jitter) |
| `kHalfHeight` (PlayerCat) | 23.0f | collision box half-height |
| `kPushSpeed` (PushableBox) | 2.0f | box horizontal speed when pushed |

---

## 9. Implementation Status

| Feature | Status |
|---------|--------|
| `IPhysicsBody` interface | ✅ |
| `IPushable` interface | ✅ |
| `StaticBody` | ✅ |
| `PlayerCat` (CHARACTER) — full physics | ✅ |
| `PushableBox` (PUSHABLE_BOX) — full physics | ✅ |
| `PhysicsWorld::Register / Unregister / Clear` | ✅ |
| `PhysicsWorld::AddStaticBoundary` | ✅ |
| `PhysicsWorld::Update` — two-phase pipeline | ✅ |
| `PhysicsWorld::StepPhysicsUpdate` (two-pass ordering) | ✅ |
| `PhysicsWorld::StepResolveAndApply` (topological) | ✅ |
| `PhysicsWorld::StepCollisionCallbacks` | ✅ |
| `PhysicsWorld::DetectRiding` (stacking carry) | ✅ |
| `PhysicsWorld::CountCharactersPushing` — all 6 chain scenarios | ✅ |
| `PhysicsWorld::FreezeAll / UnfreezeAll` | ✅ |
| `PhysicsWorld::GetBodiesOfType` | ✅ |
| `PhysicsWorld::PurgeExpired` | ✅ |
| `AddRope / RemoveRope / GetRopesOf` (structure) | ✅ |
| `StepRopes` (rope force resolution) | ❌ TODO |
| `PatrolEnemy` | ❌ TODO |
| `MovingPlatform` | ❌ TODO |
| `ConditionalPlatform` | ❌ TODO |
| `Bullet` | ❌ TODO |
| `LevelOneScene` (actual gameplay) | ❌ not yet implemented |
| `LevelExitScene` (level-exit confirmation) | ❌ not yet implemented |
