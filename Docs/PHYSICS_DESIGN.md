# Physics Design

## Scope

- `PhysicsWorld` is scene-local and stepped once per frame.
- Active body implementations: `PlayerCat`, `PushableBox`, `StaticBody`, `LevelTwoScene::MovingPlankBody`, `LevelThreeScene::MovingLiftBody`, `LevelThreeScene::PatrolMobBody`, `LevelThreeScene::PipeMobBody`.
- `PushableBox` is the only `IPushable` type.

## `IPhysicsBody` Contract

- `PhysicsUpdate()` computes frame intent.
- `GetDesiredDelta()` provides intended displacement.
- `ApplyResolvedDelta()` applies solved displacement.
- `OnCollision(CollisionInfo)` receives per-axis contact normals.
- `PostUpdate()` handles post-physics state (for example animation).
- `IsActive()` and `IsFrozen()` gate solver participation.

## PhysicsWorld Responsibilities

- Lifecycle: `Register`, `Unregister`, `Clear`.
- Static ownership: `AddStaticBoundary()` creates world-owned `StaticBody` colliders.
- Pause control: `FreezeAll()` / `UnfreezeAll()`.
- Push-force query: `CountCharactersPushing()` with recursive chain traversal.
- Rope metadata APIs exist (`AddRope`, `RemoveRope`, `GetRopesOf`), but no rope force solve step yet.

## Frame Pipeline

1. Periodically purge expired weak references (`kPurgeInterval = 60`).
2. `StepPhysicsUpdate()` in two passes:
   - active non-frozen non-box bodies
   - active non-frozen `PUSHABLE_BOX` bodies
3. `StepResolveAndApply()`:
   - snapshot active bodies
   - detect support links (`DetectRiding`)
   - resolve by support dependency order
   - apply resolved deltas
   - dispatch collision callbacks
4. Run `PostUpdate()` on active, non-frozen bodies.

Two-pass update ensures `PushableBox` sees same-frame character move intent.

## Collision and Support Rules

- Primitive is strict AABB overlap (`<`), so edge-touch alone is non-overlap.
- Solver order is horizontal pass then vertical pass.
- Riding support is detected before movement and prefers moving platforms in near-tie cases.
- Supported bodies inherit support X movement; Y carry is applied for moving-platform supports.
- Kinematic/frozen bodies are marked resolved early and act as reference geometry.
- Moving-platform vertical snap uses a continuous relative check to reduce missed contacts near endpoints.

## Cooperative Push and Box Behavior

- `PushableBox` computes `net = pushRight - pushLeft` from world queries.
- Box moves horizontally only when `abs(net) >= requiredPushers`; gravity always applies.
- Recursive push counting propagates force through character/box chains.
- Box count text displays deficit: `max(0, requiredPushers - abs(net))`.
- Adjacent active pushers get `NotifyPush()` for push animation feedback.

## Scene Integration Pattern

- Physics-backed scenes: `TitleScene`, `MenuScene`, `LocalPlayGameScene`, `LevelOneScene`, `LevelTwoScene`, `LevelThreeScene`.
- Common lifecycle:
  - `OnEnter`: clear world, register dynamics, add static boundaries
  - `Update`: write input intent, call `m_World.Update()` once
  - `OnExit`: unfreeze (if needed) then clear world
- Level scenes map pause overlay to freeze/unfreeze through `PauseGameplay()` and `ResumeGameplay()`.
