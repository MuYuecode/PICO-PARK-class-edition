# Physics Design

## Scope

Physics is scene-local. Each physics-enabled scene owns its own `PhysicsWorld`.

Active body implementations:

- `PlayerCat`
- `PushableBox`
- `StaticBody`
- `BulletBody`
- `LevelTwoScene::MovingPlankBody`
- `LevelThreeScene::MovingLiftBody`
- `LevelThreeScene::PatrolMobBody`
- `LevelThreeScene::PipeMobBody`

`PushableBox` is the only `IPushable` implementation.

## Body Types

`PhysicsBodyTraits::type` currently uses:

- `CHARACTER`: player cats.
- `PUSHABLE_BOX`: cooperative push boxes.
- `PATROL_ENEMY`: LevelThree mobs.
- `MOVING_PLATFORM`: moving planks/lifts.
- `CONDITIONAL_PLATFORM`: reserved.
- `ROPE_ENDPOINT`: reserved.
- `BULLET`: LevelFour bullet body.
- `JAR`: LevelFour jar static collider.
- `STATIC_BOUNDARY`: floors, walls, ceilings, shooter collider, and generic static geometry.

The `supportsPush` and `emitsCollisionCallbacks` flags exist in `PhysicsBodyTraits`, but current resolution logic primarily branches on `type`, `IsSolid()`, and `IsKinematic()`.

## `IPhysicsBody` Contract

An `IPhysicsBody` combines:

- Transform: `GetPosition`, `SetPosition`, `GetHalfSize`.
- Material: `IsSolid`, `IsKinematic`.
- Motion: `PhysicsUpdate`, `GetDesiredDelta`, `ApplyResolvedDelta`.
- Collision callback: `OnCollision(CollisionInfo)`.
- Push reaction: `GetMoveDir`, optional `NotifyPush`.
- Lifecycle: `IsActive`, `SetActive`, `IsFrozen`, `Freeze`, `Unfreeze`, `PostUpdate`.

## `PhysicsWorld` Responsibilities

- Register and unregister dynamic bodies.
- Own static colliders created by `AddStaticBoundary(...)`.
- Clear all bodies and rope metadata on scene exit/reset.
- Run the physics frame pipeline exactly once per scene update.
- Freeze and unfreeze all registered bodies for pause overlays.
- Provide push-query services to `PushableBox`.

Rope metadata exists (`AddRope`, `RemoveRope`, `GetRopesOf`) but rope force solving is not implemented.

## Frame Pipeline

1. `PhysicsWorld::Update()` optionally purges expired weak body references.
2. `PhysicsUpdateScheduler::StepPhysicsUpdate(...)` calls `PhysicsUpdate()` in two passes:
   - active, non-frozen, non-`PUSHABLE_BOX`
   - active, non-frozen `PUSHABLE_BOX`
3. `CollisionSolver::ResolveAndApply(...)`:
   - snapshots active bodies
   - detects support/riding relationships
   - marks kinematic/frozen bodies as resolved
   - resolves dynamic bodies against solid bodies
   - applies resolved deltas
   - emits `OnCollision(...)` callbacks
4. `PhysicsWorld` calls `PostUpdate()` on active, non-frozen bodies.

The two-pass update lets boxes query same-frame character push intent.

## Collision Rules

- Collision primitive is AABB using strict overlap (`<`), not edge-inclusive overlap.
- Resolution is horizontal pass followed by vertical pass.
- A body is resolved against other active bodies where the other body is solid.
- Non-solid bodies can still receive collision callbacks when their own movement resolves against solid bodies.
- Kinematic bodies define reference geometry and are marked resolved early.
- Frozen bodies produce no movement and are also marked resolved early.
- Moving-platform vertical snap logic handles lift/platform endpoint contacts.

## Support Rules

- `SupportResolver` computes support relationships before movement resolution.
- Moving platforms are preferred in close support ties.
- A supported body inherits support X motion.
- Support Y carry is applied only when the support body is a `MOVING_PLATFORM`.

## Cooperative Push Logic

- `PushableBox::PhysicsUpdate()` queries `IPushQueryService`.
- `PushForceResolver` recursively counts active characters pushing through character/box chains.
- Box horizontal motion activates when `abs(netPushers) >= requiredPushers`.
- Box gravity is always applied.
- Adjacent active pushers receive `NotifyPush()` for push animation feedback.

## Bullet Logic

`BulletBody` is a physics body, not a scene-local AABB helper.

- `PhysicsUpdate()` moves the bullet left by speed and delta time.
- `IsSolid()` returns false, so the bullet does not block or push other bodies.
- `IsKinematic()` returns false, so the collision solver resolves the bullet against solid bodies and emits collision callbacks.
- `OnCollision(...)` records a `HitType`:
  - `Character`
  - `Jar`
  - `Solid`
  - `None`
- `LevelFourScene` consumes the recorded hit type after `m_World.Update()` and performs gameplay reactions:
  - character hit: deactivate bullet
  - jar hit: advance jar state, then deactivate bullet
  - solid hit: deactivate bullet

The scene still owns high-level gameplay state such as current jar phase, timer, key carrier, door state, and level-clear routing.

## Scene Integration Pattern

Physics-enabled scenes follow this pattern:

- `OnEnter`: clear world, reset state, create/register dynamic bodies, create static boundaries.
- `Update`: set input/movement intent, call `m_World.Update()` once, sync visuals, process gameplay state.
- `OnExit`: unfreeze if needed, clear world, remove scene-local visuals.

Physics-backed scenes currently include:

- `TitleScene`
- `MenuScene`
- `LocalPlayGameScene`
- `LevelOneScene`
- `LevelTwoScene`
- `LevelThreeScene`
- `LevelFourScene`

`LevelExitScene` pause/resume is implemented by scene overrides that call `PhysicsWorld::FreezeAll()` and `PhysicsWorld::UnfreezeAll()`.
