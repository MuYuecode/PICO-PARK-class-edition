# 遊戲架構說明文件

> 物理系統相關說明請參閱 `PHYSICS_DESIGN.md`   
> 本文件說明其餘所有架構：程式入口、App 狀態機、Scene 系統、渲染層、UI 元件、輸入系統、設定系統、持久化儲存、資源慣例，以及未來可能擴充的位置 

---

## 目錄

1. [程式入口與主迴圈](#1-程式入口與主迴圈)
2. [App 狀態機](#2-app-狀態機)
3. [GameContext：跨場景共享資料](#3-gamecontext跨場景共享資料)
4. [Scene 系統](#4-scene-系統)
5. [渲染層](#5-渲染層)
6. [UI 元件](#6-ui-元件)
7. [輸入系統](#7-輸入系統)
8. [設定系統](#8-設定系統)
9. [持久化儲存(SaveManager)](#9-持久化儲存savemanager)
10. [資源路徑慣例](#10-資源路徑慣例)
11. [場景切換流程全覽](#11-場景切換流程全覽)
12. [擴充指引](#12-擴充指引)

---

## 1. 程式入口與主迴圈

```
main.cpp
└── Core::Context::GetInstance()   ← 引擎視窗/OpenGL 初始化(PTSD 提供)
└── App app
    └── while (!context->GetExit())
            START  → app.Start()    建立所有場景與永久物件，讀取持久化設定
            UPDATE → app.Update()   每幀驅動當前場景
            END    → app.End()      釋放資源，通知引擎退出
```

`Core::Context` 由 PTSD 引擎管理，負責視窗建立、OpenGL 環境、事件輪詢   
每次 `context->Update()` 才會真正把渲染結果刷新到螢幕 

---

## 2. App 狀態機

**檔案**：`App.hpp` / `AppStart.cpp` / `AppUpdate.cpp` / `AppEnd.cpp`

```
App::State
    START  ──→  UPDATE  ──→  END
      ↑            │
   app.Start()   每幀呼叫
                 m_CurrentScene->Update()
                 m_Root.Update()(渲染)
```

### App 持有的物件

| 成員 | 型別 | 說明 |
|------|------|------|
| `m_Ctx` | `unique_ptr<GameContext>` | 跨場景共享資料(見第 3 節) |
| `m_Root` | `Util::Renderer` | 全域渲染根節點 |
| `m_TitleScene` | `unique_ptr<Scene>` | 每個場景的**唯一擁有者** |
| `m_MenuScene` | `unique_ptr<Scene>` | 同上 |
| `m_ExitConfirmScene` | `unique_ptr<Scene>` | 同上 |
| `m_OptionMenuScene` | `unique_ptr<Scene>` | 同上 |
| `m_KeyboardConfigScene` | `unique_ptr<Scene>` | 同上 |
| `m_LocalPlayScene` | `unique_ptr<Scene>` | 同上 |
| `m_LocalPlayGameScene` | `unique_ptr<Scene>` | 同上 |
| `m_LevelSelectScene` | `unique_ptr<Scene>` | 同上 |
| `m_CurrentScene` | `Scene*`(non-owning) | 指向當前場景 |

> **未來新增場景**：在 `App.hpp` 宣告 `unique_ptr<Scene> m_XxxScene`，在 `AppStart.cpp` 建立並用 `SetXxx()` 互相串接指標，最後 `std::move` 進成員變數即可 

### TransitionTo

```cpp
void App::TransitionTo(Scene* next) {
    if (m_CurrentScene) m_CurrentScene->OnExit();
    m_CurrentScene = next;
    if (m_CurrentScene) m_CurrentScene->OnEnter();
}
```

場景切換的唯一入口 `App::Update()` 每幀檢查 `m_CurrentScene->Update()` 的回傳值，非 null 且與當前不同時才呼叫 `TransitionTo` 

### ShouldQuit

`GameContext::ShouldQuit` 為 true 時，`App::Update()` 將狀態設為 `END`，下一幀自動結束 

---

## 3. GameContext：跨場景共享資料

**檔案**：`GameContext.hpp`

```cpp
struct GameContext {
    Util::Renderer& Root;           // 渲染根節點(App 持有，Context 只借用)

    // ── 永久存在於渲染樹的物件(AppStart 加入後不再移除)──
    shared_ptr<Character>  Background;  // 背景底圖(可熱替換為白/米黃/深色)
    shared_ptr<Character>  Floor;       // 地板
    shared_ptr<BGMPlayer>  BGMPlayer;   // 背景音樂播放器
    shared_ptr<Character>  Header;      // 標題 Logo
    shared_ptr<Character>  Door;        // 門(open/close 兩種圖片，LocalPlayGameScene 切換)

    // ── 標題畫面的 8 隻裝飾貓(也是遊戲中可操控的角色)──
    vector<shared_ptr<PlayerCat>> StartupCats;

    // ── 遊戲執行期資料 ──
    int  SelectedPlayerCount  = 2;
    int  CooperativePushPower = 1;
    bool ShouldQuit           = false;

    // 角色顏色順序(index 0–7 依序為 blue, red, yellow, green, purple, pink, orange, gray)
    static constexpr array<const char*, 8> kCatColorOrder = { ... };
};
```

### 設計原則

- `GameContext` **不擁有 Renderer**，只持有引用，避免循環依賴 
- 永久物件(`Background`、`Floor`、`Header`、`Door`)只在 `AppStart` 加入渲染樹一次，場景不應移除它們(`LevelSelectScene` 的例外：暫時隱藏 `Header`，離開時還原) 
- `BGMPlayer` 在 `AppStart` 建立，各場景透過 `m_Ctx.BGMPlayer` 呼叫 `SetVolume()`，不需持有所有權 
- 場景之間需要溝通的「遊戲狀態」(人數、推力加成)存在 `GameContext`，不要跨場景直接持有對方的指標來讀取狀態 

> **未來新增全域狀態**(如關卡進度、當前分數)：加到 `GameContext` 的成員，而非放在單一場景裡 

---

## 4. Scene 系統

### 4.1 Scene 介面

**檔案**：`Scene.hpp`

```cpp
class Scene {
public:
    virtual void   OnEnter() = 0;  // 進入時：AddChild + 重置狀態
    virtual void   OnExit()  = 0;  // 離開時：RemoveChild(必須與 OnEnter 配對)
    virtual Scene* Update()  = 0;  // 每幀：邏輯更新，回傳下一個場景或 nullptr
protected:
    GameContext& m_Ctx;
};
```

**AddChild / RemoveChild 鐵律**：`OnEnter` 加入的每一個物件，`OnExit` 必須移除，否則下次進入場景時同一個 `shared_ptr` 會在渲染清單中出現兩次，造成圖層重疊 

### 4.2 現有場景一覽

| 場景 | 檔案 | 職責 | 可前往 |
|------|------|------|--------|
| `TitleScene` | `Titlescene` | 標題畫面，PRESS ENTER 閃爍，8 隻貓可互動 | MenuScene |
| `MenuScene` | `Menuscene` | 主選單(EXIT GAME / OPTION / LOCAL PLAY) | TitleScene, ExitConfirmScene, OptionMenuScene, LocalPlayScene |
| `ExitConfirmScene` | `Exitconfirmscene` | 離開確認對話框(YES/NO) | MenuScene(或設定 ShouldQuit) |
| `OptionMenuScene` | `OptionMenuScene` | 遊戲設定(背景色、音量、顯示數字)，OK 時持久化到 settings.json | MenuScene, KeyboardConfigScene |
| `KeyboardConfigScene` | `KeyboardConfigScene` | 按鍵綁定設定(1P–8P)，CommitPending 時持久化到 settings.json | OptionMenuScene |
| `LocalPlayScene` | `LocalPlayScene` | 選擇玩家人數(2–8) | MenuScene, LocalPlayGameScene |
| `LocalPlayGameScene` | `LocalPlayGameScene` | 實際遊戲(多角色物理、合作推動、門進入判斷) | LocalPlayScene, LevelSelectScene |
| `LevelSelectScene` | `LevelSelectScene` | 關卡選擇(10 關格子、橘色框、皇冠標記、最佳時間) | LocalPlayGameScene(ESC), LevelNScene(ENTER) |

### 4.3 共用 UI 物件的借用模式

`MenuScene` 建構並擁有以下物件的所有權：

```
m_MenuFrame          (選單外框)
m_ExitGameButton     (X 按鈕)
m_LeftTriButton      (◀ 按鈕)
m_RightTriButton     (▶ 按鈕)
m_blue_cat_run_img   (藍貓跑步圖示)
```

`AppStart.cpp` 呼叫這些 getter，並透過**建構子參數**將 `shared_ptr` 注入各 Scene Scene 在建構時就持有這些物件，`OnEnter` / `OnExit` 只負責 `AddChild` / `RemoveChild`，不需要再呼叫 getter 

```
AppStart 呼叫 getter → 建構子注入 → Scene 持有 shared_ptr

MenuScene::GetMenuFrame()       → 注入 ExitConfirmScene, LocalPlayScene
MenuScene::GetExitGameButton()  → 注入 ExitConfirmScene, OptionMenuScene, KeyboardConfigScene
MenuScene::GetLeftTriButton()   → 注入 LocalPlayScene
MenuScene::GetRightTriButton()  → 注入 LocalPlayScene
MenuScene::GetBlueCatRunImg()   → 注入 LocalPlayScene
```

各 Scene 在 `OnEnter` 加入渲染樹、`OnExit` 移除，但**不擁有所有權** 

> **未來新增選單子場景**(例如 `OnlinePlayScene`)：同樣從 `MenuScene` 取得上述物件，在 `OnEnter`/`OnExit` 配對加入/移除，借用結束後還原 scale/position 

### 4.4 場景間指標傳遞規則

- `App` 以 `unique_ptr<Scene>` 持有場景(**唯一擁有者**) 
- 場景之間以 **raw pointer**(non-owning)互相引用，絕對不能 `delete` 
- 循環引用問題(A 需要 B、B 需要 A)用 `SetXxx(ptr)` setter 在 `AppStart` 建立完兩者後再串接 

---

## 5. 渲染層

### 5.1 繼承關係

```
Util::GameObject          (PTSD 引擎基底，持有 m_Drawable、m_Transform、m_ZIndex)
├── Character              圖片物件(Util::Image 作為 drawable)
│   └── UI_Triangle_Button 帶「按下」狀態的圖片按鈕
├── AnimatedCharacter      動畫物件(Util::Animation 作為 drawable)
│   └── PlayerCat          可操控角色，含動畫狀態機與 IPhysicsBody 介面
└── GameText               文字物件(Util::Text 作為 drawable)
```

### 5.2 Character

**檔案**：`Character.hpp` / `Character.cpp`

最基礎的圖片物件，封裝 `Util::Image` 

| 方法 | 說明 |
|------|------|
| `SetImage(path)` | 熱替換圖片(會重建 Drawable) |
| `GetPosition()` | 回傳 `m_Transform.translation` |
| `SetPosition(vec2)` | 同上 |
| `GetSize()` | 回傳縮放後的像素大小(呼叫 `GetScaledSize()`) |
| `SetScale(vec2)` | 修改 `m_Transform.scale`(負 X = 水平翻轉) |
| `IsMouseHovering()` | AABB 滑鼠懸停偵測(委託 `AppUtil::IsMouseHovering`) |
| `IsLeftClicked()` | 滑鼠左鍵點擊偵測(委託 `AppUtil::IsLeftClicked`) |

### 5.3 AnimatedCharacter

**檔案**：`AnimatedCharacter.hpp` / `AnimatedCharacter.cpp`

封裝 `Util::Animation` 建構時傳入幀路徑清單，引擎依 `interval` 自動切幀 

| 方法 | 說明 |
|------|------|
| `Play()` | 開始播放 |
| `SetLooping(bool)` | 設定是否循環 |
| `IsPlaying()` | 查詢播放中 |
| `IfAnimationEnds()` | 當前幀是否為最後一幀(非循環動畫結束偵測) |

`PlayerCat` 繼承此類並擴充為多動畫 clip 的狀態機(詳見 `PHYSICS_DESIGN.md`) 

### 5.4 GameText

**檔案**：`GameText.hpp` / `GameText.cpp`

封裝 `Util::Text` 固定使用 `TerminusTTFWindows-Bold` 字型 

| 方法 | 說明 |
|------|------|
| `SetText(string)` | 動態更新文字內容(選單數值變更時使用) |
| `SetColor(Color)` | 動態更新文字顏色 |
| `SetVisible(bool)` | 顯示/隱藏 |
| `GetSize()` | 回傳文字佔用的像素尺寸(用於對齊計算) |
| `IsMouseHovering()` | AABB 滑鼠懸停偵測(委託 `AppUtil::IsMouseHovering`) |
| `IsLeftClicked()` | 滑鼠左鍵點擊偵測(委託 `AppUtil::IsLeftClicked`) |

### 5.5 ZIndex 慣例

| 範圍 | 用途 |
|------|------|
| `-10` | 白色背景 |
| `0` | 地板、Header |
| `5` | Door(門) |
| `10` | 選單外框、藍貓圖示 |
| `15` | 三角按鈕 |
| `20` | ExitGameButton、角色(`20.0 + i * 0.01`)、場景私有文字 |
| `25` | OptionMenuFrame / KeyboardConfigScene Frame / LevelSelectScene BgFrame |
| `30` | ChoiceFrame、DoorCountText |
| `32` | LevelSelectScene 橘色選擇框 |
| `35` | 選單文字 |
| `36` | LevelSelectScene 皇冠標記 |
| `100` | GameText 預設值 |

---

## 6. UI 元件

### UI_Triangle_Button

**檔案**：`UI_Triangle_Button.hpp` / `UI_Triangle_Button.cpp`

繼承 `Character`，管理「一般 / 按下」兩種圖片狀態 

```
正常狀態 ──Press(durationMs)──→ 按下狀態
              │
              └── 計時 durationMs 毫秒
                      ↓
              自動回到正常狀態
```

| 方法 | 說明 |
|------|------|
| `Press(ms)` | 切換為按下圖片，ms 毫秒後自動還原(預設 75ms) |
| `UpdateButton()` | **每幀必須呼叫**，負責計時器倒數 |
| `ResetState()` | 強制還原為正常狀態(場景切換時清除殘留) |

---

## 7. 輸入系統

**來源**：PTSD 引擎 `Util/Input.hpp`

| 函式 | 語意 |
|------|------|
| `Util::Input::IsKeyDown(key)` | **這一幀剛按下**(只觸發一次) |
| `Util::Input::IsKeyPressed(key)` | **持續按住中**(每幀都為 true) |
| `Util::Input::GetCursorPosition()` | 滑鼠座標(螢幕空間，Y 向上為正) |

### 使用慣例

- **選單切換、跳躍、確認**：用 `IsKeyDown`(避免單次動作重複觸發) 
- **角色水平移動**：用 `IsKeyPressed`(需要持續移動) 
- **滑鼠點擊**：`IsKeyDown(Util::Keycode::MOUSE_LB)` 搭配 `IsMouseHovering()` 實作(已封裝在 `Character::IsLeftClicked()` 和 `GameText::IsLeftClicked()`) 

### AppUtil 工具函式

**檔案**：`AppUtil.hpp` / `AppUtil.cpp`

| 函式 | 說明 |
|------|------|
| `AlignLeft(text, boundaryX)` | 計算文字置左時的中心 X(用於選單標籤對齊) |
| `AlignRight(text, boundaryX)` | 計算文字置右時的中心 X |
| `KeycodeToString(key)` | 將 `Util::Keycode` 轉成顯示字串(`UNKNOWN` → `"-"`) |
| `GetAnyKeyDown()` | 回傳這幀剛被按下的第一個可綁定按鍵 |
| `IsMouseHovering(obj)` | 對任意 `Util::GameObject` 做 AABB 懸停偵測 |
| `IsLeftClicked(obj)` | 對任意 `Util::GameObject` 做左鍵點擊偵測 |

`GetAnyKeyDown()` 供 `KeyboardConfigScene` 的按鍵捕捉使用 

---

## 8. 設定系統

### 8.1 PlayerKeyConfig

**檔案**：`KeyboardConfigScene.hpp`

每位玩家的按鍵綁定結構 

```cpp
struct PlayerKeyConfig {
    Util::Keycode up, down, left, right;  // 移動
    Util::Keycode jump;
    Util::Keycode cancel;
    Util::Keycode shot;
    Util::Keycode menu;     // 僅 1P 有效
    Util::Keycode subMenu;  // 僅 1P 有效
};
```

**預設值**(靜態常數，供 fallback 使用)：

```
k_Default1P : W/S/A/D 移動，W 跳，ESC 取消，SPACE 射擊，ENTER 選單，TAB 子選單
k_Default2P : ↑↓←→ 移動，↑ 跳，AC_BACK 取消，R_CTRL 射擊
3P–8P       : 全部 UNKNOWN(需玩家手動設定)
```

### 8.2 KeyboardConfigScene

**檔案**：`KeyboardConfigScene.hpp` / `KeyboardConfigScene.cpp`

- 建構子啟動時透過 `SaveManager::LoadKeyConfigs()` 從 `settings.json` 還原上次設定 
- 持有 `m_Applied[MAX_PLAYERS]`(已確認的設定)與 `m_Pending`(編輯中的暫存) 
- `CommitPending()` 將暫存寫入 `m_Applied[m_CurrentPlayer]`，並呼叫 `SaveManager::SaveKeyConfigs()` 持久化 
- `GetAppliedConfig(i)` 供 `LocalPlayGameScene::SpawnPlayers()` 取得按鍵設定 
- `GetConfiguredPlayerCount()` 回傳已設定足夠按鍵(≥4 個非 UNKNOWN)的玩家數，供 `LocalPlayScene` 顯示警告與阻擋進入遊戲 

### 8.3 OptionMenuScene::Settings

```cpp
struct Settings {
    int  bgColorIndex = 0;   // 背景顏色索引(對應 s_BgColorOptions：WHITE/CREAM/DARK)
    int  bgmVolume    = 10;  // BGM 音量(0–20)，實際傳入 BGMPlayer 時乘 6
    int  seVolume     = 10;  // SE 音量(0–20，目前 SE 系統尚未實作)
    bool dispNumber   = false; // 是否顯示玩家編號
};
```

- 建構子啟動時透過 `SaveManager::LoadOptionSettings()` 從 `settings.json` 還原上次設定 
- `m_Applied` 為生效值，`m_Pending` 為選單操作中的暫存值；按下 OK 才將 `m_Pending` 寫入 `m_Applied`，同時呼叫 `SaveManager::SaveOptionSettings()` 持久化，按下 CANCEL 捨棄 

---

## 9. 持久化儲存(SaveManager)

**檔案**：`SaveManager.hpp` / `SaveManager.cpp`

`SaveManager` 是純靜態的工具類別，集中管理兩個 JSON 檔案的讀寫，使用 PTSD 引擎已附帶的 **nlohmann/json** 函式庫 

### 9.1 檔案位置

| 檔案 | 用途 |
|------|------|
| `GA_RESOURCE_DIR/Save/settings.json` | Option 設定(背景色、音量)+ 8 位玩家按鍵綁定 |
| `GA_RESOURCE_DIR/Save/save_data.json` | 10 個關卡的通關狀態與各人數的最佳時間 |

`Save/` 目錄若不存在，`SaveManager` 會在首次寫入時自動建立(`std::filesystem::create_directories`) 

### 9.2 settings.json 結構

```json
{
  "bgColorIndex": 0,
  "bgmVolume": 10,
  "seVolume": 10,
  "dispNumber": false,
  "keyConfigs": [
    { "up": 26, "down": 22, "left": 4, "right": 7,
      "jump": 26, "cancel": 41, "shot": 44, "menu": 40, "subMenu": 43 },
    { "up": 82, "down": 81, "left": 80, "right": 79,
      "jump": 82, "cancel": 0, "shot": 228, "menu": 0, "subMenu": 0 },
    ...
  ]
}
```

按鍵以 `int` 儲存(`Util::Keycode` 底層為 SDL_Scancode，`UNKNOWN = 0`) 

### 9.3 save_data.json 結構

```json
{
  "levels": [
    {
      "levelId": 1,
      "completed": true,
      "bestTimes": { "2": 45.3, "3": -1.0, "4": -1.0, "5": -1.0,
                     "6": -1.0, "7": -1.0, "8": -1.0 }
    },
    ...
  ]
}
```

`bestTimes` 以人數(2–8)為 key，`-1.0` 表示該人數尚未通關 

### 9.4 讀寫策略：read-modify-write

`settings.json` 混合存放 Option 設定和按鍵綁定兩種資料，分別由不同的場景負責寫入   
為避免場景 A 的寫入覆蓋場景 B 的欄位，`SaveOptionSettings` 和 `SaveKeyConfigs` 都採用：

```
1. 讀取現有 JSON(若不存在則建立空物件)
2. 只修改自己負責的欄位
3. 寫回完整 JSON
```

### 9.5 API 摘要

| 函式 | 說明 |
|------|------|
| `SaveOptionSettings(opts)` | 寫入 Option 設定(不覆蓋 keyConfigs) |
| `LoadOptionSettings(outOpts)` | 讀取 Option 設定 |
| `SaveKeyConfigs(keys)` | 寫入全 8P 按鍵綁定(不覆蓋 option 欄位) |
| `LoadKeyConfigs(outKeys)` | 讀取按鍵綁定 |
| `SaveLevelData(levels)` | 寫入全 10 關存檔 |
| `LoadLevelData(outLevels)` | 讀取全 10 關存檔 |
| `UpdateBestTime(idx, p, elapsed)` | 便利函式：僅在新紀錄時寫入 |
| `FormatTime(seconds)` | 格式化為 `mm:ss.cs`(`-1` → `--:--.--`) |

---

## 10. 資源路徑慣例

所有路徑以 `GA_RESOURCE_DIR` 巨集為根(由 CMake 定義，指向 `resources/` 目錄) 

### 目錄結構

```
resources/
├── BGM/
│   ├── ppc.mp3
│   ├── pp1.mp3
│   └── pp2.mp3
├── Font/
│   ├── TerminusTTFWindows-Bold-4.49.3.ttf   (主要字型)
│   ├── FORCEDSQUARE.ttf
│   └── Monocraft.ttf
├── Image/
│   ├── Background/
│   │   ├── white_background.png
│   │   ├── cream_background.png
│   │   ├── dark_background.png
│   │   ├── background_floor.png
│   │   ├── header.png
│   │   ├── door_close.png / door_open.png
│   │   ├── level_select_frame.png          ← LevelSelectScene 背景框
│   │   ├── Menu_Frame.png
│   │   ├── Choice_Frame.png
│   │   ├── Option_Menu_Frame.png
│   │   ├── Option_Choice_Frame.png         ← 複用作為關卡選擇框
│   │   ├── Option_HLine.png
│   │   └── Keyboard_Config_Menu_Frame.png
│   ├── Button/
│   │   ├── ExitGameButton.png
│   │   ├── Left_Tri_Button.png / Left_Tri_Button_Full.png
│   │   └── Right_Tri_Button.png / Right_Tri_Button_Full.png
│   └── Character/
│       └── {color}_cat/
│           ├── {color}_cat_stand_1–8.png    (8 幀)
│           ├── {color}_cat_run_1–9.png      (9 幀)
│           ├── {color}_cat_jump_1.png       (起跳→最高點)
│           ├── {color}_cat_jump_2.png       (最高點→落地前)
│           ├── {color}_cat_land_1–2.png
│           └── {color}_cat_push_1–3.png    (3 幀)
└── Save/                                    ← 持久化資料(自動建立)
    ├── settings.json                        ← Option 設定 + 按鍵綁定
    └── save_data.json                       ← 關卡進度與最佳時間
```

### 角色顏色順序

由 `GameContext::kCatColorOrder` 統一定義：

```
index 0 = blue    index 4 = purple
index 1 = red     index 5 = pink
index 2 = yellow  index 6 = orange
index 3 = green   index 7 = gray
```

### CatAssets 工具(CatAssets.hpp)

| 函式 | 說明 |
|------|------|
| `BuildFramePath(color, action, frameNum)` | 建立單幀路徑 |
| `BuildFramePaths(color, action, numFrames)` | 建立多幀路徑(frame 1 ~ numFrames) |
| `BuildFullAnimPaths(color)` | 建立完整的 `CatAnimPaths` |

---

## 11. 場景切換流程全覽

```
AppStart
    │
    ▼
TitleScene ──ENTER──→ MenuScene ──A/D 選單──→ ExitConfirmScene ──YES──→ ShouldQuit
                          │                         │
                          │              ESC/NO──→ MenuScene
                          │
                          ├──→ OptionMenuScene ──ENTER(row0)/mouse──→ KeyboardConfigScene
                          │         │                                        │
                          │    OK/ESC/CANCEL ←──────────────────────────────┘
                          │    (OK 時寫 settings.json)
                          │
                          └──→ LocalPlayScene ──ENTER(人數 ≤ configuredCount)──→ LocalPlayGameScene
                                    │                                                    │
                               ESC/X Button ←──────────────────────────  全員進門──→ LevelSelectScene
                               → MenuScene                                               │
                                                                           ENTER/click──→ LevelNScene
                                                                           ESC──────────→ LocalPlayGameScene
```

---

## 12. 擴充指引

### 新增一個關卡場景(例如 `LevelOneScene`)

1. 建立 `include/LevelOneScene.hpp` 和 `src/LevelOneScene.cpp`，繼承 `Scene`，實作 `OnEnter`、`OnExit`、`Update` 
2. 在 `App.hpp` 新增 `unique_ptr<Scene> m_LevelOneScene` 
3. 在 `AppStart.cpp` 建立實例，並呼叫 `m_LevelSelectScene->SetLevelScene(0, m_LevelOneScene.get())` 
4. 關卡完成時呼叫 `SaveManager::UpdateBestTime(0, m_Ctx.SelectedPlayerCount, elapsed)` 更新最佳時間 
5. 在 `CMakeLists.txt` 加入新的 `.cpp` 檔 

### 新增一個全域設定項目

1. 在 `OptionMenuScene::Settings` 加入欄位 
2. 在 `OptionMenuScene` 的 UI 建立對應的列(參考 BG COLOR 列的模式) 
3. 在 `OptionSettingsData`(`SaveManager.hpp`)加入對應欄位 
4. 更新 `SaveManager::SaveOptionSettings` 和 `LoadOptionSettings` 讀寫該欄位 
5. 若設定需跨場景生效，在 `GameContext` 加入對應成員，`OptionMenuScene` 在 OK 時同步寫入 

### 新增一個角色顏色

1. 在 `resources/Image/Character/` 下建立 `{color}_cat/` 資料夾，放入所有動畫幀 
2. 在 `GameContext::kCatColorOrder` 陣列末尾追加顏色名稱字串 
3. `LocalPlayScene::MAX_PLAYERS` 若需要支援超過 8 人，一併調整
