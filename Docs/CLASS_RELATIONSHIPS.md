# Class Relationships

This document captures the current class graph and dependency direction.

## Inheritance Map

```text
Util::GameObject
├─ Character
│  ├─ UITriangleButton
│  └─ PushableBox      (+ IPhysicsBody, + IPushable)
├─ AnimatedCharacter
│  └─ PlayerCat        (+ IPhysicsBody)
└─ GameText
```

```text
IPhysicsBody
├─ PlayerCat
├─ PushableBox
└─ StaticBody

IPushable
└─ PushableBox
```

```text
Scene
├─ TitleScene
├─ MenuScene
├─ ExitConfirmScene
├─ OptionMenuScene
├─ KeyboardConfigScene
├─ LocalPlayScene
├─ LocalPlayGameScene
├─ LevelSelectScene
└─ LevelOneScene  (routed as SceneId::Level01)
```

## Service Contracts

```text
IAudioService       -> AudioService
IVisualThemeService -> VisualThemeService
ISessionState       -> SessionState
IGlobalActors       -> GlobalActors
```

`Scene` stores service references as `m_Audio`, `m_Theme`, `m_Session`, and `m_Actors`.

## Ownership Model

- `App` owns long-lived services and `SceneManager`.
- `SceneManager` owns scenes (`unique_ptr<Scene>`).
- `GlobalActors` stores shared render actors (`Background`, `Floor`, `Door`, `StartupCats`, optional `TestBox`).
- Physics-enabled scenes own one local `PhysicsWorld` each.
- `PhysicsWorld` stores registered bodies as weak references and owns static boundaries it creates.

## Dependency Direction

1. Interfaces (`I*`) define contracts.
2. Concrete services implement contracts.
3. `Scene` depends on interfaces only.
4. Concrete scenes depend on `Scene` and feature classes.
5. `App::Start()` performs concrete wiring.

## Current Practical Couplings

- `LocalPlayScene` depends on `KeyboardConfigScene*` for configured-player checks.
- Persistence uses static `SaveManager` calls.
- `LevelExitScene` exists in file layout but has no implementation.
