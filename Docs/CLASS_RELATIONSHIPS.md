# Class Relationships

## Core Inheritance

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

## Service and Runtime Contracts

```text
IAudioService       -> AudioService
IVisualThemeService -> VisualThemeService
ISessionState       -> SessionState
IGlobalActors       -> GlobalActors
```

- `Scene` stores only interface references from `SceneServices`.
- `SceneManager` is the sole executor of `SceneOp` emitted by scenes.

## Scene Control Relationships

```text
Scene::Update() -> RequestSceneOp(...) -> Scene::ConsumeSceneOp() -> SceneManager::UpdateCurrent()
```

- Transition output is single-channel (`SceneOp`) with four active operations:
  - `PushOverlay`, `PopOverlay`, `RestartUnderlying`, `ClearToAndGoTo`.

## Ownership Model

- `App` owns global systems and application lifetime.
- `SceneManager` owns scene instances (`std::unique_ptr<Scene>`).
- `GlobalActors` owns reusable world/UI actors shared across scenes.
- Scene-local gameplay systems (for example `PhysicsWorld`) are owned by each scene instance.

## Intentional Couplings

- `LocalPlayScene` references `KeyboardConfigScene*` to validate configured player count.
- `PushableBox` references `PhysicsWorld*` (non-owning) for cooperative push counting.
- Persistence flows are centralized in `SaveManager` calls from option/key/level scenes.
