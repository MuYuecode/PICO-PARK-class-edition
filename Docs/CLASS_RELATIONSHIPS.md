# Class Relationships

## Inheritance Map

```text
Util::GameObject
├─ Character
│  ├─ UITriangleButton
│  └─ PushableBox            (+ IPhysicsBody, + IPushable)
├─ AnimatedCharacter
│  └─ PlayerCat              (+ IPhysicsBody)
└─ GameText
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
├─ LevelOneScene
├─ LevelTwoScene
└─ LevelThreeScene
```

## Physics Body Implementers

```text
IPhysicsBody
├─ PlayerCat
├─ PushableBox
├─ StaticBody
├─ LevelTwoScene::MovingPlankBody
├─ LevelThreeScene::MovingLiftBody
├─ LevelThreeScene::PatrolMobBody
└─ LevelThreeScene::PipeMobBody

IPushable
└─ PushableBox
```

## Physics Interface Composition

- `IPhysicsBody` composes: `IPhysicsTransform`, `IPhysicsMaterial`, `IPhysicsMotion`, `IPhysicsCollisionListener`, `IPhysicsPushReactive`, `IPhysicsLifecycle`.
- `PhysicsBodyTraits` carries body category (`BodyType`) and behavior flags.

## Interface to Concrete Wiring

```text
IAudioService       -> AudioService
IVisualThemeService -> VisualThemeService
ISessionState       -> SessionState
IGlobalActors       -> GlobalActors
```

- Scenes depend on interfaces via `SceneServices`; concrete objects are wired only in `App::Start()`.

## Ownership and Runtime Links

- `SceneManager` owns scene instances and stack transitions.
- `Scene` instances do not call each other directly; routing is only through `SceneOp` + `SceneId`.
- Each level scene owns its own `PhysicsWorld` and local dynamic entities.
- `SaveManager` is static/centralized and used by options, keyboard config, and level clear-time recording.
