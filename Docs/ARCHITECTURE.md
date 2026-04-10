# Architecture Overview

## Runtime Loop

- `main.cpp` runs `App` as a small state machine: `START -> UPDATE -> END`.
- `App::Update()` frame order is fixed: `AudioService::UpdateBgm()` -> `SceneManager::UpdateCurrent()` -> quit gate via `SessionState` -> `Renderer::Update()`.
- Scene switching is intent-based: scenes emit one `SceneOp`, `SceneManager` consumes at most one op after scene update.

## Composition Root (`App::Start`)

- Creates process-lifetime objects: `GlobalActors`, `SessionState`, `BGMPlayer` + `AudioService`, `VisualThemeService`, `SceneManager`.
- Builds shared actors once (background, floor, header, door, startup cats) and stores them in `GlobalActors`.
- Registers scenes: `Title`, `Menu`, `ExitConfirm`, `OptionMenu`, `KeyboardConfig`, `LocalPlay`, `LocalPlayGame`, `LevelSelect`, `LevelExit`, `Level01`, `Level02`, `Level03`.
- `LevelSelectScene` has 10 slots, but only slot 1~3 are mapped to `Level01`~`Level03`.

## Scene Stack Model

- `SceneManager` owns all scenes (`unique_ptr`) and a stack of `SceneId`.
- `GoTo`/`ClearToAndGoTo` fully exits current stack, then enters target scene.
- `PushOverlay` pauses underlying scene via `PauseGameplay()`, then enters overlay.
- `PopOverlay` exits overlay and resumes underlying scene via `ResumeGameplay()`.
- `RestartUnderlying` exits overlay, re-runs underlying `OnExit()` + `OnEnter()`, then calls `ResumeGameplay()`.

## Ownership and Boundaries

- `App` owns long-lived systems; each scene owns only scene-local runtime objects and scene-local `PhysicsWorld`.
- Cross-scene access is interface-first through `SceneServices` (`IAudioService`, `IVisualThemeService`, `ISessionState`, `IGlobalActors`).
- Persistence is centralized in `SaveManager`:
  - `settings.json`: option settings + keyboard configs
  - `save_data.json`: per-level best times (2P~8P)

## Implemented Gameplay Route

- Main path: `Title -> Menu -> LocalPlay -> LocalPlayGame -> LevelSelect -> (Level01 | Level02 | Level03)`.
- `ESC` in level scenes opens `LevelExit` overlay (`PushOverlay`).
- Level completion updates best time via `SaveManager::UpdateBestTime(...)`, then returns to `LevelSelect`.
- `SceneId::Level04`~`SceneId::Level10` are reserved enum IDs (not registered in `App::Start`).
