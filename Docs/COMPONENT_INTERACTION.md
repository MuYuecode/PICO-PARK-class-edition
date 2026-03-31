# Component Interaction

## Per-Frame Flow

`App::Update()` executes:

1. `IAudioService::UpdateBgm()`
2. `SceneManager::UpdateCurrent()`
3. quit check (`ISessionState::ShouldQuit()`)
4. `m_Root.Update()`

## Scene Transition Handshake

Current scene interaction is strictly:

1. `SceneManager` calls `current->Update()`
2. scene may call `RequestSceneOp(...)`
3. `SceneManager` reads `current->ConsumeSceneOp()`
4. manager executes the operation if present

There is no `SceneId` return-based transition path in scene `Update()`.

## Overlay Stack Behavior

- `PushOverlay`: pauses base scene (`PauseGameplay()`), pushes overlay, calls overlay `OnEnter()`.
- `PopOverlay`: calls overlay `OnExit()`, pops stack, resumes base (`ResumeGameplay()`).
- `RestartUnderlying`: exits overlay, then re-enters base (`OnExit()` -> `OnEnter()`).
- `ClearToAndGoTo`: exits current stack and enters target scene as new base.

## Shared Services in Scene Logic

- `IAudioService`: BGM lifecycle and option preview/commit volume changes.
- `IVisualThemeService`: background preview and apply/restore behavior.
- `ISessionState`: selected player count, key bindings, cooperative values, quit flag.
- `IGlobalActors`: shared root and reusable actors (background, header, floor, door, startup cats, optional test box).

## Key Runtime Interaction Paths

- `ExitConfirmScene`: YES calls `RequestQuit()`, app exits in later frame gate.
- `OptionMenuScene`: edits pending settings, previews theme/audio, persists on OK; `seVolume` and `dispNumber` are currently UI/persistence-focused.
- `KeyboardConfigScene`: edits per-player bindings, conflict-gates save, syncs to session.
- `LocalPlayScene`: compares selected player count with configured profiles before entering gameplay.
- `LevelSelectScene`: loads save data, updates best-time/crown UI, routes to mapped level scene.

## Gameplay + Overlay Example (`LevelOne`)

- `LevelOneScene::Update()` order: input -> `PhysicsWorld::Update()` -> timer -> key/door logic -> clear gate.
- ESC emits `PushOverlay(LevelExit)`.
- `LevelOneScene::PauseGameplay()` / `ResumeGameplay()` map to `PhysicsWorld::FreezeAll()` / `UnfreezeAll()`.
- `LevelExitScene` confirmation emits:
  - return game: `PopOverlay`
  - retry: `RestartUnderlying`
  - level select/title: `ClearToAndGoTo(...)`
