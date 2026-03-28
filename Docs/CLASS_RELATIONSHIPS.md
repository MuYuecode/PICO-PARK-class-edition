# Class Relationships

> Method listings → `ARCHITECTURE.md §5`.  Physics pipeline detail → `PHYSICS_DESIGN.md`.
> This document focuses on OOP structure: inheritance, composition, dependency.

## Contents
1. [Inheritance Overview](#1-inheritance-overview)
2. [Render Object Chain](#2-render-object-chain)
3. [Scene ABC and Concrete Scenes](#3-scene-abc-and-concrete-scenes)
4. [Physics System Composition](#4-physics-system-composition)
5. [SaveManager](#5-savemanager)
6. [Cross-file Dependency Graph](#6-cross-file-dependency-graph)
7. [Design Pattern Index](#7-design-pattern-index)

---

## 1. Inheritance Overview

```
Util::GameObject              (PTSD engine base — all renderable objects)
├── Character                 Character.hpp / .cpp
│   ├── UITriangleButton      UITriangleButton.hpp / .cpp
│   └── PushableBox           PushableBox.hpp / .cpp  (also implements IPhysicsBody + IPushable)
├── AnimatedCharacter         AnimatedCharacter.hpp / .cpp
│   └── PlayerCat             PlayerCat.hpp / .cpp    (also implements IPhysicsBody)
└── GameText                  GameText.hpp / .cpp

IPhysicsBody                  IPhysicsBody.hpp  (pure-virtual ABC)
├── PlayerCat                 (CHARACTER — via multiple inheritance, fully implemented)
├── PushableBox               (PUSHABLE_BOX — via multiple inheritance, fully implemented)
└── StaticBody                StaticBody.hpp / .cpp  (STATIC_BOUNDARY, fully implemented)

IPushable                     IPushable.hpp  (secondary pure-virtual ABC)
└── PushableBox               (only method: GetRequiredPushers())

Scene                         Scene.hpp  (pure-virtual ABC)
├── TitleScene
├── MenuScene
├── ExitConfirmScene
├── OptionMenuScene
├── KeyboardConfigScene
├── LocalPlayScene
├── LocalPlayGameScene
├── LevelSelectScene
├── LevelOneScene             (not yet implemented — empty stub)
└── LevelExitScene            (not yet implemented — empty stub)

SaveManager                   SaveManager.hpp / .cpp  (pure-static utility, no base class)
```

---

## 2. Render Object Chain

### 2.1 `Character`
**IS-A** `Util::GameObject`. Encapsulates a static image (`Util::Image`). Handles AABB mouse interaction. Base for all image-type UI elements.

### 2.2 `UITriangleButton`
**IS-A** `Character`. Adds two-image pressed/normal switching and a countdown timer (`m_PressTimer`). Uses `Character::SetImage()` for visual feedback without object reconstruction.

### 2.3 `AnimatedCharacter`
**IS-A** `Util::GameObject` (parallel to `Character`, not derived from it). `m_Drawable` is `Util::Animation`. Encapsulates single-clip playback. Does **not** inherit `Character` because their `Drawable` types differ — forcing inheritance would violate LSP.

### 2.4 `PlayerCat`
**IS-A** `AnimatedCharacter` + **IS-A** `IPhysicsBody` (multiple inheritance, fully implemented).

```cpp
class PlayerCat : public AnimatedCharacter, public IPhysicsBody { … };
//                       ↑ render             ↑ physics interface
```

Holds six animation clips (stand / run / jump_rise / jump_fall / land / push) driven by a state machine in `PostUpdate() → UpdateAnimState()`.

### 2.5 `PushableBox`
**IS-A** `Character` (renders a static image) + **IS-A** `IPhysicsBody` + **IS-A** `IPushable`.

```cpp
class PushableBox : public Character, public IPhysicsBody, public IPushable { … };
```

Carries an owned `GameText` (`m_CountText`) that displays the remaining number of pushers needed. Both the box and its text must be added to the renderer separately.

### 2.6 `StaticBody`
**IS-A** `IPhysicsBody` only — no render component. Used exclusively for invisible collision geometry (floor, ceiling, walls). Created and owned by `PhysicsWorld` via `AddStaticBoundary()`.

### 2.7 `GameText`
**IS-A** `Util::GameObject`. `m_Drawable` is `Util::Text`. Parallel to `Character` and `AnimatedCharacter`.

---

## 3. Scene ABC and Concrete Scenes

### 3.1 `Scene` (`Scene.hpp`)

```cpp
class Scene {
public:
    virtual void   OnEnter() = 0;
    virtual void   OnExit()  = 0;
    virtual Scene* Update()  = 0;
protected:
    GameContext& m_Ctx;   // reference: non-null, non-reassignable (borrowed, not owned)
};
```

Three pure-virtual functions enforce the complete lifecycle contract on all subclasses.

### 3.2 Inter-scene Pointer Ownership

All ownership is in `App` (`unique_ptr`). Scenes hold **non-owning raw pointers** to each other.

```
App (unique_ptr owners)
├── TitleScene ─────────────────────────→ MenuScene*
├── MenuScene ──────────────────────────→ TitleScene*, ExitConfirmScene*,
│                                         OptionMenuScene*, LocalPlayScene*
├── ExitConfirmScene ───────────────────→ MenuScene*
├── OptionMenuScene ────────────────────→ MenuScene*, KeyboardConfigScene*
├── KeyboardConfigScene ────────────────→ OptionMenuScene*
├── LocalPlayScene ─────────────────────→ MenuScene*, KeyboardConfigScene*, LocalPlayGameScene*
├── LocalPlayGameScene ─────────────────→ LocalPlayScene*, KeyboardConfigScene*, LevelSelectScene*
└── LevelSelectScene ───────────────────→ LocalPlayGameScene*, LevelNScene*[0..9] (nullptr until implemented)
```

### 3.3 MenuScene Shared UI — Borrowing Pattern

`MenuScene` **owns** (via `shared_ptr`):

```
m_MenuFrame, m_ExitGameButton, m_LeftTriButton, m_RightTriButton, m_blue_cat_run_img
```

Injected at construction into borrower scenes via `AppStart.cpp`. Borrowers call `AddChild`/`RemoveChild` in `OnEnter`/`OnExit`; they never `delete` or `reset()`. On `OnExit`, borrowers restore the original position/scale.

```
m_MenuFrame          → ExitConfirmScene, LocalPlayScene
m_ExitGameButton     → ExitConfirmScene, OptionMenuScene, KeyboardConfigScene
m_LeftTriButton      → LocalPlayScene
m_RightTriButton     → LocalPlayScene
m_blue_cat_run_img   → LocalPlayScene
```

### 3.4 LevelSelectScene UI Ownership

`LevelSelectScene` **self-owns** all UI objects (borrows nothing from MenuScene):

```
m_SelectorFrame              ← level_select_frame.png  (ZIndex 32)
m_TitleText                  ← "LEVEL N"               (ZIndex 35)
m_BestTimeText               ← "m PLAYERS BEST TIME: …" (ZIndex 35)
m_LevelCover[LEVEL_COUNT]    ← Character array with level cover images (ZIndex 35)
m_Crown[LEVEL_COUNT]         ← Character array with Crown.png (ZIndex 36)
```

---

## 4. Physics System Composition

### 4.1 Overview

The physics system is built around three layers: the `IPhysicsBody` interface
that every simulated object must implement, the concrete body types (`PlayerCat`,
`PushableBox`, `StaticBody`), and `PhysicsWorld` as the single per-scene
coordinator that runs the entire two-phase pipeline.

```
Scene (e.g. LocalPlayGameScene, TitleScene, MenuScene)
│
└── HAS-A  PhysicsWorld  m_World
               │
               ├── owns (via m_OwnedStatics)  StaticBody[]   (floor / ceiling / walls)
               ├── weak_ptr refs              PlayerCat[]    (registered on spawn)
               └── weak_ptr refs              PushableBox[]  (registered on spawn)
```

### 4.2 `IPhysicsBody` — The Physics Interface

Every simulated object implements this pure-virtual ABC. The key methods are:

```cpp
GetBodyType()        // enum tag for type dispatch
GetPosition() / SetPosition()
GetHalfSize()        // AABB half-dimensions (x and y)
IsSolid()            // true = blocks other bodies
IsKinematic()        // true = moves by own logic, not resolved against geometry
GetDesiredDelta()    // the delta this body *wants* to move this frame
ApplyResolvedDelta() // apply the collision-resolved delta (may differ from desired)
OnCollision()        // called post-resolution with contact normal
PhysicsUpdate()      // per-frame self-drive (gravity, input, push queries)
PostUpdate()         // called after ApplyResolvedDelta; used for animation updates
NotifyPush()         // called by PushableBox to set the push animation on a cat
GetMoveDir()         // current horizontal intent (-1 / 0 / +1); CHARACTER only meaningful
```

### 4.3 `IPushable` — Pushable Object Contract

A lightweight secondary interface requiring only:

```cpp
int GetRequiredPushers() const;   // how many net pushing characters are needed
```

`PushableBox` implements this. Future types such as `ConditionalPlatform` may
also implement it to express activation thresholds.

### 4.4 `StaticBody` — Immovable Collision Geometry

`StaticBody` is a pure-physics object (no visual component). It is always
`IsSolid() = true` and `IsKinematic() = true`, which causes `PhysicsWorld` to
mark it as resolved immediately with delta `{0,0}`. All dynamic bodies
resolve against it but it never moves itself. Created and owned exclusively by
`PhysicsWorld::AddStaticBoundary()`.

### 4.5 `PhysicsWorld` — Pipeline Coordinator

`PhysicsWorld` is a **non-copyable value member** of each scene that needs
physics. It runs the full two-phase pipeline on every `Update()` call:

```
Phase 1: StepPhysicsUpdate()
         → all CHARACTER bodies call PhysicsUpdate() (commit desired deltas + moveDir)
         → all PUSHABLE_BOX bodies call PhysicsUpdate() (query CountCharactersPushing)

Resolve: StepResolveAndApply()
         → DetectRiding (stacking relationships)
         → topological collision resolution (supports first, riders after)
         → ApplyResolvedDelta on all bodies

Callbacks: StepCollisionCallbacks()
           → OnCollision() on each body that recorded a contact normal

PostUpdate: PostUpdate() on all active non-frozen bodies
```

See `PHYSICS_DESIGN.md §2` for the full annotated pipeline.

### 4.6 `CharacterPhysicsSystem` — Constants Only

After the refactoring, `CharacterPhysicsSystem` is a constants-only header.
Its former `static void Update(...)`, `ResolveHorizontal(...)`, and
`ApplyJump(...)` methods no longer exist. The constants (`kGravity`,
`kJumpForce`, `kGroundMoveSpeed`, etc.) are still useful as shared documentation.
All runtime logic lives in `PlayerCat::PhysicsUpdate()` and `PhysicsWorld`.

### 4.7 Scene ↔ PhysicsWorld Contract

```
OnEnter:
    m_World.Clear()
    for each cat: m_World.Register(cat)
    for each box: box->SetWorld(&m_World); m_World.Register(box)
    SetupStaticBoundaries()   // calls m_World.AddStaticBoundary(...)

Update (each frame):
    // 1. read input → call SetMoveDir / Jump on cats
    // 2. m_World.Update()   ← single call drives the entire pipeline

OnExit:
    m_World.Clear()
```

---

## 5. SaveManager

```
SaveManager  (pure-static, no inheritance)
│
├── Dependencies: nlohmann::json, std::filesystem
│
├── POD structs (data transfer):
│   ├── OptionSettingsData    bgColorIndex, bgmVolume, seVolume, dispNumber
│   ├── KeyConfigData         up/down/left/right/jump/cancel/shot/menu/subMenu (int)
│   └── LevelSaveData         completed (bool), bestTimes[9] (float)
│
└── Static methods:
    ├── settings.json group:   Save/LoadOptionSettings, Save/LoadKeyConfigs
    └── save_data.json group:  Save/LoadLevelData, UpdateBestTime, FormatTime
```

### Scene ↔ SaveManager Call Points

| Scene | Timing | Direction |
|-------|--------|-----------|
| `OptionMenuScene` constructor | restore `m_Applied` | read |
| `OptionMenuScene::Update()` OK | persist settings | write |
| `KeyboardConfigScene` constructor | restore `m_Applied[]` | read |
| `KeyboardConfigScene::CommitPending()` | persist key bindings | write |
| `LevelSelectScene::OnEnter()` | load level save data | read |
| Level scene completion | `UpdateBestTime()` | read+write |

### `OptionSettingsData` vs `OptionMenuScene::Settings`

Identical fields, but **deliberately separated** to avoid `SaveManager.hpp` depending on `OptionMenuScene.hpp` (which would create a circular include). Conversion is a direct field-by-field assignment in the OK branch.

---

## 6. Cross-file Dependency Graph

Arrows = `#include` dependency.

```
main.cpp
  └── App.hpp
        ├── Scene.hpp ──── GameContext.hpp ─── Character.hpp, PlayerCat.hpp,
        │                                       PushableBox.hpp, BGMPlayer.hpp
        └── Scene subclasses (all #include Scene.hpp)

IPhysicsBody.hpp ── (no game-class dependencies; only glm)

IPushable.hpp    ── (no dependencies)

StaticBody.hpp   ── IPhysicsBody.hpp

PlayerCat.hpp    ── AnimatedCharacter.hpp, IPhysicsBody.hpp

PushableBox.hpp  ── Character.hpp, IPhysicsBody.hpp, IPushable.hpp, GameText.hpp

PhysicsWorld.hpp ── IPhysicsBody.hpp, StaticBody.hpp

LocalPlayGameScene.hpp ── Scene.hpp, PlayerCat.hpp, PhysicsWorld.hpp,
                          KeyboardConfigScene.hpp

LevelSelectScene.hpp ── Scene.hpp, Character.hpp, GameText.hpp, SaveManager.hpp

SaveManager.hpp ── (no game-class dependencies; only std:: and nlohmann/json)

CharacterPhysicsSystem.hpp ── (no dependencies; constants only)

CatAssets.hpp ── PlayerCat.hpp   (header-only utility)

AppUtil.hpp ── Character.hpp, GameText.hpp, Util/Keycode.hpp
```

**Design rules enforced:**
- `SaveManager` never `#include`s any Scene or game-object header (unidirectional dependency).
- `CharacterPhysicsSystem` never `#include`s any Scene.
- `IPhysicsBody`, `IPushable`, and `StaticBody` do not include any Scene or concrete game-object headers.
- `GameContext` only includes the concrete types it holds (`Character`, `PlayerCat`, `PushableBox`, `BGMPlayer`), not Scenes.
- Scenes depend on `GameContext`, not on each other's full headers (forward declarations + raw pointers).

---

## 7. Design Pattern Index

| Pattern | Location | Description |
|---------|----------|-------------|
| **Template Method** | `Scene` ABC | `OnEnter`/`OnExit`/`Update` contract enforced; subclasses fill details |
| **State Machine** | `App::State`, `PlayerCat` animation | App uses enum for START/UPDATE/END; PlayerCat uses `CatAnimState` for clip switching |
| **Two-Phase Pipeline** | `PhysicsWorld::Update` | Phase 1 computes desired deltas; Phase 2 resolves collisions and applies; clean separation prevents mid-frame interference |
| **Topological Resolution** | `PhysicsWorld::StepResolveAndApply` | Resolve supports before riders — iterative passes handle stacking chains of arbitrary depth |
| **Stateless Utility** | `SaveManager`, `CharacterPhysicsSystem` (constants) | All-static classes reusable by multiple scenes with no instance state |
| **DTO** | `OptionSettingsData`, `KeyConfigData`, `LevelSaveData` | Data packets crossing Scene ↔ SaveManager boundaries |
| **Borrowing** | MenuScene shared UI | Fixed `shared_ptr` owner; borrowers pair AddChild/RemoveChild in OnEnter/OnExit |
| **Dependency Injection** | Scene constructors, `PushableBox::SetWorld` | Avoid global access; dependencies passed in from outside |
| **Abstract Base Class** | `Scene`, `IPhysicsBody`, `IPushable` | Compile-time enforcement of complete interface |
| **Composition over Inheritance** | Scenes borrowing MenuScene UI | Avoids deep inheritance tree, reduces coupling |
| **Pending / Applied** | `KeyboardConfigScene`, `OptionMenuScene` | In-edit buffer; committed to live value and persisted only on OK |
| **Read-Modify-Write** | `SaveManager` | `settings.json` shared by two scenes; each modifies only its own fields |
| **Header-only Utility** | `CatAssets.hpp` | `inline` functions for resource paths; no compilation unit needed |
