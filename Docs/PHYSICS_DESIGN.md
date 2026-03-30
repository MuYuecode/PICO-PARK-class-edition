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
├── CountCharactersPushing(target, dir)  → recursive chain-push query
├── GetBodiesOfType(type)                → returns all active bodies of given type
├── FreezeAll / UnfreezeAll              → for game-pause without destroying state
└── AddRope / RemoveRope / GetRopesOf    → structural wiring (step not yet implemented)

CharacterPhysicsSystem  (constants header only — CharacterPhysicsSystem.hpp)
└── compile-time float constants shared by PlayerCat and documentation
    (all runtime logic has been moved to PlayerCat::PhysicsUpdate and PhysicsWorld)
```

### 1.1 Scene lifecycle trigger (post-refactor)

- Scene transitions are now dispatched by `SceneManager` via `SceneId` (not scene pointers).
- `SceneManager::GoTo(...)` is the single place that calls `OnExit()` on the old scene and `OnEnter()` on the new scene.
- Physics cleanup timing remains unchanged: scenes still call `m_World.Clear()` in `OnExit()` and re-register in `OnEnter()`.

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
│   ├── Mark kinematic / frozen bodies as immediately resolved
│   │   StaticBody: desired = {0,0}, resolved = {0,0}, resolvedFlag = true
│   │   Frozen body: same (held in place, not moved by world)
│   ├── Iterative topological resolution loop:
│   │   for each unresolved body whose support IS already resolved:
│   │     effective = desired + support's horizontal resolved delta (carry)
│   │     resolved  = ResolveBody(i, effective, infos)
│   │         ResolveBody performs horizontal pass then vertical pass,
│   │         pushing self out of any solid overlap and recording contact normals
│   │   repeat until no progress (handles stacking chains of arbitrary depth)
│   ├── Safety pass — resolve any remaining orphaned/cyclic bodies without carry
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

**Physics constants** (defined in `PlayerCat.hpp` and mirrored in `CharacterPhysicsSystem.hpp`):

| Constant | Value | Usage |
|----------|-------|-------|
| `kGravity` | 0.75 | subtracted from `m_VelocityY` every `PhysicsUpdate` |
| `kJumpForce` | 11.0 | `m_VelocityY` set to this value on `Jump()` |
| `kGroundMoveSpeed` | 5.0 | horizontal velocity = `moveDir * kGroundMoveSpeed` |
| `kRunOnPlayerSpeed` | 6.2 | **defined but not yet used** — reserved for a future mechanic where cats move faster when standing on another cat |
| `kHalfWidth` | 18.0 | AABB half-size X |
| `kHalfHeight` | 23.0 | AABB half-size Y |

Note: `kGravity` and `kRunOnPlayerSpeed` appear in both `PlayerCat.hpp` and `CharacterPhysicsSystem.hpp` with identical values. This is a deliberate duplication for documentation convenience but represents a technical debt — if the values ever diverge, it could cause subtle bugs.

**Per-frame flow in `PhysicsUpdate()`:**
1. Snapshot `m_Grounded` into `m_PrevGrounded`; reset `m_Grounded = false` and `m_IsPushing = false`.
2. Apply gravity: `m_VelocityY -= kGravity`.
3. If `m_InputEnabled` and key bindings are not `UNKNOWN`, read keys directly; otherwise use `m_MoveDir` set by the scene.
4. If `m_InputEnabled`, check jump key: `IsKeyDown(jumpKey) && m_PrevGrounded → Jump()`.
5. Compute `m_DesiredDelta = {moveDir * kGroundMoveSpeed, m_VelocityY}`.
6. Flip `m_Transform.scale.x` to face the movement direction.

**`ApplyResolvedDelta`:** simply adds `delta` to `m_Transform.translation`.

**`OnCollision`:**
- `normal.y > +0.5` (floor contact): set `m_Grounded = true`, zero `m_VelocityY`.
- `normal.y < -0.5` (ceiling contact): clamp `m_VelocityY` to 0 if currently rising.
- `|normal.x| > 0.5` (wall contact) with non-zero `moveDir` in the blocked direction: set `m_IsPushing = true`.

### 3.2 PushableBox (PUSHABLE_BOX) — Fully Implemented

`PushableBox` derives from `Character`, `IPhysicsBody`, and `IPushable`.

**Physics constants:**

| Constant | Value | Usage |
|----------|-------|-------|
| `kGravity` | 0.5 | boxes fall slower than cats |
| `kPushSpeed` | 2.0 | horizontal velocity when net push force meets the threshold |

**Per-frame flow in `PhysicsUpdate()`:**
1. Apply gravity: `m_VelocityY -= kGravity`.
2. Query net push force: `netRight = CountCharactersPushing(this, +1)`, `netLeft = CountCharactersPushing(this, -1)`, `net = netRight - netLeft`.
3. Compute deficit = `max(0, m_RequiredPushers - |net|)` and update `m_CountText` to display it.
4. If `|net| >= m_RequiredPushers`: `vx = kPushSpeed * sign(net)`, else `vx = 0`.
5. `m_DesiredDelta = {vx, m_VelocityY}`.
6. Call `NotifyAdjacentPushers(activeDir)` to trigger the PUSH animation on contributing cats.

**`ApplyResolvedDelta`:** adds `delta` to position; also repositions `m_CountText` to `{newPos.x + 10, newPos.y}` so the label tracks the box without scene intervention.

**`NotifyAdjacentPushers(activeDir)`:** scans all CHARACTER bodies in the world. For each character within `(chHalf.x + myHalf.x) * 1.15` horizontally and `(chHalf.y + myHalf.y) * 0.90` vertically, whose `moveDir` matches `activeDir` and who is on the correct push side, calls `ch->NotifyPush()` to set `m_IsPushing = true`.

### 3.3 StaticBody (STATIC_BOUNDARY) — Fully Implemented

`StaticBody` is a pure-collision object with no visual component. It always returns `IsSolid() = true` and `IsKinematic() = true`. `SetPosition` and `ApplyResolvedDelta` are no-ops. It is created and exclusively owned by `PhysicsWorld::AddStaticBoundary()`, which stores it in `m_OwnedStatics` and registers it. When `PhysicsWorld::Clear()` is called, all owned statics are destroyed with the world.

The `BodyType` passed to `AddStaticBoundary` defaults to `STATIC_BOUNDARY`; passing other types (e.g., future `MOVING_PLATFORM`) is supported by the interface but no implementation exists yet.

---

## 4. Chain-Push Algorithm

### 4.1 Problem Statement

The cooperative push mechanic requires counting how many characters are
collectively pushing on a box (or chain of boxes and characters). A single
character on its own may not meet the threshold (`m_RequiredPushers`); a group
pushing in the same direction can.

The algorithm must handle three scenarios:

| Scenario | Expected Behaviour |
|----------|-------------------|
| `[A(→) → Box]` | Box sees 1 pusher from A. |
| `[A(→) → B(→) → Box]` | Box sees 2 pushers: A transmits through B. |
| `[A(→) → B(0) → Box]` | Box sees 1 pusher: B is passive but transmits A's force. |
| `[A(→) → Box ← B(←)]` | Box sees net 0 (1 right, 1 left). No movement. |

### 4.2 Algorithm — `CountCharactersPushingImpl`

```cpp
CountCharactersPushingImpl(target, dir, visited):
    if target in visited: return 0
    visited.push_back(target)

    count = 0
    for each active body sp adjacent to target:
        // Adjacency criteria:
        //   |cPos.y − tPos.y| < (tHalf.y + cHalf.y) * 0.9   (same vertical band)
        //   |cPos.x − tPos.x| ≤ (tHalf.x + cHalf.x) + 4.0   (touching or nearly so)
        //   sp is on the origin side of the push:
        //       dir=+1: sp.x < target.x  (pusher is to the left)
        //       dir=−1: sp.x > target.x  (pusher is to the right)

        if sp is CHARACTER:
            if sp->GetMoveDir() == dir: ++count    // active contribution
            count += recurse(sp, dir, visited)     // always recurse (passive chain)

        if sp is PUSHABLE_BOX:
            count += recurse(sp, dir, visited)     // box transmits passively

    return count
