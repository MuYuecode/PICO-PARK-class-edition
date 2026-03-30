# Physics System Design

This document summarizes the current `PhysicsWorld` implementation and usage.

## Core Types

```text
IPhysicsBody
├─ PlayerCat
├─ PushableBox
└─ StaticBody

IPushable
└─ PushableBox
```

`PhysicsWorld` is scene-local and provides:

- body registration (`Register`, `Unregister`, `Clear`)
- static boundary creation (`AddStaticBoundary`)
- optional rope metadata (`AddRope`, `RemoveRope`, `GetRopesOf`)
- frame stepping (`Update`)
- cooperative push query (`CountCharactersPushing`)

## Body Contracts

- `PhysicsUpdate()`: compute desired movement (`GetDesiredDelta`).
- `ApplyResolvedDelta()`: apply resolved world displacement.
- `OnCollision()`: receive collision normal callbacks after resolve.
- `PostUpdate()`: final per-frame hook.
- `IsActive` / `IsFrozen`: runtime participation switches.

## Per-Frame Pipeline

`PhysicsWorld::Update()` runs:

1. purge expired weak references periodically
2. `StepPhysicsUpdate()`
   - pass 1: active, non-frozen non-box bodies
   - pass 2: active, non-frozen `PUSHABLE_BOX` bodies
3. `StepResolveAndApply()`
   - snapshot active bodies
   - detect support/riding relations
   - resolve in dependency order
   - apply resolved deltas
   - dispatch collision callbacks
4. run `PostUpdate()` on active, non-frozen bodies

This ordering ensures pushable boxes read current-frame character intent.

## Collision and Riding Model

- Axis-separated resolve (horizontal, then vertical).
- Separation uses minimal displacement against solid overlap.
- Riding allows inheriting support-body horizontal movement.
- Kinematic and frozen bodies are treated as pre-resolved for ordering.

## Cooperative Push Logic

`CountCharactersPushing(target, dir)` recursively traverses adjacent chains:

- counts active characters pushing toward `dir`
- allows passive intermediaries to transmit force
- allows boxes to relay force through chains
- supports opposite-direction cancellation through per-direction counting in box logic

## Scene Integration Pattern

Typical usage in a physics scene:

- `OnEnter()`: clear world, register dynamic bodies, add static boundaries
- `Update()`: write input intent, call `m_World.Update()` once
- `OnExit()`: clear world

Current users: `TitleScene`, `MenuScene`, `LocalPlayGameScene`, `LevelOneScene`.

## LevelOne Integration Notes

`LevelOneScene` aligns collision bounds with rendered geometry:

- floor collider top is derived from floor sprite top edge
- ceiling collider bottom is derived from ceiling sprite bottom edge
- box spawn Y uses `PushableBox::GetHalfSize().y` (no hardcoded box height)

This keeps visual sprites and physical boundaries synchronized.

## Known Limits

- Rope constraints are stored but not solved yet.
- Several `BodyType` values are placeholders for future entities.
- Physics remains independent from `SceneServices`; scene-layer systems handle save/theme/audio/session concerns.
