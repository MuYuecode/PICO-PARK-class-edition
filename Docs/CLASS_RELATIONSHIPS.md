# Class Relationships

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

## Interface Contracts

```text
IAudioService       -> AudioService
IVisualThemeService -> VisualThemeService
ISessionState       -> SessionState
IGlobalActors       -> GlobalActors
```

- `Scene` receives only interface references through `SceneServices`.
- `App::Start()` is the composition root that binds concrete implementations.

## Control and Ownership Links

- `SceneManager` owns all scene objects and enforces transition semantics.
- `Scene` owns one pending `SceneOp`; `SceneManager::UpdateCurrent()` consumes at most one op per frame.
- Scenes never call other scenes directly; they communicate through `SceneOpType` commands.

## Physics-Side Coupling

- `PhysicsWorld` tracks participants by `weak_ptr<IPhysicsBody>` and owns only static boundaries (`StaticBody`).
- `PushableBox` holds a non-owning `PhysicsWorld*` to query cooperative push counts and notify push animations.
- `PlayerCat` and `PushableBox` are dual-role objects: render actor + physics body in one instance.

## Persistence and Session Coupling

- `OptionMenuScene` reads/writes option settings via `SaveManager` and previews through audio/theme services.
- `KeyboardConfigScene` synchronizes key profiles to both `SaveManager` and `SessionState`.
- `LevelOneScene` writes best times via `SaveManager::UpdateBestTime()` using current session player count.