```

**Why unconditional recursion for CHARACTER?** A passive character (not pressing any key, `moveDir = 0`) sandwiched between an active pusher and the target box still physically transmits the upstream force. If recursion were conditional on `moveDir == dir`, this scenario would report 0 pushers instead of 1, breaking the mechanic.

### 4.3 Scenario Verification

| Chain | Active pushers counted | Net result |
|-------|----------------------|------------|
| `A(→) → Box` | A (+1) | 1 |
| `A(→) → B(→) → Box` | A found via B recursion (+1), B direct (+1) | 2 |
| `A(→) → B(0) → Box` | B recurses into A, A active (+1); B passive (0) | 1 |
| `A(→) → C(box) ← B(←)` | netRight=1, netLeft=1; CountCharactersPushing returns independently | net=0, no move |

---

## 5. Collision Resolution Detail

### 5.1 AABB Overlap Test

```cpp
// Returns true if two bodies overlap (touching edges are NOT overlap)
bool AabbOverlaps(const IPhysicsBody* a, const IPhysicsBody* b) {
    glm::vec2 delta = a->GetPosition() - b->GetPosition();
    glm::vec2 sum   = a->GetHalfSize() + b->GetHalfSize();
    return abs(delta.x) < sum.x && abs(delta.y) < sum.y;
}
```

### 5.2 Riding Detection

Before any body moves, `DetectRiding()` scans for stacking relationships. Body A "rides" body B if:
- A is not kinematic (kinematic bodies define their own path).
- A's bottom edge (`aPos.y - aHalf.y`) is within `kRidingTolerance = 5.0` pixels of B's top edge (`bPos.y + bHalf.y`).
- A and B share at least 85% of their combined horizontal half-width overlap.

If A rides B, `A.supportIdx = B's index in the snapshot`. During the topological loop, A's effective delta inherits B's horizontal resolved delta, producing automatic platform-following and stacking without any special-case code in the scene.

