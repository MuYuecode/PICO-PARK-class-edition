# Architecture Overview

The project is a scene-driven game loop with interface-based service injection.

## Runtime Loop

- `main.cpp` runs `App` state machine: `START -> UPDATE -> END`.
- `App::Update()` executes in fixed order:
  1. `IAudioService::UpdateBgm()`
  2. `SceneManager::UpdateCurrent()`
  3. check `ISessionState::ShouldQuit()`
  4. `Renderer::Update()`

## Composition Root

`App::Start()` creates long-lived objects and wiring:

- `GlobalActors` (shared render objects)
- `SessionState`
- `AudioService` + `BGMPlayer`
- `VisualThemeService`
- `SceneManager`

Each `Scene` receives only interface references via `SceneServices`:

- `IAudioService&`
- `IVisualThemeService&`
- `ISessionState&`
- `IGlobalActors&`

## Scene Model

- `SceneManager` owns scene instances and the runtime scene stack.
- Normal transitions use `SceneId` return values from `Scene::Update()`.
- Overlay transitions use `SceneOp` (`PushOverlay`, `PopOverlay`, `RestartUnderlying`, `ClearToAndGoTo`).
- In `UpdateCurrent()`, pending `SceneOp` is processed before normal `SceneId` transition.

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
- `LevelExitScene` (overlay)
- `LevelOneScene` (`SceneId::Level01`)

## Ownership Boundaries

- `App` owns global lifetime objects.
- `SceneManager` owns `unique_ptr<Scene>` registry and active stack ids.
- Scenes own scene-local data (`PhysicsWorld`, temporary UI, per-level entities).
- `IGlobalActors` exposes shared visual actors; scenes configure visibility/position/image but do not re-own them.

## Engineering Rules

1. Keep gameplay/UI logic inside scene-local classes.
2. Keep cross-scene state in `ISessionState` or persisted via `SaveManager`.
3. Keep scene code depending on interfaces (`I*`) rather than concrete services.
4. Register every new scene in `App::Start()` and route by `SceneId`/`SceneOp` only.
