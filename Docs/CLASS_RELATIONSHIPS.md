# 類別關係文件

> 本文件專注於各 `.hpp` / `.cpp` 之間的 **OOP 結構關係**（繼承、組合、依賴）。  
> 渲染層的各類別功能說明已在 `ARCHITECTURE.md § 5` 說明。  
> 物理介面 `IPhysicsBody` 及各物理物件的骨架已在 `PHYSICS_DESIGN.md` 說明。

---

## 目錄

1. [繼承體系總覽](#1-繼承體系總覽)
2. [渲染物件繼承鏈](#2-渲染物件繼承鏈)
3. [Scene 抽象類別與具體場景](#3-scene-抽象類別與具體場景)
4. [物理系統的組合結構](#4-物理系統的組合結構)
5. [跨檔案依賴關係圖](#5-跨檔案依賴關係圖)
6. [OOP 設計模式索引](#6-oop-設計模式索引)

---

## 1. 繼承體系總覽

```
Util::GameObject          （PTSD 引擎 — 所有可渲染物件的根）
│
├── Character              Character.hpp / Character.cpp
│   └── UI_Triangle_Button UI_Triangle_Button.hpp / .cpp
│
└── AnimatedCharacter      AnimatedCharacter.hpp / AnimatedCharacter.cpp
    └── PlayerCat          PlayerCat.hpp / PlayerCat.cpp
        └── [待實作]
            IPhysicsBody   IPhysicsBody.hpp         ← 介面，多重繼承

Scene                      Scene.hpp                ← 純虛擬抽象類別
├── TitleScene             Titlescene.hpp / .cpp
├── MenuScene              Menuscene.hpp / .cpp
├── ExitConfirmScene       Exitconfirmscene.hpp / .cpp
├── OptionMenuScene        OptionMenuScene.hpp / .cpp
├── KeyboardConfigScene    KeyboardConfigScene.hpp / .cpp
├── LocalPlayScene         LocalPlayScene.hpp / .cpp
└── LocalPlayGameScene     LocalPlayGameScene.hpp / .cpp
```

---

## 2. 渲染物件繼承鏈

> 各類別的**方法列表**已在 `ARCHITECTURE.md § 5` 說明，此處只描述繼承關係的設計意圖。

### 2.1 `Character`（`Character.hpp`）

- **IS-A** `Util::GameObject`：繼承引擎提供的 `m_Transform`、`m_ZIndex`、`m_Drawable`。
- 職責邊界：封裝**靜態圖片**的顯示與 AABB 互動偵測，不含動畫邏輯。
- 作為其他圖片型 UI 元件的基底（見 `UI_Triangle_Button`）。

### 2.2 `UI_Triangle_Button`（`UI_Triangle_Button.hpp`）

- **IS-A** `Character`：繼承圖片顯示與滑鼠偵測。
- **擴充**：新增「按下 / 正常」兩張圖片的切換邏輯與計時器。
- **使用 `Character::SetImage()`** 熱替換圖片來實作視覺回饋，不需要重新建立物件。

### 2.3 `AnimatedCharacter`（`AnimatedCharacter.hpp`）

- **IS-A** `Util::GameObject`：與 `Character` 平行繼承，但 `m_Drawable` 改為 `Util::Animation`。
- 職責邊界：封裝**單一動畫 clip** 的播放控制，不含多 clip 切換邏輯。
- 不繼承 `Character`，因為兩者的 `Drawable` 型別不同，強行繼承會破壞 Liskov 替換原則。

### 2.4 `PlayerCat`（`PlayerCat.hpp`）

- **IS-A** `AnimatedCharacter`：繼承動畫播放機制。
- **擴充**：持有多組動畫 clip（stand / run / jump_rise / jump_fall / land / push），實作多動畫狀態機。
- **待實作 IS-A** `IPhysicsBody`（多重繼承）：讓 `PhysicsWorld` 能以統一介面管理；見 `PHYSICS_DESIGN.md § 1`。

```
// 多重繼承示意（待實作）
class PlayerCat : public AnimatedCharacter, public IPhysicsBody { ... };
//                      ↑ 渲染能力              ↑ 物理介面
```

### 2.5 `GameText`（`GameText.hpp`）

- **IS-A** `Util::GameObject`：`m_Drawable` 為 `Util::Text`。
- 與 `Character` / `AnimatedCharacter` 平行，三者都是 `GameObject` 的具體化，只是 `Drawable` 型別不同。
- 此部分已在 `ARCHITECTURE.md § 5.4` 說明。

---

## 3. Scene 抽象類別與具體場景

### 3.1 `Scene`（`Scene.hpp`）

```cpp
class Scene {
public:
    virtual void   OnEnter() = 0;   // 純虛擬
    virtual void   OnExit()  = 0;   // 純虛擬
    virtual Scene* Update()  = 0;   // 純虛擬
protected:
    GameContext& m_Ctx;             // 所有子類別共用
};
```

- **抽象基底類別（ABC）**：三個純虛擬函式強制子類別實作完整的生命週期介面。
- 子類別無法跳過 `OnExit` 不實作，編譯器會報錯——這是 ABC 相較於空虛擬函式的優勢。
- `m_Ctx` 以 **reference** 持有，保證非空且不可重新指向，明確表達「借用不擁有」的語意。

### 3.2 具體場景的依賴關係

場景之間以 **raw pointer（non-owning）** 互相引用，所有權集中在 `App`。

```
App（unique_ptr 擁有者）
│
├── TitleScene ──────────────────────→ MenuScene*
│
├── MenuScene ───────────────────────→ TitleScene*
│                                   → ExitConfirmScene*
│                                   → OptionMenuScene*
│                                   → LocalPlayScene*
│
├── ExitConfirmScene ────────────────→ MenuScene*
│
├── OptionMenuScene ─────────────────→ MenuScene*
│                                   → KeyboardConfigScene*
│
├── KeyboardConfigScene ─────────────→ OptionMenuScene*
│
├── LocalPlayScene ──────────────────→ MenuScene*
│                                   → LocalPlayGameScene*
│
└── LocalPlayGameScene ──────────────→ LocalPlayScene*
                                    → KeyboardConfigScene*（讀取按鍵設定）
```

`LocalPlayGameScene` 額外依賴 `KeyboardConfigScene`（唯讀，取得已設定的按鍵綁定），但不負責切換到它。

### 3.3 借用模式（MenuScene 共用 UI 物件）

`MenuScene` **擁有**四個 UI 物件（`unique_ptr` 語意的 `shared_ptr`），子場景以 getter **借用**：

```
MenuScene（擁有者）         子場景（借用者，不擁有）
─────────────────────────  ──────────────────────────────────────
m_MenuFrame            →   ExitConfirmScene, LocalPlayScene
m_ExitGameButton       →   ExitConfirmScene, OptionMenuScene, KeyboardConfigScene
m_LeftTriButton        →   LocalPlayScene
m_RightTriButton       →   LocalPlayScene
```

- 借用者在 `OnEnter` 加入渲染樹，`OnExit` 移除，**不 `delete`、不 `reset()`**。
- 這是 **組合優於繼承** 的實踐：子場景不繼承 `MenuScene`，而是在需要時借用其資源。

---

## 4. 物理系統的組合結構

### 4.1 結構圖

```
LocalPlayGameScene
│
├── HAS-A  CharacterPhysicsSystem   m_Physics    （純算法，無狀態）
│
├── HAS-A  vector<PlayerBinding>    m_Players
│              │
│              └── PlayerBinding
│                    ├── PhysicsAgent  agent
│                    │     ├── shared_ptr<PlayerCat>  actor
│                    │     └── PhysicsState            state
│                    └── PlayerKeyConfig               key
│
└── 透過 GameContext::Floor（shared_ptr<Character>）取得地板參考
```

### 4.2 `PhysicsState`（`CharacterPhysicsSystem.hpp`）

純資料結構（POD-like），無方法，無繼承。

```cpp
struct PhysicsState {
    float velocityY;       // 垂直速度
    float lastDeltaX;      // 上幀水平位移（攜帶計算用）
    int   supportIndex;    // 站在哪隻角色上（-1 = 無）
    int   moveDir;         // 輸入方向（-1/0/+1）
    bool  grounded;        // 是否接地
    bool  prevGrounded;    // 上幀接地狀態（落地偵測）
    bool  beingStoodOn;    // 有人踩在自己頭上（禁止跳躍用）
};
```

- **Data Class 模式**：只負責攜帶狀態，邏輯全部在 `CharacterPhysicsSystem`。
- Scene 持有 `PhysicsState`，而非讓 `PlayerCat` 自己持有——這將**渲染物件**和**物理狀態**分離，符合單一責任原則。

### 4.3 `PhysicsAgent`（`CharacterPhysicsSystem.hpp`）

```cpp
struct PhysicsAgent {
    shared_ptr<PlayerCat> actor;  // 渲染與視覺
    PhysicsState          state;  // 物理狀態
};
```

- **Data Transfer Object（DTO）**：將 actor 與 state 打包，讓 `CharacterPhysicsSystem::Update` 能以統一視角處理所有角色。
- `CharacterPhysicsSystem` 接收 `vector<PhysicsAgent>&` 而非直接存取 `m_Players`，保持系統與場景的解耦。

### 4.4 `CharacterPhysicsSystem`（`CharacterPhysicsSystem.hpp`）

```cpp
class CharacterPhysicsSystem {
public:
    void  Update(vector<PhysicsAgent>&, const shared_ptr<Character>& floor) const;
    float ResolveHorizontal(int idx, float targetX,
                            const vector<PhysicsAgent>&) const;
private:
    // ... 全部 const 方法，無成員變數
};
```

- **Stateless Service（無狀態服務）**：所有方法均為 `const`，不持有任何執行期狀態。
- 多個 Scene（`LocalPlayGameScene`、未來的 `LevelScene`）可共用**同一個 System 類別**而不相互干擾。
- 與 `PhysicsWorld` 的分工：`CharacterPhysicsSystem` 只處理 CHARACTER 間的細緻推擠與踩頭；跨類型互動（角色 vs 箱子 vs 敵人）由 `PhysicsWorld` 負責。詳見 `PHYSICS_DESIGN.md`。

---

## 5. 跨檔案依賴關係圖

以下為**編譯依賴**（`#include`）的主要方向，箭頭表示「依賴」。

```
main.cpp
    └── App.hpp
            ├── Scene.hpp ──────── GameContext.hpp ─── Character.hpp
            │                                       └── PlayerCat.hpp
            ├── TitleScene / MenuScene / ... （各 Scene 子類別）
            │       └── 都 #include Scene.hpp
            │
            └── （透過 GameContext）共用 Character.hpp、PlayerCat.hpp

CharacterPhysicsSystem.hpp
    ├── PlayerCat.hpp
    └── Character.hpp

LocalPlayGameScene.hpp
    ├── CharacterPhysicsSystem.hpp
    ├── KeyboardConfigScene.hpp    （取得按鍵設定）
    └── Scene.hpp

PlayerCat.hpp
    └── AnimatedCharacter.hpp
            └── （PTSD Util::Animation）

Character.hpp
    └── （PTSD Util::Image）
```

**設計原則**：
- `CharacterPhysicsSystem` **不** `#include` 任何 Scene，保持單向依賴。
- `GameContext` 只 `#include` 它需要的具體物件型別（`Character`、`PlayerCat`），不依賴 Scene。
- Scene 依賴 `GameContext` 而非直接依賴其他 Scene 的完整 header（只持有 raw pointer，可用前向宣告）。

---

## 6. OOP 設計模式索引

| 模式 | 應用位置 | 說明 |
|------|----------|------|
| **Template Method** | `Scene` ABC | `OnEnter` / `OnExit` / `Update` 三步驟固定，子類別填入細節 |
| **State Machine** | `App::State`、`PlayerCat` 動畫狀態 | App 用 enum 狀態機切換 START/UPDATE/END；PlayerCat 用 `CatAnimState` 切換動畫 clip |
| **Stateless Service** | `CharacterPhysicsSystem` | 無成員變數的純算法類別，可被多個 Scene 複用 |
| **Data Transfer Object** | `PhysicsAgent`、`PhysicsState` | 打包資料跨越 Scene ↔ System 邊界 |
| **Borrowing（借用模式）** | MenuScene 共用 UI 物件 | `shared_ptr` 擁有者固定，借用者在 OnEnter/OnExit 配對使用 |
| **Dependency Injection** | `PushableBox::SetWorld()`、Scene 建構子傳入 raw pointer | 避免全域存取，依賴由外部注入 |
| **Abstract Base Class** | `Scene`、`IPhysicsBody` | 強制子類別實作完整介面，編譯期保證 |
| **Composition over Inheritance** | 子場景借用 MenuScene 的 UI，而非繼承 MenuScene | 避免深層繼承樹，降低耦合 |
