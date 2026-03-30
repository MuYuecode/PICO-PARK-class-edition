# Component Interaction and Runtime Flow

This document describes how runtime systems collaborate each frame.

## Frame Pipeline

Per frame in `App::Update()`:

1. `IAudioService::UpdateBgm()`
2. `SceneManager::UpdateCurrent()`
3. if `ISessionState::ShouldQuit()` -> `App::State = END`
4. `Renderer::Update()`

`SceneManager::UpdateCurrent()` calls current `Update()` and applies scene switch via `GoTo(next)` when needed.

## Scene Lifecycle Pattern

- `OnEnter()`: attach required nodes, reset scene-local state, register local physics bodies.
- `Update()`: process input, run local logic, optionally return next `SceneId`.
- `OnExit()`: detach scene-owned nodes and clear temporary state.

Scenes usually reconfigure shared actors from `IGlobalActors` (show/hide/move/image), not recreate them.

## Shared-Service Interaction Rules

- Audio: scenes call `IAudioService`; direct `BGMPlayer` usage is app-level.
- Theme: scenes call `IVisualThemeService`.
- Session: scenes use `ISessionState` for player count, key mappings, cooperative values, and quit request.
- Global actors: scenes use `IGlobalActors` for root and shared actors (`Background`, `Header`, `Floor`, `Door`, `StartupCats`, optional `TestBox`).

## Key Runtime Flows

- Exit: `ExitConfirmScene` calls `m_Session.RequestQuit()`, then `App` moves to `END`.
- Option: `OptionMenuScene` edits pending values, previews through services, commits to `SaveManager`.
- Keyboard config: `KeyboardConfigScene` captures/validates keys, persists to `SaveManager`, syncs to `ISessionState`.
- Local play gate: `LocalPlayScene` checks configured players (via `KeyboardConfigScene*`) before entering gameplay.
- Level select: `LevelSelectScene` loads best-time data and routes by configured `SceneId` map.

## LevelOneScene Interaction (Current)

`LevelOneScene` per-frame sequence:

1. read player input and set move intent
2. call `m_World.Update()` once
3. update timer text
4. key pickup check (`TryPickKey`)
5. key follow update (`UpdateKeyFollow`)
6. door-open check when key carrier touches door (`TryOpenDoorAndClear`)
7. per-player door entry check (requires each player's own `up` key)
8. when all players entered, write best time and return `SceneId::LevelSelect`

Box thresholds are set on enter from selected players:

- `BoxA = playerCount - 1`
- `BoxB = playerCount`

## Physics Scene Interaction

`TitleScene`, `MenuScene`, `LocalPlayGameScene`, and `LevelOneScene` follow a shared physics pattern:

1. write intent (`SetMoveDir`, `Jump`)
2. call `PhysicsWorld::Update()` exactly once
3. run scene-specific post-physics logic

`PhysicsWorld` handles update ordering, resolve/apply, collision callbacks, and `PostUpdate()` dispatch.
