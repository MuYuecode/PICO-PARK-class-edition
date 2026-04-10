# Physics Design

## Scope

- Physics is scene-local (`PhysicsWorld` owned by each physics-enabled scene).
- Active body implementations:
  - `PlayerCat`
  - `PushableBox`
  - `StaticBody`
  - `LevelTwoScene::MovingPlankBody`
  - `LevelThreeScene::MovingLiftBody`
  - `LevelThreeScene::PatrolMobBody`
  - `LevelThreeScene::PipeMobBody`
- `PushableBox` is the only `IPushable` implementation.

## `IPhysicsBody` Contract

- Transform: `GetPosition/SetPosition/GetHalfSize`.
- Material: `IsSolid`, `IsKinematic`.
- Motion: `PhysicsUpdate`, `GetDesiredDelta`, `ApplyResolvedDelta`.
- Collision callback: `OnCollision(CollisionInfo)`.
- Push reaction: `GetMoveDir`, optional `NotifyPush`.
- Lifecycle: `IsActive/SetActive`, `IsFrozen/Freeze/Unfreeze`, `PostUpdate`.

## `PhysicsWorld` Responsibilities

- Body lifecycle: `Register`, `Unregister`, `Clear`.
- Static ownership: `AddStaticBoundary` creates world-owned `StaticBody` colliders.
- Pause control: `FreezeAll` / `UnfreezeAll`.
- Push query service: `CountCharactersPushing`, `GetBodiesOfType`.
- Rope metadata exists (`AddRope`, `RemoveRope`, `GetRopesOf`) but rope force solving is not implemented.

## Frame Pipeline

1. Optional weak reference purge every `kPurgeInterval` frames.
2. `PhysicsUpdateScheduler::StepPhysicsUpdate(...)` in two passes:
   - active, non-frozen, non-`PUSHABLE_BOX`
   - active, non-frozen `PUSHABLE_BOX`
3. `CollisionSolver::ResolveAndApply(...)`:
   - snapshot active bodies and desired deltas
   - detect supports (`SupportResolver::DetectRiding`)
   - resolve in support-aware order
   - apply deltas
   - emit collision callbacks
4. `PostUpdate()` for active, non-frozen bodies.

Two-pass update ensures box push queries see same-frame character intent.

## Collision and Support Rules

- Broad primitive is strict AABB overlap (`<`), not edge-inclusive.
- Resolution is horizontal pass then vertical pass.
- Supports are computed pre-move; moving platforms are preferred in close ties.
- Supported body inherits support X motion; support Y carry is applied only when support is `MOVING_PLATFORM`.
- Kinematic/frozen bodies are marked resolved early and act as reference geometry.
- Additional moving-platform vertical snap logic reduces missed contacts around platform endpoints.

## Cooperative Push Logic

- `PushableBox` computes `net = pushRight - pushLeft` from world queries.
- Horizontal motion activates only when `abs(net) >= requiredPushers`; gravity is always applied.
- Push counting is recursive through character/box chains (`PushForceResolver`).
- Box text shows remaining deficit: `max(0, requiredPushers - abs(net))`.
- Adjacent active pushers receive `NotifyPush()` for push animation feedback.

## Scene Integration Pattern

- Physics-backed scenes: `TitleScene`, `MenuScene`, `LocalPlayGameScene`, `LevelOneScene`, `LevelTwoScene`, `LevelThreeScene`.
- Common lifecycle:
  - `OnEnter`: clear world, register dynamic bodies, add static boundaries
  - `Update`: set movement intent, call `m_World.Update()` exactly once
  - `OnExit`: unfreeze if needed, then clear world
- Level pause overlay (`LevelExitScene`) maps to freeze/unfreeze via scene `PauseGameplay`/`ResumeGameplay`.
