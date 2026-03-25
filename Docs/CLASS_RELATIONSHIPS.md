# Class Relationships

> Method listings → `ARCHITECTURE.md §5`.  Physics interface skeletons → `PHYSICS_DESIGN.md`.
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
│   └── UI_Triangle_Button    UI_Triangle_Button.hpp / .cpp
├── AnimatedCharacter         AnimatedCharacter.hpp / .cpp
│   └── PlayerCat             PlayerCat.hpp / .cpp
│           └── IPhysicsBody  IPhysicsBody.hpp   (multiple inheritance, fully implemented)
└── GameText                  GameText.hpp / .cpp

Scene                         Scene.hpp  (pure-virtual ABC)
├── TitleScene                Titlescene.hpp / .cpp
├── MenuScene                 Menuscene.hpp / .cpp
├── ExitConfirmScene          Exitconfirmscene.hpp / .cpp
├── OptionMenuScene           OptionMenuScene.hpp / .cpp
├── KeyboardConfigScene       KeyboardConfigScene.hpp / .cpp
├── LocalPlayScene            LocalPlayScene.hpp / .cpp
├── LocalPlayGameScene        LocalPlayGameScene.hpp / .cpp
└── LevelSelectScene          LevelSelectScene.hpp / .cpp

SaveManager                   SaveManager.hpp / .cpp  (pure-static utility, no base class)
```

---

## 2. Render Object Chain

### 2.1 `Character`
**IS-A** `Util::GameObject`. Encapsulates a static image (`Util::Image`). Handles AABB mouse interaction. Base for all image-type UI elements.

### 2.2 `UI_Triangle_Button`
**IS-A** `Character`. Adds two-image pressed/normal switching and a countdown timer (`m_PressTimer`). Uses `Character::SetImage()` for visual feedback without object reconstruction.

### 2.3 `AnimatedCharacter`
**IS-A** `Util::GameObject` (parallel to `Character`, not derived from it). `m_Drawable` is `Util::Animation`. Encapsulates single-clip playback. Does **not** inherit `Character` because their `Drawable` types differ — forcing inheritance would violate LSP.

### 2.4 `PlayerCat`
**IS-A** `AnimatedCharacter` + **IS-A** `IPhysicsBody` (multiple inheritance, fully implemented).

```cpp
class PlayerCat : public AnimatedCharacter, public IPhysicsBody { … };
//                       ↑ render             ↑ physics interface
```

Holds six animation clips (stand / run / jump_rise / jump_fall / land / push) with a state machine.

### 2.5 `GameText`
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

### 4.1 Structure

```
LocalPlayGameScene
├── calls (static)  CharacterPhysicsSystem::Update   (stateless algorithm)
├── HAS-A  vector<PlayerBinding>  m_Players
│              └── PlayerBinding
│                    ├── PhysicsAgent  agent
│                    │     ├── shared_ptr<PlayerCat>  actor
│                    │     └── PhysicsState            state
│                    ├── PlayerKeyConfig               key
│                    └── bool                          entered
└── reads  GameContext::Floor (floor reference) / GameContext::Door (image swap)
```

### 4.2 `PhysicsState` — POD data, no methods

```cpp
struct PhysicsState {
    float velocityY;      // vertical velocity
    float lastDeltaX;     // last frame's horizontal delta (carry calc)
    int   supportIndex;   // index of character being stood on (-1 = none)
    int   moveDir;        // -1 / 0 / +1
    bool  grounded;
    bool  prevGrounded;   // previous frame (landing detection)
    bool  beingStoodOn;   // someone is on this character's head
};
```

### 4.3 `PhysicsAgent` — DTO

```cpp
struct PhysicsAgent {
    shared_ptr<PlayerCat> actor;
    PhysicsState          state;
};
```

Packs actor + state so `CharacterPhysicsSystem::Update` can process all characters with a uniform view.

### 4.4 `CharacterPhysicsSystem` — Stateless Service

All methods are `static`. No instance state. Can be reused by any scene.

```cpp
static void  Update(vector<PhysicsAgent>&, const shared_ptr<Character>& floor);
static float ResolveHorizontal(int idx, float targetX, const vector<PhysicsAgent>&);
static void  ApplyJump(PhysicsState& state);
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
        ├── Scene.hpp ──── GameContext.hpp ─── Character.hpp, PlayerCat.hpp, BGMPlayer.hpp
        └── Scene subclasses (all #include Scene.hpp)

CharacterPhysicsSystem.hpp ── PlayerCat.hpp, Character.hpp

LocalPlayGameScene.hpp ── CharacterPhysicsSystem.hpp, KeyboardConfigScene.hpp, Scene.hpp

LevelSelectScene.hpp ── Scene.hpp, Character.hpp, GameText.hpp, SaveManager.hpp

SaveManager.hpp ── (no game-class dependencies; only std:: and nlohmann/json)

OptionMenuScene.cpp / KeyboardConfigScene.cpp ── SaveManager.hpp

PlayerCat.hpp ── AnimatedCharacter.hpp, IPhysicsBody.hpp

CatAssets.hpp ── PlayerCat.hpp   (header-only utility)

AppUtil.hpp ── Character.hpp, GameText.hpp, Util/Keycode.hpp
```

**Design rules enforced:**
- `SaveManager` never `#include`s any Scene or game-object header (unidirectional dependency).
- `CharacterPhysicsSystem` never `#include`s any Scene.
- `GameContext` only includes the concrete types it needs (`Character`, `PlayerCat`, `BGMPlayer`), not Scenes.
- Scenes depend on `GameContext`, not on each other's full headers (forward declarations + raw pointers).

---

## 7. Design Pattern Index

| Pattern | Location | Description |
|---------|----------|-------------|
| **Template Method** | `Scene` ABC | `OnEnter`/`OnExit`/`Update` contract enforced, subclasses fill details |
| **State Machine** | `App::State`, `PlayerCat` animation | App uses enum for START/UPDATE/END; PlayerCat uses `CatAnimState` for clip switching |
| **Stateless Service** | `CharacterPhysicsSystem`, `SaveManager` | All-static algorithm/utility classes reusable by multiple scenes |
| **DTO** | `PhysicsAgent`, `PhysicsState`, `OptionSettingsData`, `KeyConfigData`, `LevelSaveData` | Data packets crossing Scene↔System / SaveManager boundaries |
| **Borrowing** | MenuScene shared UI | Fixed `shared_ptr` owner; borrowers pair AddChild/RemoveChild in OnEnter/OnExit |
| **Dependency Injection** | Scene constructors | Avoid global access; dependencies passed in from outside |
| **Abstract Base Class** | `Scene`, `IPhysicsBody` | Compile-time enforcement of complete interface |
| **Composition over Inheritance** | Scenes borrowing MenuScene UI | Avoids deep inheritance tree, reduces coupling |
| **Pending / Applied** | `KeyboardConfigScene`, `OptionMenuScene` | In-edit buffer; committed to live value and persisted only on OK |
| **Read-Modify-Write** | `SaveManager` | `settings.json` shared by two scenes; each modifies only its own fields |
| **Header-only Utility** | `CatAssets.hpp` | `inline` functions for resource paths; no compilation unit needed |
