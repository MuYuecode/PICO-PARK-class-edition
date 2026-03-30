# Component Interaction and Runtime Flow

This document describes verified runtime collaboration between core systems.

## Frame Pipeline

`App::Update()` runs:

1. `IAudioService::UpdateBgm()`
2. `SceneManager::UpdateCurrent()`
3. if `ISessionState::ShouldQuit()` then `App::State = END`
4. `Renderer::Update()`

## SceneManager Update Contract

`SceneManager::UpdateCurrent()` does:

1. call top scene `Update()` and capture returned `SceneId`
2. consume pending `SceneOp` from that scene
3. if `SceneOp` exists, execute it (priority over `SceneId`)
4. otherwise process normal `SceneId` transition via `GoTo`

Overlay operations:

- `PushOverlay`: pause base scene, push overlay, call overlay `OnEnter()`
- `PopOverlay`: overlay `OnExit()`, pop, resume base scene
- `RestartUnderlying`: overlay `OnExit()`, pop, then base `OnExit()` + `OnEnter()`
- `ClearToAndGoTo`: clear stack and enter target scene

## Shared Services and Actors

- `IAudioService`: BGM control from scenes (`OptionMenuScene` updates volume preview/commit).
- `IVisualThemeService`: background preview and apply/restore flow.
- `ISessionState`: selected player count, key configs, cooperative value, quit request.
- `IGlobalActors`: shared render root and reusable actors (`Background`, `Header`, `Floor`, `Door`, startup cats, optional test box).

## High-Impact Flows

- `ExitConfirmScene`: YES triggers `m_Session.RequestQuit()`; app exits on next frame check.
- `OptionMenuScene`: edits pending settings, previews via audio/theme services, saves with `SaveManager` on OK.
- `KeyboardConfigScene`: edits per-player bindings, validates conflicts, saves via `SaveManager`, syncs into `ISessionState`.
- `LocalPlayScene`: blocks entry when selected players exceed configured keyboard profiles.
- `LevelSelectScene`: reads level save data, shows best times/crowns, routes to mapped level `SceneId`.

## Level Gameplay + Overlay Flow

- `LevelOneScene` update order: input -> `PhysicsWorld::Update()` -> timer -> key/door logic -> clear check.
- `ESC` in `LevelOneScene` emits `SceneOpType::PushOverlay` to `SceneId::LevelExit`.
- `LevelOneScene::PauseGameplay()` freezes world; `ResumeGameplay()` unfreezes it.
- `LevelExitScene` maps actions to scene ops:
  - RETURN GAME / ESC / exit button -> `PopOverlay`
  - RETRY -> `RestartUnderlying`
  - LEVEL SELECT -> `ClearToAndGoTo(LevelSelect)`
  - TITLE -> `ClearToAndGoTo(Title)`
