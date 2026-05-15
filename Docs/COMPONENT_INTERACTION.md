# Component Interaction

## Frame Execution Chain

1. `AudioService::UpdateBgm()` advances music state.
2. `SceneManager::UpdateCurrent()` updates the top scene.
3. The top scene may emit one `SceneOp`.
4. `SceneManager` applies at most one stack mutation.
5. `App` checks `SessionState::ShouldQuit()`.
6. `Renderer` draws the current actor graph.

## Scene Transition Contract

Scenes do not call each other directly. A scene requests navigation through `RequestSceneOp(...)`.

Supported operations:

- `PushOverlay`
- `PopOverlay`
- `RestartUnderlying`
- `ClearToAndGoTo`

`SceneManager` is the only component that mutates the scene stack.

## Shared Services

- `IGlobalActors`: shared root and shared visuals, including background, floor, header, door, startup cats, and optional test box.
- `ISessionState`: selected player count, cooperative push power, applied keyboard configs, and quit flag.
- `IAudioService`: BGM playback, BGM update, and volume control.
- `IVisualThemeService`: background theme selection and restore.
- `SaveManager`: settings, keyboard mappings, and level best-time persistence.

## Input and Configuration Flow

- `KeyboardConfigScene` edits per-player key bindings and writes them to `SaveManager`.
- `SessionState` keeps the currently applied key configs for gameplay scenes.
- `LocalPlayScene` checks whether the selected player count has enough configured keyboard profiles.
- Level scenes read `SessionState::GetSelectedPlayerCount()` and `GetAppliedKeyConfigs()` on entry.

## Local Play Flow

`LocalPlayGameScene` is the free-play cooperative room:

- It spawns selected players.
- It uses the global door actor.
- Players must enter the open door with their own `up` key.
- Completion routes to `LevelSelect`.

## Level Flow

All implemented level scenes follow the same broad lifecycle:

- `OnEnter`: hide or reposition shared actors, create local visuals, clear physics, register bodies, and reset state.
- `Update`: handle input intent, run the local physics world, update gameplay state, update timer text, and request transitions.
- `OnExit`: remove local actors, clear physics, restore shared actors as needed.

Level-specific behavior:

- `LevelOneScene`: cooperative box pushing and door clear.
- `LevelTwoScene`: button activation, moving planks, key pickup, door open, and all-player entry.
- `LevelThreeScene`: consensus controls, moving lifts, mobs, checkpoint, key pickup, door open, and clear.
- `LevelFourScene`: shooter spawns `BulletBody`, bullet collisions are reported by the physics model, Jar progresses through three states, then key pickup and door clear become available.

## Overlay Coordination

Implemented levels open `LevelExitScene` on `ESC` through `PushOverlay`.

Overlay actions:

- Return: `PopOverlay`
- Retry: `RestartUnderlying`
- Level Select: `ClearToAndGoTo(LevelSelect)`
- Title: `ClearToAndGoTo(Title)`

Pause and resume are delegated to the underlying scene. Physics-enabled levels freeze and unfreeze their local `PhysicsWorld`.

## Level Completion

When a level clears:

1. The level writes best time through `SaveManager::UpdateBestTime(...)`.
2. The level requests `ClearToAndGoTo(LevelSelect)`.
3. `LevelSelectScene` reloads saved level data and updates crown/best-time display.
