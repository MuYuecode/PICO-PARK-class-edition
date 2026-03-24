# 類別關係文件

> 本文件專注於各 `.hpp` / `.cpp` 之間的 **OOP 結構關係**(繼承、組合、依賴)   
> 渲染層的各類別功能說明已在 `ARCHITECTURE.md § 5` 說明   
> 物理介面 `IPhysicsBody` 及各物理物件的骨架已在 `PHYSICS_DESIGN.md` 說明 

---

## 目錄

1. [繼承體系總覽](#1-繼承體系總覽)
2. [渲染物件繼承鏈](#2-渲染物件繼承鏈)
3. [Scene 抽象類別與具體場景](#3-scene-抽象類別與具體場景)
4. [物理系統的組合結構](#4-物理系統的組合結構)
5. [持久化系統(SaveManager)](#5-持久化系統savemanager)
6. [跨檔案依賴關係圖](#6-跨檔案依賴關係圖)
7. [OOP 設計模式索引](#7-oop-設計模式索引)

---

## 1. 繼承體系總覽

```
Util::GameObject          (PTSD 引擎 — 所有可渲染物件的根)
│
├── Character              Character.hpp / Character.cpp
│   └── UI_Triangle_Button UI_Triangle_Button.hpp / .cpp
│
├── AnimatedCharacter      AnimatedCharacter.hpp / AnimatedCharacter.cpp
│   └── PlayerCat          PlayerCat.hpp / PlayerCat.cpp
│           └── IPhysicsBody   IPhysicsBody.hpp    ← 已實作多重繼承
│
└── GameText               GameText.hpp / GameText.cpp

Scene                      Scene.hpp                ← 純虛擬抽象類別
├── TitleScene             Titlescene.hpp / .cpp
├── MenuScene              Menuscene.hpp / .cpp
├── ExitConfirmScene       Exitconfirmscene.hpp / .cpp
├── OptionMenuScene        OptionMenuScene.hpp / .cpp
├── KeyboardConfigScene    KeyboardConfigScene.hpp / .cpp
├── LocalPlayScene         LocalPlayScene.hpp / .cpp
├── LocalPlayGameScene     LocalPlayGameScene.hpp / .cpp
└── LevelSelectScene       LevelSelectScene.hpp / .cpp

SaveManager                SaveManager.hpp / .cpp   ← 純靜態工具類別(不繼承任何類)
```

---

## 2. 渲染物件繼承鏈

> 各類別的**方法列表**已在 `ARCHITECTURE.md § 5` 說明，此處只描述繼承關係的設計意圖 

### 2.1 `Character`(`Character.hpp`)

- **IS-A** `Util::GameObject`：繼承引擎提供的 `m_Transform`、`m_ZIndex`、`m_Drawable` 
- 職責邊界：封裝**靜態圖片**的顯示與 AABB 互動偵測，不含動畫邏輯 
- 作為其他圖片型 UI 元件的基底(見 `UI_Triangle_Button`) 

### 2.2 `UI_Triangle_Button`(`UI_Triangle_Button.hpp`)

- **IS-A** `Character`：繼承圖片顯示與滑鼠偵測 
- **擴充**：新增「按下 / 正常」兩張圖片的切換邏輯與計時器(`m_PressTimer`) 
- **使用 `Character::SetImage()`** 熱替換圖片來實作視覺回饋，不需要重新建立物件 

### 2.3 `AnimatedCharacter`(`AnimatedCharacter.hpp`)

- **IS-A** `Util::GameObject`：與 `Character` 平行繼承，但 `m_Drawable` 改為 `Util::Animation` 
- 職責邊界：封裝**單一動畫 clip** 的播放控制，不含多 clip 切換邏輯 
- 不繼承 `Character`，因為兩者的 `Drawable` 型別不同，強行繼承會破壞 Liskov 替換原則 

### 2.4 `PlayerCat`(`PlayerCat.hpp`)

- **IS-A** `AnimatedCharacter`：繼承動畫播放機制 
- **IS-A** `IPhysicsBody`(多重繼承，**已完整實作**)：讓 `PhysicsWorld` 能以統一介面管理 
- **擴充**：持有六組動畫 clip(stand / run / jump_rise / jump_fall / land / push)，實作多動畫狀態機 

```cpp
// 多重繼承(已實作)
class PlayerCat : public AnimatedCharacter, public IPhysicsBody { ... };
//                      ↑ 渲染能力              ↑ 物理介面
```

### 2.5 `GameText`(`GameText.hpp`)

- **IS-A** `Util::GameObject`：`m_Drawable` 為 `Util::Text` 
- 與 `Character` / `AnimatedCharacter` 平行，三者都是 `GameObject` 的具體化，只是 `Drawable` 型別不同 

---

## 3. Scene 抽象類別與具體場景

### 3.1 `Scene`(`Scene.hpp`)

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

- **抽象基底類別(ABC)**：三個純虛擬函式強制子類別實作完整的生命週期介面 
- `m_Ctx` 以 **reference** 持有，保證非空且不可重新指向，明確表達「借用不擁有」的語意 

### 3.2 具體場景的依賴關係

場景之間以 **raw pointer(non-owning)** 互相引用，所有權集中在 `App` 

```
App(unique_ptr 擁有者)
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
│                                   → KeyboardConfigScene*(讀取 configuredCount)
│                                   → LocalPlayGameScene*
│
├── LocalPlayGameScene ──────────────→ LocalPlayScene*(ESC 返回)
│                                   → KeyboardConfigScene*(讀取按鍵設定)
│                                   → LevelSelectScene*(全員進門後切換)
│
└── LevelSelectScene ────────────────→ LocalPlayGameScene*(ESC 返回)
                                    → LevelNScene*[0..9](ENTER 進入關卡，可為 nullptr)
```

### 3.3 借用模式(MenuScene 共用 UI 物件)

`MenuScene` **擁有**五個 UI 物件(`shared_ptr` 語意)，子場景以 getter **借用**：

```
MenuScene(擁有者)           子場景(借用者，不擁有)
─────────────────────────    ──────────────────────────────────────
m_MenuFrame              →   ExitConfirmScene, LocalPlayScene
m_ExitGameButton         →   ExitConfirmScene, OptionMenuScene, KeyboardConfigScene
m_LeftTriButton          →   LocalPlayScene
m_RightTriButton         →   LocalPlayScene
m_blue_cat_run_img       →   LocalPlayScene
```

- 借用者在 `OnEnter` 加入渲染樹，`OnExit` 移除，**不 `delete`、不 `reset()`** 
- 借用結束(`OnExit`)後需還原 position / scale，避免下次借用時繼承到修改後的值 

### 3.4 LevelSelectScene 的 UI 物件所有權

`LevelSelectScene` **自行建立並擁有**所有 UI 物件，不借用其他場景的物件：

```
LevelSelectScene(擁有者)
├── m_BgFrame          ← level_select_frame.png
├── m_SelectorFrame    ← Option_Choice_Frame.png(scale 調整為方形)
├── m_TitleText        ← "LEVEL N"
├── m_BestTimeText     ← "m PLAYERS BEST TIME: mm:ss.cs"
├── m_LevelTexts[10]   ← 10 個關卡數字
└── m_CrownTexts[10]   ← 10 個皇冠標記
```

---

## 4. 物理系統的組合結構

### 4.1 結構圖

```
LocalPlayGameScene
│
├── (直接呼叫靜態方法) CharacterPhysicsSystem::Update  (純靜態算法，無需依賴實例)
│
├── HAS-A  vector<PlayerBinding>    m_Players
│              │
│              └── PlayerBinding
│                    ├── PhysicsAgent  agent
│                    │     ├── shared_ptr<PlayerCat>  actor
│                    │     └── PhysicsState            state
│                    ├── PlayerKeyConfig               key
│                    └── bool                          entered(是否已進門)
│
└── 透過 GameContext::Floor(shared_ptr<Character>)取得地板參考
    透過 GameContext::Door(shared_ptr<Character>)控制門的圖片
```

### 4.2 `PhysicsState`(`CharacterPhysicsSystem.hpp`)

純資料結構(POD-like)，無方法，無繼承 

```cpp
struct PhysicsState {
    float velocityY;       // 垂直速度
    float lastDeltaX;      // 上幀水平位移(攜帶計算用)
    int   supportIndex;    // 站在哪隻角色上(-1 = 無)
    int   moveDir;         // 輸入方向(-1/0/+1)
    bool  grounded;        // 是否接地
    bool  prevGrounded;    // 上幀接地狀態(落地偵測)
    bool  beingStoodOn;    // 有人踩在自己頭上(下幀跳躍判斷用)
};
```

### 4.3 `PhysicsAgent`(`CharacterPhysicsSystem.hpp`)

```cpp
struct PhysicsAgent {
    shared_ptr<PlayerCat> actor;  // 渲染與視覺
    PhysicsState          state;  // 物理狀態
};
```

- **Data Transfer Object(DTO)**：將 actor 與 state 打包，讓 `CharacterPhysicsSystem::Update` 能以統一視角處理所有角色 

### 4.4 `CharacterPhysicsSystem`(`CharacterPhysicsSystem.hpp`)

```cpp
class CharacterPhysicsSystem {
public:
    // 所有方法均為 static，無成員變數
    static void  Update(vector<PhysicsAgent>&, const shared_ptr<Character>& floor);
    static float ResolveHorizontal(int idx, float targetX,
                                   const vector<PhysicsAgent>&);
    static void  ApplyJump(PhysicsState& state);
    // ...
};
```

- **Stateless Service(純靜態服務)**：所有方法均為 `static`，不持有任何執行期狀態 

---

## 5. 持久化系統(SaveManager)

### 5.1 類別結構

```
SaveManager                SaveManager.hpp / .cpp
│
├── 無繼承、無成員變數(純靜態工具)
│
├── 依賴：nlohmann::json(PTSD 引擎透過 FetchContent 提供)
├── 依賴：std::filesystem(C++17 標準庫)
│
├── 資料結構(儲存用 POD-like struct)：
│   ├── OptionSettingsData    bgColorIndex, bgmVolume, seVolume, dispNumber
│   ├── KeyConfigData         up/down/left/right/jump/cancel/shot/menu/subMenu(int)
│   └── LevelSaveData         completed(bool), bestTimes[9](float)
│
└── 靜態方法分組：
    ├── settings.json 組：SaveOptionSettings / LoadOptionSettings
    │                     SaveKeyConfigs / LoadKeyConfigs
    └── save_data.json 組：SaveLevelData / LoadLevelData / UpdateBestTime
                          FormatTime(格式化工具)
```

### 5.2 與場景的關係

| 場景 | 呼叫 SaveManager 的時機 | 讀/寫 |
|------|------------------------|-------|
| `OptionMenuScene` 建構子 | 讀取 Option 設定還原 `m_Applied` | 讀 |
| `OptionMenuScene::Update()` OK 確認 | 寫入 Option 設定 | 寫 |
| `KeyboardConfigScene` 建構子 | 讀取按鍵綁定還原 `m_Applied[]` | 讀 |
| `KeyboardConfigScene::CommitPending()` | 寫入按鍵綁定 | 寫 |
| `LevelSelectScene::OnEnter()` | 讀取關卡存檔更新格子狀態 | 讀 |
| 各關卡場景完成時 | `UpdateBestTime()` 更新最佳時間 | 讀+寫 |

### 5.3 OptionSettingsData 與 OptionMenuScene::Settings 的關係

兩個結構欄位相同，但刻意**分開定義**以避免 `SaveManager.hpp` 依賴 `OptionMenuScene.hpp`，形成循環包含：

```
OptionMenuScene::Settings  ← 場景使用的工作用結構(含業務邏輯)
OptionSettingsData          ← SaveManager 使用的純資料傳輸結構(無業務邏輯)
```

轉換發生在 `OptionMenuScene::Update()` 的 OK 分支(直接賦值，欄位一一對應) 

---

## 6. 跨檔案依賴關係圖

以下為**編譯依賴**(`#include`)的主要方向，箭頭表示「依賴」 

```
main.cpp
    └── App.hpp
            ├── Scene.hpp ──────── GameContext.hpp ─── Character.hpp
            │                                       ├── PlayerCat.hpp
            │                                       └── BGMPlayer.hpp
            ├── TitleScene / MenuScene / ... (各 Scene 子類別)
            │       └── 都 #include Scene.hpp
            │
            └── (透過 GameContext)共用 Character.hpp、PlayerCat.hpp

CharacterPhysicsSystem.hpp
    ├── PlayerCat.hpp
    └── Character.hpp

LocalPlayGameScene.hpp
    ├── CharacterPhysicsSystem.hpp
    ├── KeyboardConfigScene.hpp    (取得按鍵設定)
    └── Scene.hpp

LevelSelectScene.hpp
    ├── Scene.hpp
    ├── Character.hpp
    ├── GameText.hpp
    └── SaveManager.hpp            (讀取關卡存檔)

SaveManager.hpp
    └── (無遊戲類別依賴，只依賴 std:: 與 nlohmann/json)

OptionMenuScene.cpp / KeyboardConfigScene.cpp
    └── SaveManager.hpp            (讀寫 settings.json)

PlayerCat.hpp
    ├── AnimatedCharacter.hpp
    └── IPhysicsBody.hpp           ← 多重繼承

CatAssets.hpp                      (header-only，工具函式)
    └── PlayerCat.hpp              (使用 CatAnimPaths)

AppUtil.hpp
    ├── Character.hpp
    ├── GameText.hpp
    └── Util/Keycode.hpp
```

**設計原則**：
- `SaveManager` **不** `#include` 任何 Scene 或遊戲物件 header，保持單向依賴 
- `CharacterPhysicsSystem` **不** `#include` 任何 Scene，保持單向依賴 
- `GameContext` 只 `#include` 它需要的具體物件型別(`Character`、`PlayerCat`、`BGMPlayer`)，不依賴 Scene 
- Scene 依賴 `GameContext` 而非直接依賴其他 Scene 的完整 header(只持有 raw pointer，可用前向宣告) 

---

## 7. OOP 設計模式索引

| 模式 | 應用位置 | 說明 |
|------|----------|------|
| **Template Method** | `Scene` ABC | `OnEnter` / `OnExit` / `Update` 三步驟固定，子類別填入細節 |
| **State Machine** | `App::State`、`PlayerCat` 動畫狀態 | App 用 enum 狀態機切換 START/UPDATE/END；PlayerCat 用 `CatAnimState` 切換動畫 clip |
| **Stateless Service** | `CharacterPhysicsSystem`、`SaveManager` | 所有方法為 `static` 的純算法/工具類別，可被多個場景複用 |
| **Data Transfer Object** | `PhysicsAgent`、`PhysicsState`、`OptionSettingsData`、`KeyConfigData`、`LevelSaveData` | 打包資料跨越 Scene ↔ System / SaveManager 邊界 |
| **Borrowing(借用模式)** | MenuScene 共用 UI 物件 | `shared_ptr` 擁有者固定，借用者在 OnEnter/OnExit 配對使用 |
| **Dependency Injection** | Scene 建構子傳入 raw pointer | 避免全域存取，依賴由外部注入 |
| **Abstract Base Class** | `Scene`、`IPhysicsBody` | 強制子類別實作完整介面，編譯期保證 |
| **Composition over Inheritance** | 子場景借用 MenuScene 的 UI，而非繼承 MenuScene | 避免深層繼承樹，降低耦合 |
| **Pending / Applied 兩層資料** | `KeyboardConfigScene`、`OptionMenuScene` | 操作中只改 Pending，確認後才寫入 Applied 並持久化 |
| **Read-Modify-Write** | `SaveManager` | settings.json 同時存放兩種資料，各場景只修改自己負責的欄位 |
| **Header-only Utility** | `CatAssets.hpp` | inline 函式集中管理資源路徑，不需編譯單元 |
