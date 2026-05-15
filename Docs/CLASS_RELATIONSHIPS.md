# Class Relationships

## Game Object Hierarchy

```text
Util::GameObject
  Character
    PushableBox (+ IPhysicsBody, + IPushable)
    UITriangleButton
  AnimatedCharacter
    PlayerCat (+ IPhysicsBody)
  GameText
```

Notes:

- `Character` is the base visual actor for image-backed gameplay objects.
- `AnimatedCharacter` is a separate animated `GameObject` base, not a `Character` subclass.
- `PlayerCat` is both an animated actor and a physics body.
- `PushableBox` is both a renderable actor and the only `IPushable` implementation.
- `GameText` is text-backed UI/gameplay display.

## Scene Hierarchy

```text
Scene
  TitleScene
  MenuScene
  ExitConfirmScene
  OptionMenuScene
  KeyboardConfigScene
  LocalPlayScene
  LocalPlayGameScene
  LevelSelectScene
  LevelExitScene
  LevelOneScene
  LevelTwoScene
  LevelThreeScene
  LevelFourScene
```

Every scene implements at least `OnEnter()`, `OnExit()`, and `Update()`. Gameplay scenes that can be paused by `LevelExitScene` also override `PauseGameplay()` and `ResumeGameplay()`.

## Physics Body Implementers

```text
IPhysicsBody
  PlayerCat
  PushableBox
  StaticBody
  BulletBody
  LevelTwoScene::MovingPlankBody
  LevelThreeScene::MovingLiftBody
  LevelThreeScene::PatrolMobBody
  LevelThreeScene::PipeMobBody
```

`IPhysicsBody` is composed from smaller interfaces:

- `IPhysicsTransform`
- `IPhysicsMaterial`
- `IPhysicsMotion`
- `IPhysicsCollisionListener`
- `IPhysicsPushReactive`
- `IPhysicsLifecycle`

`PhysicsBodyTraits` gives each body a `BodyType` plus behavior flags. The current `BodyType` values are:

- `CHARACTER`
- `PUSHABLE_BOX`
- `PATROL_ENEMY`
- `MOVING_PLATFORM`
- `CONDITIONAL_PLATFORM`
- `ROPE_ENDPOINT`
- `BULLET`
- `JAR`
- `STATIC_BOUNDARY`

## Services

```text
IAudioService          -> AudioService
IVisualThemeService    -> VisualThemeService
ISessionState          -> SessionState
IGlobalActors          -> GlobalActors
```

Scenes receive these through `SceneServices`.

## Runtime Ownership

- `SceneManager` owns all scene objects and their stack state.
- Each physics-enabled scene owns one `PhysicsWorld`.
- `PhysicsWorld` owns `StaticBody` instances created by `AddStaticBoundary()`.
- Dynamic physics bodies are owned by scenes or gameplay objects and registered into the local `PhysicsWorld`.
- Shared actors such as background, floor, header, door, and startup cats are owned by `GlobalActors`.
- `SaveManager` is static and centralizes JSON persistence.
