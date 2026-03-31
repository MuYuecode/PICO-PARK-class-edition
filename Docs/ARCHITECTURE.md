# Architecture Overview

## Runtime Backbone

- `main.cpp` drives `App` as a small state machine: `START -> UPDATE -> END`.
- `App::Update()` order is fixed: `AudioService::UpdateBgm()` -> `SceneManager::UpdateCurrent()` -> quit gate (`SessionState`) -> `Renderer::Update()`.
- `SceneManager` is the only transition executor; scenes only emit `SceneOp` intents.

## Composition Root (`App::Start`)

- Builds long-lived services: `GlobalActors`, `SessionState`, `BGMPlayer`/`AudioService`, `VisualThemeService`, `SceneManager`.
- Creates shared world actors once (background, floor, header, door, startup cats) and stores them in `GlobalActors`.
- Registers all scenes, maps `SceneId::Level01` to `LevelOneScene`, then enters `SceneId::Title`.

## Scene Stack Model

- Allowed stack shapes: one base scene, or base + one overlay.
- `PushOverlay` pauses base (`PauseGameplay`) and enters overlay.
- `PopOverlay` exits overlay and resumes base (`ResumeGameplay`).
- `RestartUnderlying` exits overlay, restarts base (`OnExit` -> `OnEnter`), then resumes it.
- `ClearToAndGoTo` clears overlay/base and enters target as new base.

## Ownership Boundaries

- `App` owns process-lifetime objects and scene registry lifetime.
- `SceneManager` owns scenes as `std::unique_ptr<Scene>`.
- Each scene owns its transient gameplay objects (`PhysicsWorld`, local UI, level entities).
- Scenes access shared visuals through `IGlobalActors` (non-owning) and shared mutable session data through `ISessionState`.
- File persistence is centralized in `SaveManager`:
  - `settings.json` for option + key config data
  - `save_data.json` for level best-time data

## Current Gameplay Topology

- Core route: `Title -> Menu -> LocalPlay -> LocalPlayGame -> LevelSelect -> Level01`.
- `LevelOneScene` uses an overlay pause menu (`LevelExitScene`) via `PushOverlay`/`PopOverlay`/`RestartUnderlying`.
- Only Level 1 is currently wired from level select; `Level02` to `Level10` ids exist but are not registered yet.