### 5.3 Topological Resolution Loop

```
Repeat until no progress:
    for each unresolved body i:
        if i has a support j AND j is not yet resolved: skip (defer to next pass)
        effective.x = i.desired.x + (j is non-kinematic ? j.resolved.x : 0)
        effective.y = i.desired.y
        i.resolved = ResolveBody(i, effective, infos)
        i.resolvedFlag = true
```

This loop automatically handles stacking chains of any depth. A body on top of a body on top of a platform resolves in three passes: platform first (kinematic, resolved immediately), middle layer second, top layer last, each inheriting the horizontal delta of the one below.

### 5.4 ResolveBody — Horizontal-then-Vertical Separation

```
Horizontal pass:
    testPos = selfPos + {resolved.x, 0}
    for each solid body j (using j's resolved position if already finalised):
        overlap = (selfHalf + jHalf) - |testPos - jPos|
        if overlap.x > 0 AND overlap.y > 0:   // full AABB overlap
            sign = sign(testPos.x - jPos.x)
            resolved.x = (jPos.x + (selfHalf.x + jHalf.x) * sign) - selfPos.x
            record collidedH = j, normalH = {sign, 0}

Vertical pass (using resolved horizontal position):
    testPos = selfPos + {resolved.x, resolved.y}
    for each solid body j:
        if overlap.x > 0 AND overlap.y > 0:
            sign = sign(testPos.y - jPos.y)
            resolved.y = (jPos.y + (selfHalf.y + jHalf.y) * sign) - selfPos.y
            record collidedV = j, normalV = {0, sign}
```

