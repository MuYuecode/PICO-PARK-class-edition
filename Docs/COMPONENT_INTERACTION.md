# Component Interaction

## Per-Frame Execution Chain

1. `AudioService::UpdateBgm()` advances BGM track switching.
2. `SceneManager::UpdateCurrent()` runs scene logic and may execute one transition.
3. `SessionState::ShouldQuit()` can switch `App` to `END`.
4. `Renderer::Update()` processes the render tree.

## Scene Transition Handshake

1. Active scene executes `Update()`.
2. Scene returns intent via `ConsumeSceneOp()`.
3. `SceneManager` applies stack operation (`PushOverlay`, `PopOverlay`, `RestartUnderlying`, `ClearToAndGoTo`).

Scenes never mutate manager state directly; they only emit operation requests.

## Service and Data Flow

- `IGlobalActors`: shared visual nodes (`Root`, `Background`, `Floor`, `Door`, startup cats, test box).
- `ISessionState`: runtime gameplay data (player count, key configs, cooperative value, quit flag).
- `IAudioService` and `IVisualThemeService`: immediate preview paths used by option UI.
- `SaveManager`: persistent storage boundary for options, key profiles, and level best times.

## Key Interaction Scenarios

- `ExitConfirmScene`: YES calls `RequestQuit()`; `App` exits on the next quit gate.
- `OptionMenuScene`: applies pending theme/audio values immediately, commits only on OK, reverts on cancel/escape.
- `KeyboardConfigScene`: edits one player profile, blocks conflicting save for non-1P profiles, then syncs to session.
- `LocalPlayScene`: validates selected player count against configured key profiles before entering gameplay.
- `LocalPlayGameScene`: tracks per-player door entry, then routes to `LevelSelectScene` when all entered.

## `LevelOneScene` + `LevelExitScene` Overlay Cycle

- `LevelOneScene` update order: player input -> `PhysicsWorld::Update()` -> timer -> key pickup/follow -> door unlock -> per-player entry.
- Pressing ESC pushes `LevelExitScene` as overlay; base gameplay is frozen via `PauseGameplay()`.
- Overlay actions:
  - Return -> `PopOverlay` (resume)
  - Retry -> `RestartUnderlying` (re-enter same level)
  - Leave -> `ClearToAndGoTo(LevelSelect or Title)`
- On full clear, `LevelOneScene` saves best time and returns to `LevelSelectScene`.
