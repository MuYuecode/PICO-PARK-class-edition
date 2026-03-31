# Physics Design

## Scope

- `PhysicsWorld` is scene-local and deterministic per frame.
- Active body implementations: `PlayerCat`, `PushableBox`, `StaticBody`.
- `PushableBox` is the only current `IPushable` implementation.

## Body Contract

- `PhysicsUpdate()` computes intended movement state.
- `GetDesiredDelta()` exposes this frame's desired displacement.
- `ApplyResolvedDelta()` applies world-resolved displacement.
- `OnCollision(CollisionInfo)` receives contact normals after resolution.
- `PostUpdate()` finalizes animation/state after physics.
- `IsActive()` and `IsFrozen()` gate participation in simulation.

## World Responsibilities

- Lifecycle: `Register`, `Unregister`, `Clear`.
- Geometry ownership: `AddStaticBoundary()` allocates and owns `StaticBody` colliders.
- Stepping: `Update()` (including periodic weak-pointer purge).
- Pause support: `FreezeAll()` and `UnfreezeAll()`.
- Cooperative-force query: `CountCharactersPushing()` with recursive chain traversal.
- Rope APIs are metadata only (`AddRope`, `RemoveRope`, `GetRopesOf`); no solver step yet.

## Frame Pipeline

1. Purge expired weak references every `kPurgeInterval` frames.
2. `StepPhysicsUpdate()` in two passes:
   - non-box active, non-frozen bodies
   - `PUSHABLE_BOX` bodies
3. `StepResolveAndApply()`:
   - snapshot active bodies
   - detect support links (`DetectRiding`, tolerance-based)
   - resolve in support dependency order
   - apply resolved deltas
   - dispatch `OnCollision` callbacks
4. Run `PostUpdate()` on active, non-frozen bodies.

The two-pass update ensures box push evaluation sees same-frame player move intents.

## Collision and Support Rules

- Collision primitive: AABB overlap with strict `<` checks (edge-touch is not overlap).
- Resolution order: horizontal pass, then vertical pass.
- Riders inherit resolved horizontal movement from their support body.
- Kinematic/frozen bodies are marked resolved first and treated as reference geometry.
- Fallback path resolves any remaining unresolved body without support carry.

## Cooperative Push Mechanics

- `PushableBox::PhysicsUpdate()` computes `net = rightPushers - leftPushers` from world queries.
- Box moves only when `abs(net) >= requiredPushers`; otherwise only gravity applies.
- Recursive counting allows force transmission through chained characters/boxes.
- Box maintains an in-world deficit label: `max(0, requiredPushers - abs(net))`.
- Adjacent qualifying characters receive `NotifyPush()` to drive push animation state.

## Scene Integration Pattern

- Physics-backed scenes: `TitleScene`, `MenuScene`, `LocalPlayGameScene`, `LevelOneScene`.
- Typical lifecycle:
  - `OnEnter`: clear world, register dynamic bodies, build static room boundaries
  - `Update`: set movement intents, call `m_World.Update()` once
  - `OnExit`: clear world
- `LevelOneScene` maps overlay pause/resume directly to freeze/unfreeze world state.
