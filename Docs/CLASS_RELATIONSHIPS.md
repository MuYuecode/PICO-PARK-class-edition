# Class Relationships

## Inheritance Skeleton

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

## Interface-to-Concrete Bindings

```text
IAudioService       -> AudioService
IVisualThemeService -> VisualThemeService
ISessionState       -> SessionState
IGlobalActors       -> GlobalActors
```

- `Scene` receives only interfaces through `SceneServices`; concrete wiring happens in `App::Start()`.

## Control and Ownership Links

- `SceneManager` owns all scene instances and stack transitions.
- Each scene owns one pending transition intent (`SceneOp`), consumed by manager post-update.
- Scenes are decoupled from each other; routing happens through `SceneId` + `SceneOpType` only.

## State and Persistence Coupling

- `SessionState` is the runtime source for selected player count, cooperative push power, key configs, and quit flag.
- `KeyboardConfigScene` writes key configs to both `SaveManager` and `SessionState`.
- `OptionMenuScene` uses live service preview (audio/theme) and persists only on commit.
- `LevelOneScene`, `LevelTwoScene`, and `LevelThreeScene` write clear-time records via `SaveManager::UpdateBestTime()`.
