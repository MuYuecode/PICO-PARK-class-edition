# Architecture Overview

> Physics system (pipeline, body types, chain pushing) → `PHYSICS_DESIGN.md`.
> This document covers everything else.

## Contents
1. [Entry Point & Main Loop](#1-entry-point--main-loop)
2. [App State Machine](#2-app-state-machine)
3. [GameContext](#3-gamecontext)
4. [Scene System](#4-scene-system)
5. [Rendering Layer](#5-rendering-layer)
6. [UI Components](#6-ui-components)
7. [Input System](#7-input-system)
8. [Settings System](#8-settings-system)
9. [SaveManager](#9-savemanager)
10. [Resource Path Conventions](#10-resource-path-conventions)
11. [Scene Transition Overview](#11-scene-transition-overview)
12. [Extension Guide](#12-extension-guide)

---

## 1. Entry Point & Main Loop

```
main.cpp
└── Core::Context::GetInstance()   (PTSD engine: window / OpenGL init)
└── App app
    └── while (!context->GetExit())
            START  → app.Start()    build all scenes, load persistent settings
            UPDATE → app.Update()   drive current scene each frame
            END    → app.End()      release resources, signal engine exit
```

`context->Update()` flushes the rendered frame to screen.

---

## 2. App State Machine

**Files:** `App.hpp` / `AppStart.cpp` / `AppUpdate.cpp` / `AppEnd.cpp`

```
App::State:  START ──→ UPDATE ──→ END
```

### App Members

| Member | Type | Notes |
|--------|------|-------|
| `m_Ctx` | `unique_ptr<GameContext>` | cross-scene shared data |
| `m_Root` | `Util::Renderer` | global render root |
| `m_TitleScene` … `m_LevelSelectScene` | `unique_ptr<Scene>` | sole owners of each scene |
| `m_CurrentScene` | `Scene*` (non-owning) | active scene |

> **Adding a new scene:** declare `unique_ptr<Scene> m_XxxScene` in `App.hpp`, construct in `AppStart.cpp`, wire pointers via setters, then `std::move` into the member.

### TransitionTo

```cpp
void App::TransitionTo(Scene* next) {
    if (m_CurrentScene) m_CurrentScene->OnExit();
    m_CurrentScene = next;
    if (m_CurrentScene) m_CurrentScene->OnEnter();
}
```

Called by `App::Update()` when `m_CurrentScene->Update()` returns a non-null, different pointer.  
`GameContext::ShouldQuit = true` → `App::Update()` sets state to `END`.

---

## 3. GameContext

**File:** `GameContext.hpp`

```cpp
struct GameContext {
    Util::Renderer& Root;           // borrowed from App, not owned

    // Permanent render-tree objects (added in AppStart, never removed)
    shared_ptr<Character>  Background;  // hot-swappable bg image
    shared_ptr<Character>  Floor;
    shared_ptr<BGMPlayer>  BGMPlayer;
    shared_ptr<Character>  Header;
    shared_ptr<Character>  Door;        // switched open/close by LocalPlayGameScene
    shared_ptr<PushableBox> TestBox;    // demo box on title screen

    // 8 decoration cats shared between title screen and gameplay
    vector<shared_ptr<PlayerCat>> StartupCats;

    // Runtime game data
    int  SelectedPlayerCount  = 2;
    int  CooperativePushPower = 1;
    bool ShouldQuit           = false;

    // index 0–7: blue, red, yellow, green, purple, pink, orange, gray
    static constexpr array<const char*, 8> kCatColorOrder = { … };
};
```

**Rules:**
- Permanent objects are added to the render tree once in `AppStart`; scenes must not remove them (`LevelSelectScene` hides `Header` via `SetVisible`, not `RemoveChild`).
- Cross-scene game state (`SelectedPlayerCount`, `CooperativePushPower`) lives here; scenes must not read each other's members directly.
- `TestBox` is a `PushableBox` used for the title-screen physics demo. It is registered into `TitleScene`'s local `PhysicsWorld` during `TitleScene::OnEnter()`, and un-registered when `m_World.Clear()` is called on exit.

---

## 4. Scene System

### 4.1 Scene Interface (`Scene.hpp`)

```cpp
class Scene {
public:
    virtual void   OnEnter() = 0;  // AddChild + reset state
    virtual void   OnExit()  = 0;  // RemoveChild (must pair with OnEnter)
    virtual Scene* Update()  = 0;  // per-frame logic; return next scene or nullptr
protected:
    GameContext& m_Ctx;
};
```

**Invariant:** every `AddChild` in `OnEnter` must have a matching `RemoveChild` in `OnExit`. Violations cause duplicate render-list entries.

### 4.2 Scene Catalogue

| Scene | Role | Physics | Destinations |
|-------|------|---------|-------------|
| `TitleScene` | title screen, flashing prompt, 8 cats + TestBox | `PhysicsWorld` (floor + walls; cats + TestBox) | MenuScene |
| `MenuScene` | main menu (EXIT / OPTION / LOCAL PLAY) | `PhysicsWorld` (floor + walls; startup cats) | TitleScene, ExitConfirmScene, OptionMenuScene, LocalPlayScene |
| `ExitConfirmScene` | YES/NO quit dialog | none | MenuScene (or ShouldQuit) |
| `OptionMenuScene` | bg color, volume, disp number; saves on OK | none | MenuScene, KeyboardConfigScene |
| `KeyboardConfigScene` | key binding (1P–8P); saves on CommitPending | none | OptionMenuScene |
| `LocalPlayScene` | select player count (2–8) | none | MenuScene, LocalPlayGameScene |
| `LocalPlayGameScene` | multi-player physics, door entry | `PhysicsWorld` (floor + ceiling + walls; player cats) | LocalPlayScene, LevelSelectScene |
| `LevelSelectScene` | 10-level grid, crown marks, best times | none | LocalPlayGameScene (ESC), LevelNScene (ENTER) |
| `LevelOneScene` | actual gameplay level | `PhysicsWorld` (to be implemented) | **not yet implemented** |
| `LevelExitScene` | confirmation before exiting a level | none | **not yet implemented** |

`LevelOneScene` is the scene where the actual cooperative level takes place. `LevelExitScene` is a confirmation dialog (analogous to `ExitConfirmScene`) that appears when players try to exit mid-level. Both are currently empty stubs with no implementation.

### 4.3 PhysicsWorld Lifecycle in Scenes

Scenes that own a `PhysicsWorld` member (`TitleScene`, `MenuScene`,
`LocalPlayGameScene`) follow a strict pattern:

```
OnEnter:
    m_World.Clear()
    Register all dynamic bodies (cats, boxes)
    AddStaticBoundary for floor / ceiling / walls
    (for PushableBox: call box->SetWorld(&m_World) before Register)

Update:
    // 1. input phase: SetMoveDir / Jump on cats
    m_World.Update()   ← single call drives the entire two-phase pipeline

OnExit:
    m_World.Clear()
```

The `PhysicsWorld` internally orders body updates correctly (characters before
boxes) and resolves collisions topologically (supports before riders). Callers
need only provide the input and call `Update()` once.

### 4.4 Shared UI Object Borrowing Pattern

`MenuScene` owns these objects and exposes them via getters:

```
m_MenuFrame, m_ExitGameButton, m_LeftTriButton, m_RightTriButton, m_blue_cat_run_img
```

`AppStart.cpp` injects them via constructor parameters into borrower scenes:

```
GetMenuFrame()       → ExitConfirmScene, LocalPlayScene
GetExitGameButton()  → ExitConfirmScene, OptionMenuScene, KeyboardConfigScene
GetLeftTriButton()   → LocalPlayScene
GetRightTriButton()  → LocalPlayScene
GetBlueCatRunImg()   → LocalPlayScene
```

Borrowers call `AddChild`/`RemoveChild` in `OnEnter`/`OnExit` and restore position/scale on exit.

### 4.5 Scene Pointer Rules

- `App` owns scenes via `unique_ptr` (sole owner).
- Inter-scene references use **raw non-owning pointers**; never `delete`.
- Circular references (A↔B) resolved via `SetXxx(ptr)` setters after both are constructed in `AppStart`.

---

## 5. Rendering Layer

### 5.1 Inheritance

```
Util::GameObject         (PTSD base: m_Drawable, m_Transform, m_ZIndex)
├── Character            static image (Util::Image)
│   ├── UITriangleButton pressed/normal image with timer
│   └── PushableBox      physics box + optional GameText counter label
├── AnimatedCharacter    animation clip (Util::Animation)
│   └── PlayerCat        multi-clip state machine + IPhysicsBody
└── GameText             text (Util::Text), fixed font TerminusTTFWindows-Bold
```

### 5.2 Character (`Character.hpp`)

| Method | Notes |
|--------|-------|
| `SetImage(path)` | hot-swap image (rebuilds Drawable) |
| `GetPosition()` / `SetPosition(vec2)` | `m_Transform.translation` |
| `GetSize()` | scaled pixel size via `GetScaledSize()` |
| `SetScale(vec2)` | negative X = horizontal flip |
| `IsMouseHovering()` / `IsLeftClicked()` | AABB hit-test via `AppUtil` |

### 5.3 AnimatedCharacter (`AnimatedCharacter.hpp`)

| Method | Notes |
|--------|-------|
| `Play()` / `SetLooping(bool)` / `IsPlaying()` | clip control |
| `IfAnimationEnds()` | true when current frame == last frame (non-looping end detection) |

### 5.4 GameText (`GameText.hpp`)

| Method | Notes |
|--------|-------|
| `SetText(string)` / `SetColor(Color)` | dynamic update |
| `GetSize()` | pixel dimensions (used for alignment math) |
| `IsMouseHovering()` / `IsLeftClicked()` | AABB hit-test via `AppUtil` |

### 5.5 ZIndex Convention

| Range | Usage |
|------|-------|
| `-10` | background |
| `0` | floor, header, title-scene subtitle/prompt text |
| `5` | door |
| `10` | menu frame, blue cat image |
| `15` | triangle buttons, `PushableBox` body, startup cats (`15.0 + i*0.01`) |
| `16` | `PushableBox` counter text |
| `20` | Option menu frame, ExitConfirm choice frame |
| `20` | LocalPlay gameplay cats |
| `25` | ExitConfirm texts, KeyboardConfig frame |
| `30` | ExitGameButton, Option/Keyboard choice frame, door count text |
| `32` | LevelSelect selector frame |
| `35` | Option/Keyboard UI texts & buttons, level covers, LevelSelect title/best-time text |
| `36` | LevelSelect crown marks |
| `100` | GameText default (when not explicitly overridden) |

---

## 6. UI Components

### UITriangleButton (`UITriangleButton.hpp`)

Extends `Character` with two-image pressed/normal state and timer.

```
Normal ──Press(ms)──→ Pressed ──timer expires──→ Normal
```

| Method | Notes |
|--------|-------|
| `Press(ms=75)` | switch to pressed image; auto-reverts after `ms` ms |
| `UpdateButton()` | **call every frame** to tick the timer |
| `ResetState()` | force back to normal (call on scene enter) |

---

## 7. Input System

**Source:** PTSD `Util/Input.hpp`

| Function | Semantics |
|----------|-----------|
| `IsKeyDown(key)` | pressed **this frame only** |
| `IsKeyPressed(key)` | held down (true every frame) |
| `GetCursorPosition()` | mouse in screen space (Y-up) |

**Convention:** use `IsKeyDown` for menu navigation / jump / confirm; `IsKeyPressed` for continuous horizontal movement.

### AppUtil (`AppUtil.hpp`)

| Function | Notes |
|----------|-------|
| `AlignLeft(text, boundX)` / `AlignRight(...)` | center-X for left/right alignment |
| `KeycodeToString(key)` | `UNKNOWN` → `"-"` |
| `GetAnyKeyDown()` | returns first bindable key pressed this frame (for KeyboardConfigScene capture) |
| `IsMouseHovering(obj)` / `IsLeftClicked(obj)` | AABB test on any `Util::GameObject` |

---

## 8. Settings System

### PlayerKeyConfig (`KeyboardConfigScene.hpp`)

```cpp
struct PlayerKeyConfig {
    Util::Keycode up, down, left, right, jump, cancel, shot;
    Util::Keycode menu;     // 1P only
    Util::Keycode subMenu;  // 1P only
};
```

**Defaults:**
- `k_Default1P`: W/S/A/D move, W jump, ESC cancel, SPACE shot, ENTER menu, TAB submenu
- `k_Default2P`: arrow keys move, UP jump, AC_BACK cancel, R_CTRL shot
- 3P–8P: all `UNKNOWN` (must be configured manually)

### KeyboardConfigScene

- Constructor loads `settings.json` via `SaveManager::LoadKeyConfigs()`.
- `m_Applied[8]` = committed configs; `m_Pending` = in-edit buffer.
- `CommitPending()` writes `m_Applied[m_CurrentPlayer]` and calls `SaveManager::SaveKeyConfigs()`.
- `GetAppliedConfig(i)` → used by `LocalPlayGameScene::SpawnPlayers()`.
- `GetConfiguredPlayerCount()` → returns count of players with ≥4 non-UNKNOWN keys; used by `LocalPlayScene` to block entry.
- `menu` and `subMenu` rows are **not selectable** for players other than 1P.

### OptionMenuScene::Settings

```cpp
struct Settings {
    int  bgColorIndex = 0;   // WHITE / CREAM / DARK
    int  bgmVolume    = 10;  // 0–20, multiplied by 6 when passed to BGMPlayer
    int  seVolume     = 10;  // 0–20 (SE system not yet implemented)
    bool dispNumber   = false;
};
```

- `m_Applied` = live values; `m_Pending` = in-edit buffer; OK commits and persists, CANCEL discards.

---

## 9. SaveManager

**Files:** `SaveManager.hpp` / `SaveManager.cpp`

Pure-static utility class. Uses **nlohmann/json** (bundled with PTSD).

### File Locations

| File | Contents |
|------|---------|
| `GA_RESOURCE_DIR/Save/settings.json` | option settings + 8-player key bindings |
| `GA_RESOURCE_DIR/Save/save_data.json` | 10-level completion state and best times |

`Save/` is auto-created on first write via `std::filesystem::create_directories`.

### settings.json Schema

```json
{
  "bgColorIndex": 0, "bgmVolume": 10, "seVolume": 10, "dispNumber": false,
  "keyConfigs": [
    { "up": 26, "down": 22, "left": 4, "right": 7,
      "jump": 26, "cancel": 41, "shot": 44, "menu": 40, "subMenu": 43 },
    …
  ]
}
```

Keys stored as `int` (SDL_Scancode; `UNKNOWN = 0`).

### save_data.json Schema

```json
{
  "levels": [
    { "levelId": 1, "completed": true,
      "bestTimes": { "2": 45.3, "3": -1.0, … "8": -1.0 } }
  ]
}
```

`bestTimes` key is player count (2–8); `-1.0` = not yet cleared.

### Read-Modify-Write Strategy

Both `SaveOptionSettings` and `SaveKeyConfigs` share `settings.json`. Each reads the file, modifies only its own fields, then writes back — preventing one scene from overwriting the other's data.

### API

| Function | Notes |
|----------|-------|
| `SaveOptionSettings(opts)` / `LoadOptionSettings(out)` | option fields only |
| `SaveKeyConfigs(keys)` / `LoadKeyConfigs(out)` | keyConfigs array only |
| `SaveLevelData(levels)` / `LoadLevelData(out)` | entire save_data.json |
| `UpdateBestTime(idx, playerCount, elapsed)` | load → compare → save if new record |
| `FormatTime(seconds)` | `-1` → `"--:--.--"`, else `"mm:ss.cs"` |

---

## 10. Resource Path Conventions

Root macro: `GA_RESOURCE_DIR` (CMake-defined, points to `resources/`).

```
resources/
├── BGM/          ppc.mp3  pp1.mp3  pp2.mp3
├── Font/         TerminusTTFWindows-Bold-4.49.3.ttf  (primary font)
│                 FORCEDSQUARE.ttf  Monocraft.ttf
├── Image/
│   ├── Background/
│   │   ├── white_background.png / cream_background.png / dark_background.png
│   │   ├── background_floor.png / header.png
│   │   ├── background_lwall.png / background_rwall.png  (decorative; physics = StaticBody)
│   │   ├── door_close.png / door_open.png
│   │   ├── level_select_frame.png      ← LevelSelectScene selector frame
│   │   ├── Menu_Frame.png / Choice_Frame.png
│   │   ├── Option_Menu_Frame.png / Option_Choice_Frame.png / Option_HLine.png
│   │   └── Keyboard_Config_Menu_Frame.png
│   ├── Button/
│   │   ├── ExitGameButton.png
│   │   ├── Left_Tri_Button.png / Left_Tri_Button_Full.png
│   │   └── Right_Tri_Button.png / Right_Tri_Button_Full.png
│   ├── Character/{color}_cat/
│   │   ├── {color}_cat_stand_1–8.png   (8 frames, 400ms interval, looping)
│   │   ├── {color}_cat_run_1–9.png     (9 frames, 120ms, looping)
│   │   ├── {color}_cat_jump_1.png      (rise; 150ms, looping)
│   │   ├── {color}_cat_jump_2.png      (fall; 150ms, looping)
│   │   ├── {color}_cat_land_1.png      (80ms, non-looping)
│   │   └── {color}_cat_push_1–3.png    (3 frames, 120ms, looping)
│   └── Level_Cover/
│       ├── LevelOne–LevelFour.png  Crown.png  02.png  ExitFrame.png  Key.png
│       └── LevelOneScene/  Box.png  Frame.png  LevelOneScene.png
└── Save/         settings.json  save_data.json
```

**Cat color order** (`GameContext::kCatColorOrder`, index 0–7):
blue · red · yellow · green · purple · pink · orange · gray

### CatAssets (`CatAssets.hpp` — header-only)

| Function | Notes |
|----------|-------|
| `BuildFramePath(color, action, frameNum)` | single frame path |
| `BuildFramePaths(color, action, n)` | frames 1..n |
| `BuildFullAnimPaths(color)` | complete `CatAnimPaths` struct |

---

## 11. Scene Transition Overview

```
AppStart
    ↓
TitleScene ──ENTER──→ MenuScene ──A/D──→ ExitConfirmScene ──YES──→ ShouldQuit
                          │                    │
                          │               ESC/NO──→ MenuScene
                          │
                          ├──→ OptionMenuScene ──row0/mouse──→ KeyboardConfigScene
                          │         │                                   │
                          │    OK/ESC/CANCEL ←────────────────────────-┘
                          │
                          └──→ LocalPlayScene ──ENTER(count≤configured)──→ LocalPlayGameScene
                                   │                                              │
                              ESC/X←──────────────────────  all entered──→ LevelSelectScene
                              → MenuScene                                         │
                                                               ENTER/click──→ LevelOneScene (TODO)
                                                               ESC──────────→ LocalPlayGameScene

LevelOneScene (TODO) ──completion──→ LevelExitScene (TODO) ──confirm──→ LevelSelectScene
                      ──ESC / abort──→ LevelExitScene (TODO)
```

---

## 12. Extension Guide

### Add a Level Scene (e.g. `LevelOneScene`)

`LevelOneScene` and `LevelExitScene` are declared but contain no implementation
yet. When ready:

1. Implement `LevelOneScene` in `include/LevelOneScene.hpp` + `src/LevelOneScene.cpp`,
   inheriting `Scene`. Give it a `PhysicsWorld m_World` member.
2. In `OnEnter`: clear the world, spawn cats + boxes (with `box->SetWorld(&m_World)`),
   call `SetupStaticBoundaries()` which uses `m_World.AddStaticBoundary()` for
   floor, ceiling, and all wall geometry.
3. In `Update`: read input → `SetMoveDir`/`Jump` on cats → `m_World.Update()`.
4. On completion: call `SaveManager::UpdateBestTime(levelIdx, playerCount, elapsed)`,
   then transition to `LevelExitScene` (the level-exit confirmation).
5. Implement `LevelExitScene` similarly to `ExitConfirmScene` (YES/NO pattern).
6. In `AppStart.cpp`, construct both scenes, wire them via `LevelSelectScene`'s
   currently-commented-out `m_LevelScenes[0]` slot.

### Add Wall Geometry in a Level

All in-level walls that should block movement must be registered as `StaticBody`
objects via `m_World.AddStaticBoundary(center, halfSize)`. The visual wall
sprite (`Character`) is added to the renderer separately; it is purely decorative.
The `StaticBody` has `IsSolid() = true` and `IsKinematic() = true`, so all
dynamic bodies (cats, boxes) are resolved against it via AABB separation.

```cpp
// Example: a narrow vertical wall at x = 200, spanning y = -200 to +200
m_World.AddStaticBoundary({200.f, 0.f}, {10.f, 200.f});
```

### Add a PushableBox in a Level

```cpp
auto box = std::make_shared<PushableBox>(
    GA_RESOURCE_DIR "/Image/Level_Cover/LevelOneScene/Box.png",
    /*requiredPushers=*/2);
box->SetPosition({x, y});
box->SetZIndex(15.f);
box->SetWorld(&m_World);          // must precede Register

m_World.Register(box);
m_Ctx.Root.AddChild(box);
if (box->GetTextObject()) {
    box->GetTextObject()->SetZIndex(16.f);
    m_Ctx.Root.AddChild(box->GetTextObject());
}
```

On `OnExit`, remove both the box and its text from `m_Ctx.Root`, then call
`m_World.Clear()`.

### Add a Global Setting

1. Add field to `OptionMenuScene::Settings` and `OptionSettingsData` in `SaveManager.hpp`.
2. Build corresponding UI row in `OptionMenuScene` (follow BG_COLOR pattern).
3. Update `SaveManager::SaveOptionSettings` / `LoadOptionSettings`.
4. If the setting must be cross-scene, add a member to `GameContext` and sync in `OptionMenuScene` OK branch.

### Add a Cat Color

1. Create `resources/Image/Character/{color}_cat/` with all animation frames.
2. Append the color string to `GameContext::kCatColorOrder`.
3. Adjust `LocalPlayScene::MAX_PLAYERS` if supporting more than 8 players.
