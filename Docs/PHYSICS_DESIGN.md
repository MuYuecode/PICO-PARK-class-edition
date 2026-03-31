# Physics Design

## Scope

`PhysicsWorld` is scene-local and coordinates body update, collision resolution, and callback dispatch.

```text
IPhysicsBody: PlayerCat, PushableBox, StaticBody
IPushable:    PushableBox
```

## Body Interface Expectations

- `PhysicsUpdate()` computes intent.
- `GetDesiredDelta()` exposes intended movement for this frame.
- `ApplyResolvedDelta()` applies world-resolved displacement.
- `OnCollision()` receives contact normals.
- `PostUpdate()` runs after resolution.
- `IsActive` and `IsFrozen` gate participation.

## World Responsibilities

- body lifecycle: `Register`, `Unregister`, `Clear`
- static collider creation: `AddStaticBoundary` (owned as `StaticBody`)
- stepping: `Update`
- pause control: `FreezeAll`, `UnfreezeAll`
- cooperative push query: `CountCharactersPushing`
- rope metadata storage: `AddRope`, `RemoveRope`, `GetRopesOf`

## Frame Pipeline

`PhysicsWorld::Update()` runs:

1. periodic weak-reference purge
2. `StepPhysicsUpdate()` in two passes
   - non-`PUSHABLE_BOX` first
   - `PUSHABLE_BOX` second
3. `StepResolveAndApply()`
   - snapshot active bodies
   - detect rider/support links
   - resolve desired deltas with support ordering
   - apply resolved deltas
   - dispatch collision callbacks
4. `PostUpdate()` for active, non-frozen bodies

The two-pass update ensures pushable boxes read current-frame character push intent.

## Collision and Support Resolution

- AABB overlap with axis-separated resolution (horizontal, then vertical).
- Bodies riding supports inherit support horizontal movement.
- Kinematic/frozen bodies are marked resolved early.
- Unresolved leftovers are force-resolved as a safety fallback.

## Cooperative Push Mechanics

- `CountCharactersPushing(target, dir)` recursively traverses adjacent push chains.
- Counts active `CHARACTER` bodies with matching push direction.
- Allows passive intermediates and box relays.
- `PushableBox` computes right/left counts separately, then applies net force threshold (`requiredPushers`).

## Scene Integration Pattern

Main physics scenes: `TitleScene`, `MenuScene`, `LocalPlayGameScene`, `LevelOneScene`.

- `OnEnter()`: register dynamic bodies + static boundaries.
- `Update()`: set input intent and call `m_World.Update()` once.
- `OnExit()`: `m_World.Clear()`.

`LevelOneScene` pause hooks use `FreezeAll()` and `UnfreezeAll()` for overlay pause/resume.

## Current Limits

- Rope constraints are stored as data only; no rope solver pass is implemented.
- Several `BodyType` entries are reserved for future gameplay entities.
