# Architecture Overview

## Runtime Loop

- `main.cpp` owns the process entry point and runs `App`.
- `App` is a small state machine: `START -> UPDATE -> END`.
- `App::Update()` frame order is:
  1. `AudioService::UpdateBgm()`
  2. `SceneManager::UpdateCurrent()`
  3. quit gate through `SessionState::ShouldQuit()`
  4. `Renderer::Update()`
- Scene switching is intent-based. A scene requests one `SceneOp`; `SceneManager` consumes at most one op after the scene update.

## Composition Root

`App::Start()` wires process-lifetime systems:

- `GlobalActors`
- `SessionState`
- `SceneManager`
- `BGMPlayer` + `AudioService`
- `VisualThemeService`

It also creates shared actors once and stores them in `GlobalActors`:

- background
- global floor
- header
- door
- startup cats

Scenes receive dependencies through `SceneServices`, so individual scenes depend on service interfaces instead of constructing global systems directly.

## Registered Scenes

`App::Start()` currently registers:

- `Title`
- `Menu`
- `ExitConfirm`
- `OptionMenu`
- `KeyboardConfig`
- `LocalPlay`
- `LocalPlayGame`
- `LevelSelect`
- `LevelExit`
- `Level01`
- `Level02`
- `Level03`
- `Level04`

`SceneId::Level05` through `SceneId::Level10` exist as reserved IDs but are not registered yet.

## Level Select

`LevelSelectScene` exposes 10 slots.

- Slot 0 maps to `Level01`.
- Slot 1 maps to `Level02`.
- Slot 2 maps to `Level03`.
- Slot 3 maps to `Level04`.
- Remaining slots default to `SceneId::None` unless explicitly mapped.

The cover image list includes `LevelOne.png`, `LevelTwo.png`, `LevelThree.png`, and `LevelFour.png`.

## Scene Stack Model

`SceneManager` owns all scene instances and a stack of `SceneId`.

- `GoTo`: exits the full current stack, then enters the target scene.
- `ClearToAndGoTo`: same stack-clearing behavior, used for level clear and menu routing.
- `PushOverlay`: pauses the current top scene, pushes an overlay scene, then enters it.
- `PopOverlay`: exits the overlay and resumes the underlying scene.
- `RestartUnderlying`: exits the overlay, exits and re-enters the underlying scene, then resumes it.

Scenes never mutate the stack directly. They only request a `SceneOp`.

## Ownership Boundaries

- `App` owns long-lived systems.
- `SceneManager` owns scene instances.
- Each level scene owns its own scene-local `PhysicsWorld`.
- `GlobalActors` owns shared visual actors reused across scenes.
- `SaveManager` centralizes persistence for settings and level times.

## Gameplay Route

Main route:

`Title -> Menu -> LocalPlay -> LocalPlayGame -> LevelSelect -> Level01/Level02/Level03/Level04`

Implemented level behavior:

- `LevelOneScene`: cooperative push box level.
- `LevelTwoScene`: button, moving planks, key, door, and multi-player door entry.
- `LevelThreeScene`: shared-consensus control, lifts, mobs, checkpoint, key, and door clear.
- `LevelFourScene`: bullet shooter, jar state progression, key unlock, door entry, and best-time save.

All implemented level scenes open `LevelExitScene` with `ESC`.

## Persistence

`SaveManager` writes:

- `Resources/Save/settings.json`: option settings and keyboard configuration.
- `Resources/Save/save_data.json`: per-level best times for player counts 2P through 8P.

Each cleared level calls `SaveManager::UpdateBestTime(levelIndex, playerCount, elapsedSeconds)` before returning to `LevelSelect`.
