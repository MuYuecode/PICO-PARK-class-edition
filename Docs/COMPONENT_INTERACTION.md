# Component Interaction

## Frame Execution Chain

1. `AudioService::UpdateBgm()` advances music state.
2. `SceneManager::UpdateCurrent()` updates current top scene.
3. Top scene may emit one `SceneOp`; manager executes at most one stack mutation.
4. `App` checks `SessionState::ShouldQuit()`.
5. `Renderer` updates and draws the current actor graph.

## Scene Transition Contract

1. Scene performs local logic in `Update()`.
2. Scene requests transition via `RequestSceneOp(...)`.
3. `SceneManager` consumes via `ConsumeSceneOp()` and executes:
   - `PushOverlay`
   - `PopOverlay`
   - `RestartUnderlying`
   - `ClearToAndGoTo`

Scenes do not directly call other scenes or mutate scene stacks.

## Shared Service Flows

- `IGlobalActors`: shared root and global visuals (`Background`, `Floor`, `Header`, `Door`, startup cats; optional `TestBox`).
- `ISessionState`: selected player count, cooperative push power, applied key configs, quit flag.
- `IAudioService`: BGM playback, volume, per-frame update.
- `IVisualThemeService`: background theme apply/restore with bounded index.
- `SaveManager`: option settings, keyboard mappings, level best times.

## Key Runtime Scenarios

- `ExitConfirmScene`: `YES` sets quit flag (`RequestQuit`), app exits on frame gate.
- `OptionMenuScene`: pending values preview live (theme/audio); `OK` commits + saves, `Cancel/ESC` reverts to applied values.
- `KeyboardConfigScene`: edits per-player bindings, enforces conflict rule for non-1P, syncs to both save file and session state.
- `LocalPlayScene`: blocks start when selected player count exceeds configured keyboard profiles.
- `LocalPlayGameScene`: all active players must enter open door with their `up` key before routing to `LevelSelect`.
- `LevelSelectScene`: enters only mapped slots (`SceneId != None`), displays per-player-count best time and crowns.

## Level and Overlay Coordination

- `LevelOneScene`, `LevelTwoScene`, `LevelThreeScene` open `LevelExitScene` overlay on `ESC`.
- Pause/resume is delegated to levels (`PauseGameplay`/`ResumeGameplay`) and implemented by `PhysicsWorld::FreezeAll()`/`UnfreezeAll()`.
- Overlay actions:
  - Return: `PopOverlay`
  - Retry: `RestartUnderlying`
  - Level Select / Title: `ClearToAndGoTo`
- On clear, each implemented level writes best time through `SaveManager::UpdateBestTime(...)` before returning to `LevelSelect`.
