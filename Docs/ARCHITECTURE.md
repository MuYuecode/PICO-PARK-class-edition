# Architecture Overview

## Runtime Backbone

- `main.cpp` drives `App` through `START -> UPDATE -> END`.
- `App::Update()` order is fixed: BGM tick -> active scene update/transition -> quit gate -> renderer tick.
- Scene switching is intent-based: scenes publish `SceneOp`, `SceneManager` executes exactly one op after scene update.

## Composition Root (`App::Start`)

- Creates process-lifetime services: `GlobalActors`, `SessionState`, `BGMPlayer`/`AudioService`, `VisualThemeService`, `SceneManager`.
- Builds shared visuals once: background, floor, header, door, startup cats.
- Registers all shipped scenes, then enters `SceneId::Title`.
- Current level registration is active for `Level01`, `Level02`, and `Level03`.

## Scene and Stack Model

- `SceneManager` owns scene instances (`unique_ptr`) and a stack of `SceneId`.
- Stack forms either a base scene or base + overlay (`LevelExitScene`).
- Overlay protocol: `PushOverlay` pauses base, `PopOverlay` resumes base, `RestartUnderlying` recreates base scene state, `ClearToAndGoTo` resets stack.

## Ownership Boundaries

- `App` owns long-lived systems; each scene owns only local runtime objects (UI, actors, local `PhysicsWorld`).
- Shared cross-scene access is interface-only via `SceneServices` (`IAudioService`, `IVisualThemeService`, `ISessionState`, `IGlobalActors`).
- Persistence is centralized in `SaveManager`:
  - `settings.json`: option values + keyboard mappings
  - `save_data.json`: best time table per level (`2P`~`8P`)

## Active Gameplay Topology

- Primary route: `Title -> Menu -> LocalPlay -> LocalPlayGame -> LevelSelect -> (Level01 | Level02 | Level03)`.
- `LevelSelectScene` keeps 10 slots; only mapped slots are enterable (`SceneId != None`).
- `LevelOneScene`, `LevelTwoScene`, `LevelThreeScene` share the same pause overlay contract (`ESC` -> `LevelExitScene`).
