# Class Relationships

This file records the current dependency graph and key coupling points.

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
├─ LevelExitScene
└─ LevelOneScene
```

## Service Contracts

```text
IAudioService       -> AudioService
IVisualThemeService -> VisualThemeService
ISessionState       -> SessionState
IGlobalActors       -> GlobalActors
```

`Scene` stores only interface references (`m_Audio`, `m_Theme`, `m_Session`, `m_Actors`).

## Scene Control Contracts

```text
SceneOpType/SceneOp -> consumed by SceneManager
SceneManager        -> owns scene registry and stack ids
```

- Scene output channels:
  - `SceneId` for normal route changes
  - `SceneOp` for overlay stack operations

## Ownership and Lifetime

- `App` owns services, `GlobalActors`, and `SceneManager`.
- `SceneManager` owns all scenes (`std::unique_ptr`).
- `GlobalActors` owns shared render actors (background/header/floor/door/startup cats/test box).
- Physics scenes own local `PhysicsWorld`; `PhysicsWorld` keeps weak body refs and owns created `StaticBody` boundaries.

## Practical Couplings

- `LocalPlayScene` keeps a raw `KeyboardConfigScene*` to validate configured player count before entering gameplay.
- `PushableBox` keeps a non-owning `PhysicsWorld*` for cooperative push queries.
- Scene persistence is centralized through static `SaveManager` calls (`OptionMenuScene`, `KeyboardConfigScene`, `LevelSelectScene`, `LevelOneScene`).
