# Header Purpose Reference

This document summarizes the purpose of each public project header under `include/`.

## App

- `app/App.hpp`: owns process-level systems and drives the `START/UPDATE/END` app state.
- `app/AppUtil.hpp`: shared UI and sprite helpers, including key-name conversion, hit-test helpers, and `Character` alignment helpers.

## Core

- `core/Scene.hpp`: base scene interface with lifecycle hooks and pending `SceneOp` support.
- `core/SceneId.hpp`: enum IDs for registered and reserved scenes.
- `core/SceneManager.hpp`: owns scenes, maintains the scene stack, and applies scene operations.
- `core/SceneOp.hpp`: value type describing scene stack operations.
- `core/SceneServices.hpp`: dependency bundle passed into scenes.

## Services

- `services/IAudioService.hpp`: audio service interface.
- `services/AudioService.hpp`: BGM-facing audio service implementation.
- `services/BGMPlayer.hpp`: lower-level BGM player wrapper.
- `services/IVisualThemeService.hpp`: visual theme service interface.
- `services/VisualThemeService.hpp`: background theme service implementation.
- `services/ISessionState.hpp`: session state interface.
- `services/SessionState.hpp`: selected player count, keyboard configs, cooperative power, and quit state.
- `services/IGlobalActors.hpp`: shared actor access interface.
- `services/GlobalActors.hpp`: shared actor storage for root, background, floor, header, door, startup cats, and optional test box.
- `services/SaveManager.hpp`: JSON persistence for settings, key configs, and level best times.

## Gameplay

- `gameplay/Character.hpp`: image-backed visual actor.
- `gameplay/AnimatedCharacter.hpp`: animated image actor.
- `gameplay/PlayerCat.hpp`: controllable cat actor and `IPhysicsBody`.
- `gameplay/PushableBox.hpp`: pushable box actor, `IPhysicsBody`, and `IPushable`.
- `gameplay/IPushable.hpp`: marker/interface for pushable gameplay objects.
- `gameplay/PlayerKeyConfig.hpp`: per-player key binding value type.
- `gameplay/CatAssets.hpp`: helper for building cat animation asset paths.
- `gameplay/BoundaryFactory.hpp`: helper for constructing static boundary objects.

## UI

- `ui/GameText.hpp`: renderable text object.
- `ui/UITriangleButton.hpp`: triangle-button UI component used by menus.

## Physics

- `physics/IPhysicsBody.hpp`: aggregate physics body interface.
- `physics/IPhysicsTransform.hpp`: position and half-size interface.
- `physics/IPhysicsMaterial.hpp`: solidity and kinematic flags.
- `physics/IPhysicsMotion.hpp`: per-frame motion intent and resolved movement interface.
- `physics/IPhysicsCollisionListener.hpp`: collision callback interface.
- `physics/IPhysicsLifecycle.hpp`: active/frozen/post-update lifecycle interface.
- `physics/IPhysicsPushReactive.hpp`: push reaction hook.
- `physics/PhysicsBodyTraits.hpp`: `BodyType` and behavior flags.
- `physics/PhysicsWorld.hpp`: local physics world, body registry, static boundary ownership, update pipeline, and push queries.
- `physics/PhysicsUpdateScheduler.hpp`: two-pass `PhysicsUpdate()` scheduler.
- `physics/PhysicsSnapshot.hpp`: per-frame body snapshot structs used by collision resolution.
- `physics/CollisionSolver.hpp`: support-aware AABB collision resolution and collision callback emission.
- `physics/SupportResolver.hpp`: riding/support detection before resolution.
- `physics/PushForceResolver.hpp`: recursive cooperative push counting.
- `physics/IPushQueryService.hpp`: push-query interface used by `PushableBox`.
- `physics/StaticBody.hpp`: world-owned solid static collider.
- `physics/BulletBody.hpp`: moving bullet body that records collision hit type for LevelFour gameplay.
- `physics/LevelThreeScenePhysicsBodies.hpp`: LevelThree-only moving lift and mob body definitions.
- `physics/CharacterPhysicsSystem.hpp`: older character physics integration helper retained in the codebase.

## Scenes

- `scenes/TitleScene.hpp`: title screen.
- `scenes/MenuScene.hpp`: main menu routing to local play, options, and exit.
- `scenes/ExitConfirmScene.hpp`: quit confirmation overlay.
- `scenes/OptionMenuScene.hpp`: theme/audio options.
- `scenes/KeyboardConfigScene.hpp`: keyboard binding editor for players 1 through 8.
- `scenes/LocalPlayScene.hpp`: local player-count selection and config validation.
- `scenes/LocalPlayGameScene.hpp`: cooperative free-play room before level select.
- `scenes/LevelSelectScene.hpp`: 10-slot level grid with best-time/crown display.
- `scenes/LevelExitScene.hpp`: in-level pause overlay.
- `scenes/LevelOneScene.hpp`: Level01 gameplay scene.
- `scenes/LevelTwoScene.hpp`: Level02 gameplay scene.
- `scenes/LevelThreeScene.hpp`: Level03 gameplay scene.
- `scenes/LevelFourScene.hpp`: Level04 gameplay scene with shooter bullets, jar states, key, and door clear.
