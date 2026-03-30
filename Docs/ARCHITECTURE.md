# Architecture Overview

This project uses scene-driven flow with interface-based service injection.

## Runtime Loop

- `main.cpp` runs `App` until app state reaches `END`.
- `App` state machine: `START -> UPDATE -> END`.
- `App::Update()` order:
  1. `IAudioService::UpdateBgm()`
  2. `SceneManager::UpdateCurrent()`
  3. quit check via `ISessionState::ShouldQuit()`
  4. `Renderer::Update()`

## Composition Root

`App::Start()` creates and wires long-lived objects:

- `GlobalActors`
- `SessionState`
- `AudioService`
- `VisualThemeService`
- `SceneManager`

Scenes receive shared capabilities through `SceneServices`:

- `IAudioService&`
- `IVisualThemeService&`
- `ISessionState&`
- `IGlobalActors&`

## Scene and Transition Model

- All scenes derive from `Scene` and implement `OnEnter`, `Update`, `OnExit`.
- `SceneManager` is the only transition owner (`Register`, `GoTo`, `UpdateCurrent`).
- Transition rule: scene returns a `SceneId`; manager performs `OnExit` (old) then `OnEnter` (new).

## Ownership Boundaries

- `App` owns services, global actors, and `SceneManager`.
- `SceneManager` owns scene instances.
- Scenes own scene-local runtime state (UI nodes, local physics world, temporary gameplay data).
- Shared render actors are accessed via `IGlobalActors` and are not re-owned by scenes.

## Registered Scenes

Registered in `App::Start()`:

- `TitleScene`
- `MenuScene`
- `ExitConfirmScene`
- `OptionMenuScene`
- `KeyboardConfigScene`
- `LocalPlayScene`
- `LocalPlayGameScene`
- `LevelSelectScene`
- `LevelOneScene` (registered with `SceneId::Level01`)

`LevelExitScene` files exist but are currently empty.

## Extension Rules

1. Keep scene-specific behavior inside that scene.
2. Put cross-scene runtime state in `ISessionState`.
3. Use `IGlobalActors` only for shared long-lived render actors.
4. Extend `IAudioService` / `IVisualThemeService` for global behavior.
5. Register new scenes in `App::Start()` and route only by `SceneId`.