The horizontal pass runs first to correctly handle corner cases. Solving Y after X means a body sliding diagonally along a wall resolves its horizontal position first, then checks vertically with the corrected X — this prevents false ceiling hits when a body grazes a wall corner.

When another body has already been resolved (`infos[j].resolvedFlag == true`), its position is advanced by `j.resolved` before overlap-testing. This ensures that a body resolving against a moving platform or a just-resolved box sees that object's final-frame position, not its pre-move position.

---

## 6. Rope Constraint System (Structural Wiring — Not Yet Active)

`PhysicsWorld` stores `RopeConstraint` records but does not step them:

```cpp
struct RopeConstraint {
    IPhysicsBody* bodyA   = nullptr;
    IPhysicsBody* bodyB   = nullptr;
    float         maxLen  = 200.f;
    float         friction = 0.5f;
    // TODO: implement force resolution in StepRopes()
};
```

`AddRope()`, `RemoveRope()`, and `GetRopesOf()` exist for future use. The
`Update()` pipeline does not call any rope-stepping function. When implemented,
`StepRopes()` would run after `StepPhysicsUpdate()` and before
`StepResolveAndApply()`, adding constraint forces to bodies' desired deltas.

---

## 7. Freeze / Pause System

`IPhysicsBody` exposes `Freeze()`, `Unfreeze()`, and `IsFrozen()`. `PhysicsWorld` exposes `FreezeAll()` and `UnfreezeAll()` as convenience wrappers.

A frozen body:
- Has its `desired` delta overridden to `{0, 0}` in `StepResolveAndApply()` — it does not move regardless of velocity.
- Is marked as `resolvedFlag = true` immediately (no collision resolution performed against it as a resolver, though solid frozen bodies still block other bodies normally).
- Is skipped in `StepPhysicsUpdate()` (Phase 1 and PostUpdate) — its state does not evolve.

This design allows the entire simulation to be paused cleanly without destroying body state. On `UnfreezeAll()`, bodies resume from their prior state.

---

## 8. Body Lifecycle and Purging

`PhysicsWorld` stores bodies as `vector<weak_ptr<IPhysicsBody>>`. This means:
- Scenes do not need to call `Unregister()` when a body is destroyed; the expired weak pointer is cleaned up automatically.
- Every `kPurgeInterval = 60` frames, `PurgeExpired()` removes all expired weak pointers from the list.
- `Clear()` removes everything unconditionally and destroys owned `StaticBody` instances.

**Recommended scene pattern:** call `m_World.Clear()` on `OnExit` rather than calling `Unregister()` individually. Since the scene owns all cat and box shared pointers, letting `Clear()` release the weak references is sufficient.

---

## 9. Known Limitations and Future Work

| Item | Status | Notes |
|------|--------|-------|
| `kRunOnPlayerSpeed` | ⬜ defined, unused | Speed value for a cat running on top of another cat; not triggered anywhere in `PlayerCat::PhysicsUpdate()` |
| `BodyType::PATROL_ENEMY` … `BULLET` | ⬜ enum entries only | No concrete class; `PhysicsWorld` skips unknown types silently |
| `RopeConstraint` | ⬜ data structure only | `StepRopes()` not implemented |
| `LevelExitScene` | ⬜ empty stub | Not yet routed in `SceneManager`; `LevelOneScene` ESC currently returns `SceneId::LevelSelect` |
| Levels 2–10 | ⬜ not implemented | `LevelSelectScene::m_LevelSceneIds[1..9]` are `SceneId::None`; selecting them does nothing |
| SE (Sound Effects) | ⬜ volume setting exists | `OptionMenuScene` stores `seVolume` but no SE system is connected |
| `CooperativePushPower` | ⬜ computed, not consumed | `LocalPlayGameScene` updates this value in `GameContext` but no system reads it for gameplay purposes yet |
