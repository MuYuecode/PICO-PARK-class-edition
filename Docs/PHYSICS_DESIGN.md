# Physics System Design

This document summarizes the current `PhysicsWorld` behavior.

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

- body lifecycle (`Register`, `Unregister`, `Clear`)
- static colliders (`AddStaticBoundary`)
- frame stepping (`Update`)
- pause control (`FreezeAll`, `UnfreezeAll`)
- cooperative push query (`CountCharactersPushing`)
- rope metadata storage (`AddRope`, `RemoveRope`, `GetRopesOf`)

## Body Contract

- `PhysicsUpdate()` computes intent (`GetDesiredDelta()`).
- `ApplyResolvedDelta()` applies resolved movement.
- `OnCollision()` receives contact normal callbacks.
- `PostUpdate()` handles final per-frame state.
- `IsActive` and `IsFrozen` gate participation.

## Per-Frame Pipeline

`PhysicsWorld::Update()` sequence:

1. periodic purge of expired weak references
2. `StepPhysicsUpdate()`
   - pass A: active, non-frozen, non-box bodies
   - pass B: active, non-frozen `PUSHABLE_BOX` bodies
3. `StepResolveAndApply()`
   - snapshot active bodies
   - detect riding/support links
   - resolve deltas in support-safe order
   - apply resolved deltas
   - dispatch `OnCollision()` callbacks
4. run `PostUpdate()` on active, non-frozen bodies

The two update passes ensure boxes read current-frame character push intent.

## Resolve Model

- AABB overlap test with axis-separated resolution (horizontal then vertical).
- Riding propagation adds support-body horizontal delta to rider effective motion.
- Kinematic and frozen bodies are marked resolved early in dependency ordering.

## Cooperative Push Model

`CountCharactersPushing(target, dir)` recursively traverses adjacent chains:

- counts active `CHARACTER` bodies pushing in `dir`
- allows passive intermediates to relay force
- allows box-to-box relay
- supports opposite-direction cancellation by separate left/right counts in `PushableBox`

## Scene Integration

Physics users: `TitleScene`, `MenuScene`, `LocalPlayGameScene`, `LevelOneScene`.

Typical pattern:

- `OnEnter()`: `Clear()`, register bodies, create static boundaries
- `Update()`: write move/jump intent, call `m_World.Update()` once
- `OnExit()`: `Clear()`

Pause integration:

- `LevelOneScene::PauseGameplay()` -> `FreezeAll()`
- `LevelOneScene::ResumeGameplay()` -> `UnfreezeAll()`

## Known Limits

- Rope constraints are data-only; no rope solver step is implemented.
- Several `BodyType` values are reserved for future entities.
