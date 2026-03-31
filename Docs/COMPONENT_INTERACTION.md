# Component Interaction

## Per-Frame Call Chain

1. `AudioService::UpdateBgm()` advances track playback state.
2. `SceneManager::UpdateCurrent()` runs top scene logic.
3. `SceneManager` consumes at most one `SceneOp` and mutates stack.
4. `SessionState::ShouldQuit()` gates `App` transition to `END`.
5. `Renderer` updates and draws the current actor tree.

## Scene Transition Contract

1. Scene executes `Update()`.
2. Scene emits intent via `RequestSceneOp(...)`.
3. Manager reads `ConsumeSceneOp()` and executes one operation:
   - `PushOverlay`
   - `PopOverlay`
   - `RestartUnderlying`
   - `ClearToAndGoTo`

Scenes never call each other or mutate stack state directly.

## Shared Service Flows

- `IGlobalActors`: shared visual anchors and startup actors (`Root`, background/floor/header/door, startup cats).
- `ISessionState`: selected player count, cooperative push power, key configs, quit signal.
- `IAudioService` and `IVisualThemeService`: live preview path used by options UI.
- `SaveManager`: persistent path for options, key configs, and per-level best times.

## Key Runtime Scenarios

- `ExitConfirmScene`: confirm YES sets quit flag; app exits on next frame gate.
- `OptionMenuScene`: pending values are previewed instantly; `OK` commits, `Cancel/ESC` restores applied values.
- `KeyboardConfigScene`: updates per-player bindings, enforces conflict rule for non-1P, syncs to save + session.
- `LocalPlayScene`: blocks start when selected player count exceeds configured key profiles.
- `LocalPlayGameScene`: transitions to `LevelSelectScene` only after all active players enter the open door.
- `LevelSelectScene`: enters only mapped level slots (`SceneId != None`).

## Level and Overlay Coordination

- `LevelOneScene`, `LevelTwoScene`, and `LevelThreeScene` open `LevelExitScene` overlay on `ESC`.
- Gameplay pause/resume is delegated to level scenes through `PauseGameplay()` / `ResumeGameplay()` (`PhysicsWorld::FreezeAll()` / `UnfreezeAll()`).
- Overlay outcomes:
  - Return -> `PopOverlay`
  - Retry -> `RestartUnderlying`
  - Level Select / Title -> `ClearToAndGoTo`
- On clear, level scenes update best time via `SaveManager` before routing back to `LevelSelectScene`.
