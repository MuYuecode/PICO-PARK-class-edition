# Architecture Overview

This project is a scene-oriented game runtime with interface-injected services and scene-local gameplay systems.

## Runtime State Machine

- `main.cpp` drives `App` states: `START -> UPDATE -> END`.
- `App::Update()` runs in this order:
  1. `IAudioService::UpdateBgm()`
  2. `SceneManager::UpdateCurrent()`
  3. quit gate via `ISessionState::ShouldQuit()`
  4. `m_Root.Update()` (render tree update)

## Composition Root (`App::Start`)

- Creates long-lived systems: `GlobalActors`, `SessionState`, `AudioService`/`BGMPlayer`, `VisualThemeService`, `SceneManager`.
- Builds shared actors (background, floor, header, door, startup cats) once and stores them in `GlobalActors`.
- Registers all scenes into `SceneManager` and enters `SceneId::Title`.

## Scene Contract

- Base class `Scene` exposes `OnEnter()`, `OnExit()`, `Update()` (`void`).
- Scene transitions are **single-channel** via `SceneOp` only:
  - `PushOverlay`, `PopOverlay`, `RestartUnderlying`, `ClearToAndGoTo`.
- Scene requests transitions through `RequestSceneOp(...)`; `SceneManager` executes them after `Update()` via `ConsumeSceneOp()`.

## Scene Stack Model

- `SceneManager` owns `std::unique_ptr<Scene>` registry and a stack of `SceneId`.
- Base scene + optional overlay is the supported runtime shape.
- Overlay operations:
  - `PushOverlay`: pause base scene, enter overlay.
  - `PopOverlay`: exit overlay, resume base scene.
  - `RestartUnderlying`: exit overlay, re-enter base scene.
  - `ClearToAndGoTo`: clear current stack and enter target.

## Data and Ownership Boundaries

- `App` owns global lifetime objects.
- Scenes own scene-local state (UI nodes, temporary gameplay entities, `PhysicsWorld`).
- Shared visual actors are accessed through `IGlobalActors` and not re-owned by scenes.
- Cross-scene runtime data lives in `ISessionState`; persistence uses `SaveManager`.

## Registered Scene Set

- `TitleScene`, `MenuScene`, `ExitConfirmScene`, `OptionMenuScene`, `KeyboardConfigScene`, `LocalPlayScene`, `LocalPlayGameScene`, `LevelSelectScene`, `LevelExitScene` (overlay), `LevelOneScene`.
- `LevelOneScene` is registered under `SceneId::Level01`.
