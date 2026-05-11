# LevelFourScene 程式碼實作計畫

這份文件指定完整實作方式。照順序完成即可，不需要再做設計決策。

## 目標

新增 `LevelFourScene`，玩法為玩家跳躍閃避由右側 Shooter 發出的子彈，讓子彈依序擊中 `Jar0 -> Jar1 -> Jar2 -> 消失`。Jar 消失後，玩家才能撿到和 Jar 重疊的 Key，持 Key 開 Door，所有玩家進門後通關。Key、Door、計時、存檔、多人進門邏輯全部沿用 `LevelTwoScene` 的做法。

## 需要修改或新增的檔案

1. 新增 `include/scenes/LevelFourScene.hpp`
2. 新增 `src/scenes/LevelFourScene.cpp`
3. 修改 `src/app/AppStart.cpp`
4. 修改 `include/app/AppUtil.hpp`
5. 不修改 `CMakeLists.txt`，因為目前 `file(GLOB_RECURSE SRC_FILES src/*.cpp)` 和 `file(GLOB_RECURSE HEADER_FILES include/*.hpp)` 會自動納入新檔案。
6. 不修改 `include/core/SceneId.hpp`，因為 `SceneId::Level04` 已經存在。
7. 不修改 `include/scenes/LevelSelectScene.hpp` 的封面路徑，因為第 4 關已經使用 `/Image/Level_Cover/LevelFour.png`。

## 參考來源

1. `LevelTwoScene`：複製玩家生成、按鍵處理、Key 拾取、Key 跟隨、Door 開啟、進門通關、計時與 `SaveManager::UpdateBestTime`。
2. `LevelThreeScene`：參考局部 helper、動態物件 body 與 `SetupSceneVisuals` / `SetupStaticBoundaries` 的拆分方式。
3. `PhysicsBodyTraits.hpp`：已經有 `BodyType::BULLET`，子彈 body 直接使用此類型。

## AppUtil 對齊工具

在 `include/app/AppUtil.hpp` 加入 `Character` 版本的 sprite 對齊 helper。保留現有 `GameText` helper，不刪改既有函式。

新增 include：

```cpp
#include "gameplay/Character.hpp"
```

在 `namespace AppUtil` 內新增：

```cpp
inline float SpriteHalfW(const Character& sprite) {
    return std::abs(sprite.GetScaledSize().x) * 0.5f;
}

inline float SpriteHalfH(const Character& sprite) {
    return std::abs(sprite.GetScaledSize().y) * 0.5f;
}

inline float LeftEdge(const Character& sprite) {
    return sprite.GetPosition().x - SpriteHalfW(sprite);
}

inline float RightEdge(const Character& sprite) {
    return sprite.GetPosition().x + SpriteHalfW(sprite);
}

inline float TopEdge(const Character& sprite) {
    return sprite.GetPosition().y + SpriteHalfH(sprite);
}

inline float BottomEdge(const Character& sprite) {
    return sprite.GetPosition().y - SpriteHalfH(sprite);
}

inline float AlignSpriteLeft(const Character& sprite, float boundaryX) {
    return boundaryX + SpriteHalfW(sprite);
}

inline float AlignSpriteRight(const Character& sprite, float boundaryX) {
    return boundaryX - SpriteHalfW(sprite);
}

inline float AlignSpriteTop(const Character& sprite, float boundaryY) {
    return boundaryY - SpriteHalfH(sprite);
}

inline float AlignSpriteBottom(const Character& sprite, float boundaryY) {
    return boundaryY + SpriteHalfH(sprite);
}

inline float AlignSpriteBelow(const Character& sprite, const Character& anchor) {
    return BottomEdge(anchor) - SpriteHalfH(sprite);
}

inline float AlignSpriteAbove(const Character& sprite, const Character& anchor) {
    return TopEdge(anchor) + SpriteHalfH(sprite);
}
```

如果缺少 `<cmath>`，同一個 header 補上：

```cpp
#include <cmath>
```

## 座標系與固定位置

沿用現有關卡的 1280x720 中心座標：

```cpp
static constexpr float kViewLeftX   = -640.0f;
static constexpr float kViewRightX  =  640.0f;
static constexpr float kViewTopY    =  360.0f;
static constexpr float kViewBottomY = -360.0f;
```

LevelFour 素材尺寸：

| 物件 | 尺寸 | 半尺寸 |
| --- | --- | --- |
| Ceiling | 1280 x 24 | 640 x 12 |
| Floor | 1280 x 23 | 640 x 11.5 |
| LHighWall | 96 x 608 | 48 x 304 |
| LMidWall | 32 x 63 | 16 x 31.5 |
| RHighWall | 128 x 607 | 64 x 303.5 |
| RMidWall | 32 x 64 | 16 x 32 |
| Jar0/Jar1/Jar2 | 43 x 55 | 21.5 x 27.5 |
| Shooter | 61 x 56 | 30.5 x 28 |

用 AppUtil helper 擺放後，中心座標必須得到下列值：

| 物件 | 放置規則 | 中心座標 |
| --- | --- | --- |
| Ceiling | x = 0，top edge 對齊 `kViewTopY` | `{0.0f, 348.0f}` |
| Floor | x = 0，bottom edge 對齊 `kViewBottomY` | `{0.0f, -348.5f}` |
| LHighWall | left edge 對齊 `kViewLeftX`，top edge 對齊 Ceiling bottom edge | `{-592.0f, 32.0f}` |
| LMidWall | left edge 對齊 `kViewLeftX`，top edge 對齊 LHighWall bottom edge | `{-624.0f, -303.5f}` |
| RHighWall | right edge 對齊 `kViewRightX`，top edge 對齊 Ceiling bottom edge | `{576.0f, 32.5f}` |
| RMidWall | right edge 對齊 `kViewRightX`，top edge 對齊 RHighWall bottom edge | `{624.0f, -303.0f}` |
| Jar0 | left edge 對齊 LMidWall right edge，bottom edge 對齊 Floor top edge | `{-586.5f, -309.5f}` |
| Shooter | right edge 對齊 RMidWall left edge，bottom edge 對齊 Floor top edge | `{577.5f, -309.0f}` |
| Key | 中心和 Jar0 相同 | `{-586.5f, -309.5f}` |
| Door | 在 Shooter 右方，bottom edge 對齊 Floor top edge | `{609.0f, -337.0f + doorHalfH}` |

Door 的 `doorHalfH` 要用實際縮放後尺寸計算：

```cpp
const float doorHalfH = std::abs(m_Actors.Door()->GetScaledSize().y) * 0.5f;
m_Actors.Door()->SetPosition({609.0f, FloorTopY() + doorHalfH});
```

玩家起始位置：

```cpp
const float spawnY = FloorTopY() + PlayerCat::kHalfHeight;
const float spawnX = -500.0f + static_cast<float>(i) * 38.0f;
```

這會讓所有 2 到 8 位玩家都在 Jar 右方、Door 左方。

## LevelFourScene.hpp 結構

新增 `include/scenes/LevelFourScene.hpp`，內容按下列結構建立：

```cpp
#ifndef LEVEL_FOUR_SCENE_HPP
#define LEVEL_FOUR_SCENE_HPP

#include <memory>
#include <vector>

#include "core/Scene.hpp"
#include "gameplay/Character.hpp"
#include "gameplay/PlayerCat.hpp"
#include "gameplay/PlayerKeyConfig.hpp"
#include "physics/PhysicsWorld.hpp"
#include "ui/GameText.hpp"

class LevelFourScene : public Scene {
public:
    explicit LevelFourScene(SceneServices services);
    ~LevelFourScene() override = default;

    void OnEnter() override;
    void OnExit() override;
    void Update() override;
    void PauseGameplay() override;
    void ResumeGameplay() override;

private:
    struct PlayerBinding {
        std::shared_ptr<PlayerCat> cat;
        PlayerKeyConfig key;
    };

    class BulletBody;

    enum class JarState {
        Jar0,
        Jar1,
        Jar2,
        Gone,
    };

    static bool AabbOverlap(const glm::vec2& aPos, const glm::vec2& aHalf,
                            const glm::vec2& bPos, const glm::vec2& bHalf);

    void SetupSceneVisuals();
    void SetupStaticBoundaries();
    void SpawnPlayers(int count);
    void ApplyInitialFormation() const;
    void HandlePlayerInput() const;
    void UpdateTimerText() const;

    float FloorTopY() const;
    float CurrentBulletSpeed() const;
    void SpawnBullet();
    void DeactivateBullet();
    void SyncBulletSprite() const;
    void UpdateBulletSpawner(float dtSec);
    void HandleBulletHits();
    void AdvanceJarState();

    void TryPickKey();
    void UpdateKeyFollow() const;
    void TryOpenDoorAndClear();
    void UpdateDoorEntryAndClear();

    static constexpr int kLevelIndex = 3;
    static constexpr float kViewLeftX = -640.0f;
    static constexpr float kViewRightX = 640.0f;
    static constexpr float kViewTopY = 360.0f;
    static constexpr float kViewBottomY = -360.0f;

    static constexpr glm::vec2 kKeyHalf = {20.0f, 26.0f};
    static constexpr glm::vec2 kBulletHalf = {9.0f, 9.0f};
    static constexpr float kBulletBaseSpeed = 240.0f;
    static constexpr float kBulletRespawnDelaySec = 0.8f;
    static constexpr float kInitialBulletDelaySec = 1.0f;

    PhysicsWorld m_World;

    std::shared_ptr<Character> m_CeilingSprite;
    std::shared_ptr<Character> m_FloorSprite;
    std::shared_ptr<Character> m_LHighWallSprite;
    std::shared_ptr<Character> m_LMidWallSprite;
    std::shared_ptr<Character> m_RHighWallSprite;
    std::shared_ptr<Character> m_RMidWallSprite;
    std::shared_ptr<Character> m_JarSprite;
    std::shared_ptr<Character> m_ShooterSprite;
    std::shared_ptr<Character> m_KeySprite;

    std::shared_ptr<GameText> m_BulletSprite;
    std::shared_ptr<GameText> m_TimerText;
    std::shared_ptr<BulletBody> m_BulletBody;

    std::vector<PlayerBinding> m_Players;
    std::vector<bool> m_PlayerEntered;

    JarState m_JarState = JarState::Jar0;
    int m_KeyCarrierIdx = -1;
    int m_EnteredCount = 0;

    bool m_DoorOpened = false;
    bool m_ClearDone = false;
    bool m_BulletActive = false;

    float m_BulletCooldownSec = kInitialBulletDelaySec;
    float m_ElapsedSec = 0.0f;
};

#endif
```

## LevelFourScene.cpp 實作順序

新增 `src/scenes/LevelFourScene.cpp`。include 順序：

```cpp
#include "scenes/LevelFourScene.hpp"

#include <algorithm>
#include <cmath>

#include "app/AppUtil.hpp"
#include "gameplay/CatAssets.hpp"
#include "scenes/KeyboardConfigScene.hpp"
#include "services/SaveManager.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"
```

加上 alias：

```cpp
using ip = Util::Input;
using k = Util::Keycode;
```

### BulletBody

在 `.cpp` 最上方實作 private nested class。子彈使用 PhysicsWorld 更新位置，但碰撞由 `LevelFourScene::HandleBulletHits` 做 AABB 判定。

```cpp
class LevelFourScene::BulletBody final : public IPhysicsBody {
public:
    BulletBody(const glm::vec2& pos, const glm::vec2& half)
        : m_Pos(pos), m_Half(glm::abs(half)) {
        SetActive(false);
    }

    [[nodiscard]] const PhysicsBodyTraits& GetPhysicsTraits() const override {
        static const PhysicsBodyTraits kTraits{BodyType::BULLET, false, false};
        return kTraits;
    }

    [[nodiscard]] glm::vec2 GetPosition() const override { return m_Pos; }
    void SetPosition(const glm::vec2& pos) override { m_Pos = pos; }
    [[nodiscard]] glm::vec2 GetHalfSize() const override { return m_Half; }

    [[nodiscard]] bool IsSolid() const override { return false; }
    [[nodiscard]] bool IsKinematic() const override { return false; }
    [[nodiscard]] int GetMoveDir() const override { return 0; }

    void SetSpeed(float speed) { m_Speed = std::max(0.0f, speed); }

    void PhysicsUpdate() override {
        const float dtSec = Util::Time::GetDeltaTimeMs() / 1000.0f;
        m_Desired = {-m_Speed * std::max(0.0f, dtSec), 0.0f};
    }

    [[nodiscard]] glm::vec2 GetDesiredDelta() const override { return m_Desired; }
    void ApplyResolvedDelta(const glm::vec2& delta) override { m_Pos += delta; }
    void OnCollision(const CollisionInfo& /*info*/) override {}
    void PostUpdate() override {}

private:
    glm::vec2 m_Pos;
    glm::vec2 m_Half;
    glm::vec2 m_Desired = {0.0f, 0.0f};
    float m_Speed = 0.0f;
};
```

### Constructor

Constructor 建立所有 sprite，不做座標設定。resource path 必須完全如下：

```cpp
LevelFourScene::LevelFourScene(SceneServices services)
    : Scene(services) {
    constexpr const char* base = GA_RESOURCE_DIR "/Image/Level_Cover/LevelFourScene/";

    m_CeilingSprite = std::make_shared<Character>(std::string(base) + "Ceiling.png");
    m_FloorSprite = std::make_shared<Character>(std::string(base) + "Floor.png");
    m_LHighWallSprite = std::make_shared<Character>(std::string(base) + "LHighWall.png");
    m_LMidWallSprite = std::make_shared<Character>(std::string(base) + "LMidWall.png");
    m_RHighWallSprite = std::make_shared<Character>(std::string(base) + "RHighWall.png");
    m_RMidWallSprite = std::make_shared<Character>(std::string(base) + "RMidWall.png");
    m_JarSprite = std::make_shared<Character>(std::string(base) + "Jar0.png");
    m_ShooterSprite = std::make_shared<Character>(std::string(base) + "Shooter.png");

    m_KeySprite = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/Key.png");
    m_BulletSprite = std::make_shared<GameText>("O", 28, Util::Color::FromRGB(0, 0, 0, 255));
    m_TimerText = std::make_shared<GameText>("TIME 00:00.00", 42, Util::Color::FromRGB(255, 140, 0, 255));

    m_CeilingSprite->SetZIndex(4.0f);
    m_FloorSprite->SetZIndex(4.0f);
    m_LHighWallSprite->SetZIndex(4.1f);
    m_LMidWallSprite->SetZIndex(4.1f);
    m_RHighWallSprite->SetZIndex(4.1f);
    m_RMidWallSprite->SetZIndex(4.1f);
    m_ShooterSprite->SetZIndex(16.0f);
    m_KeySprite->SetZIndex(17.0f);
    m_JarSprite->SetZIndex(18.0f);
    m_BulletSprite->SetZIndex(19.0f);
    m_TimerText->SetZIndex(40.0f);
    m_TimerText->SetPosition({450.0f, 320.0f});
    m_BulletSprite->SetVisible(false);
}
```

### AABB

完全沿用 `LevelTwoScene::AabbOverlap`：

```cpp
bool LevelFourScene::AabbOverlap(const glm::vec2& aPos, const glm::vec2& aHalf,
                                 const glm::vec2& bPos, const glm::vec2& bHalf) {
    return std::abs(aPos.x - bPos.x) < (aHalf.x + bHalf.x) &&
           std::abs(aPos.y - bPos.y) < (aHalf.y + bHalf.y);
}
```

### FloorTopY

```cpp
float LevelFourScene::FloorTopY() const {
    return AppUtil::TopEdge(*m_FloorSprite);
}
```

### SetupSceneVisuals

所有背景位置在這裡設定，使用 AppUtil 對齊 helper。

```cpp
void LevelFourScene::SetupSceneVisuals() {
    m_CeilingSprite->SetPosition({
        0.0f,
        AppUtil::AlignSpriteTop(*m_CeilingSprite, kViewTopY)
    });

    m_FloorSprite->SetPosition({
        0.0f,
        AppUtil::AlignSpriteBottom(*m_FloorSprite, kViewBottomY)
    });

    m_LHighWallSprite->SetPosition({
        AppUtil::AlignSpriteLeft(*m_LHighWallSprite, kViewLeftX),
        AppUtil::AlignSpriteBelow(*m_LHighWallSprite, *m_CeilingSprite)
    });

    m_LMidWallSprite->SetPosition({
        AppUtil::AlignSpriteLeft(*m_LMidWallSprite, kViewLeftX),
        AppUtil::AlignSpriteBelow(*m_LMidWallSprite, *m_LHighWallSprite)
    });

    m_RHighWallSprite->SetPosition({
        AppUtil::AlignSpriteRight(*m_RHighWallSprite, kViewRightX),
        AppUtil::AlignSpriteBelow(*m_RHighWallSprite, *m_CeilingSprite)
    });

    m_RMidWallSprite->SetPosition({
        AppUtil::AlignSpriteRight(*m_RMidWallSprite, kViewRightX),
        AppUtil::AlignSpriteBelow(*m_RMidWallSprite, *m_RHighWallSprite)
    });

    m_JarSprite->SetImage(GA_RESOURCE_DIR "/Image/Level_Cover/LevelFourScene/Jar0.png");
    m_JarSprite->SetVisible(true);
    m_JarSprite->SetPosition({
        AppUtil::RightEdge(*m_LMidWallSprite) + AppUtil::SpriteHalfW(*m_JarSprite),
        FloorTopY() + AppUtil::SpriteHalfH(*m_JarSprite)
    });

    m_ShooterSprite->SetPosition({
        AppUtil::LeftEdge(*m_RMidWallSprite) - AppUtil::SpriteHalfW(*m_ShooterSprite),
        FloorTopY() + AppUtil::SpriteHalfH(*m_ShooterSprite)
    });

    m_KeySprite->SetVisible(true);
    m_KeySprite->SetPosition(m_JarSprite->GetPosition());

    if (m_Actors.Door() != nullptr) {
        const float doorHalfH = std::abs(m_Actors.Door()->GetScaledSize().y) * 0.5f;
        m_Actors.Door()->SetVisible(true);
        m_Actors.Door()->SetImage(GA_RESOURCE_DIR "/Image/Background/door_close.png");
        m_Actors.Door()->SetPosition({609.0f, FloorTopY() + doorHalfH});
    }
}
```

### SetupStaticBoundaries

六個背景元件都建立靜態碰撞，Floor 和 Ceiling 也建立碰撞。Jar、Shooter、Door 不建立碰撞，避免阻擋玩家或子彈流程。

```cpp
void LevelFourScene::SetupStaticBoundaries() {
    const auto addSpriteBoundary = [&](const std::shared_ptr<Character>& sprite) {
        if (sprite == nullptr) return;
        const glm::vec2 half = glm::abs(sprite->GetScaledSize()) * 0.5f;
        m_World.AddStaticBoundary(sprite->GetPosition(), half, BodyType::STATIC_BOUNDARY);
    };

    addSpriteBoundary(m_CeilingSprite);
    addSpriteBoundary(m_FloorSprite);
    addSpriteBoundary(m_LHighWallSprite);
    addSpriteBoundary(m_LMidWallSprite);
    addSpriteBoundary(m_RHighWallSprite);
    addSpriteBoundary(m_RMidWallSprite);
}
```

### SpawnPlayers / ApplyInitialFormation / HandlePlayerInput

這三個函式直接從 `LevelTwoScene` 複製後改類名。唯一差異是初始位置：

```cpp
void LevelFourScene::ApplyInitialFormation() const {
    const float spawnY = FloorTopY() + PlayerCat::kHalfHeight;
    for (int i = 0; i < static_cast<int>(m_Players.size()); ++i) {
        const auto& pb = m_Players[i];
        if (pb.cat == nullptr) continue;
        const float x = -500.0f + static_cast<float>(i) * 38.0f;
        pb.cat->SetPosition({x, spawnY});
    }
}
```

`SpawnPlayers` 必須包含 LevelTwo 的預設按鍵 fallback：

```cpp
if (i == 0 && pb.key.left == k::UNKNOWN && pb.key.right == k::UNKNOWN) {
    pb.key = KeyboardConfigScene::k_Default1P;
} else if (i == 1 && pb.key.left == k::UNKNOWN && pb.key.right == k::UNKNOWN) {
    pb.key = KeyboardConfigScene::k_Default2P;
}
```

### 子彈速度

速度規則固定如下：

```cpp
float LevelFourScene::CurrentBulletSpeed() const {
    switch (m_JarState) {
        case JarState::Jar0: return kBulletBaseSpeed;
        case JarState::Jar1: return kBulletBaseSpeed * 1.7f;
        case JarState::Jar2: return kBulletBaseSpeed * 1.7f * 2.0f;
        case JarState::Gone: return 0.0f;
    }
    return kBulletBaseSpeed;
}
```

### SpawnBullet

同一時間只允許一顆子彈存在。Jar 消失後停止產生子彈。

```cpp
void LevelFourScene::SpawnBullet() {
    if (m_BulletBody == nullptr || m_JarState == JarState::Gone) return;

    const float spawnX = AppUtil::LeftEdge(*m_ShooterSprite) - kBulletHalf.x - 2.0f;
    const float spawnY = m_JarSprite->GetPosition().y;

    m_BulletBody->SetPosition({spawnX, spawnY});
    m_BulletBody->SetSpeed(CurrentBulletSpeed());
    m_BulletBody->SetActive(true);
    m_BulletActive = true;

    m_BulletSprite->SetPosition(m_BulletBody->GetPosition());
    m_BulletSprite->SetVisible(true);
}
```

### DeactivateBullet / SyncBulletSprite

```cpp
void LevelFourScene::DeactivateBullet() {
    if (m_BulletBody != nullptr) m_BulletBody->SetActive(false);
    if (m_BulletSprite != nullptr) m_BulletSprite->SetVisible(false);
    m_BulletActive = false;
    m_BulletCooldownSec = kBulletRespawnDelaySec;
}

void LevelFourScene::SyncBulletSprite() const {
    if (m_BulletBody == nullptr || m_BulletSprite == nullptr) return;
    if (!m_BulletActive || !m_BulletBody->IsActive()) return;
    m_BulletSprite->SetPosition(m_BulletBody->GetPosition());
}
```

### UpdateBulletSpawner

```cpp
void LevelFourScene::UpdateBulletSpawner(float dtSec) {
    if (m_JarState == JarState::Gone) {
        if (m_BulletActive) DeactivateBullet();
        return;
    }
    if (m_BulletActive) return;

    m_BulletCooldownSec -= dtSec;
    if (m_BulletCooldownSec <= 0.0f) {
        SpawnBullet();
    }
}
```

### HandleBulletHits

碰撞順序固定為玩家、Jar、左邊界。玩家碰到子彈只讓子彈消失，不殺死玩家。

```cpp
void LevelFourScene::HandleBulletHits() {
    if (!m_BulletActive || m_BulletBody == nullptr || !m_BulletBody->IsActive()) return;

    const glm::vec2 bulletPos = m_BulletBody->GetPosition();

    for (const auto& pb : m_Players) {
        if (pb.cat == nullptr || !pb.cat->IsActive()) continue;
        if (AabbOverlap(bulletPos, kBulletHalf, pb.cat->GetPosition(), pb.cat->GetHalfSize())) {
            DeactivateBullet();
            return;
        }
    }

    if (m_JarState != JarState::Gone) {
        const glm::vec2 jarHalf = glm::abs(m_JarSprite->GetScaledSize()) * 0.5f;
        if (AabbOverlap(bulletPos, kBulletHalf, m_JarSprite->GetPosition(), jarHalf)) {
            AdvanceJarState();
            DeactivateBullet();
            return;
        }
    }

    if (bulletPos.x + kBulletHalf.x < kViewLeftX) {
        DeactivateBullet();
    }
}
```

### AdvanceJarState

```cpp
void LevelFourScene::AdvanceJarState() {
    switch (m_JarState) {
        case JarState::Jar0:
            m_JarState = JarState::Jar1;
            m_JarSprite->SetImage(GA_RESOURCE_DIR "/Image/Level_Cover/LevelFourScene/Jar1.png");
            break;
        case JarState::Jar1:
            m_JarState = JarState::Jar2;
            m_JarSprite->SetImage(GA_RESOURCE_DIR "/Image/Level_Cover/LevelFourScene/Jar2.png");
            break;
        case JarState::Jar2:
            m_JarState = JarState::Gone;
            m_JarSprite->SetVisible(false);
            break;
        case JarState::Gone:
            break;
    }
}
```

## Key / Door / Clear 邏輯

下列函式直接從 `LevelTwoScene` 複製後改類名：

1. `TryPickKey`
2. `UpdateKeyFollow`
3. `TryOpenDoorAndClear`
4. `UpdateDoorEntryAndClear`
5. `UpdateTimerText`

唯一必要差異：

`TryPickKey` 第一行必須阻擋 Jar 尚未消失時撿 Key：

```cpp
if (m_JarState != JarState::Gone) return;
```

`TryPickKey` 的 AABB 使用 `kKeyHalf` 和 `m_KeySprite->GetPosition()`，和 LevelTwo 相同。

`TryOpenDoorAndClear` 開門後：

```cpp
m_DoorOpened = true;
m_Actors.Door()->SetImage(GA_RESOURCE_DIR "/Image/Background/door_open.png");
m_KeySprite->SetVisible(false);
```

`UpdateDoorEntryAndClear` 和 LevelTwo 完全相同：每位玩家在 open door 區域按自己的 `up`，就隱藏、停用並移到 `{640.0f, -360.0f}`。所有玩家都進門後：

```cpp
SaveManager::UpdateBestTime(kLevelIndex, m_Session.GetSelectedPlayerCount(), m_ElapsedSec);
m_ClearDone = true;
```

## OnEnter

實作順序固定：

1. 取得並限制玩家數：

```cpp
const int playerCount = std::clamp(m_Session.GetSelectedPlayerCount(), 2, 8);
m_Session.SetSelectedPlayerCount(playerCount);
```

2. 重設狀態：

```cpp
m_PlayerEntered.assign(static_cast<size_t>(playerCount), false);
m_EnteredCount = 0;
m_ClearDone = false;
m_DoorOpened = false;
m_KeyCarrierIdx = -1;
m_ElapsedSec = 0.0f;
m_JarState = JarState::Jar0;
m_BulletActive = false;
m_BulletCooldownSec = kInitialBulletDelaySec;
```

3. 清空 physics world：

```cpp
m_World.Clear();
```

4. 隱藏全域 Header、Floor、TestBox、StartupCats，寫法照 LevelTwo。
5. 呼叫 `SetupSceneVisuals()`。
6. 將背景、Jar、Shooter、Key、BulletSprite、TimerText 加到 root，順序如下：

```cpp
m_Actors.Root().AddChild(m_CeilingSprite);
m_Actors.Root().AddChild(m_FloorSprite);
m_Actors.Root().AddChild(m_LHighWallSprite);
m_Actors.Root().AddChild(m_LMidWallSprite);
m_Actors.Root().AddChild(m_RHighWallSprite);
m_Actors.Root().AddChild(m_RMidWallSprite);
m_Actors.Root().AddChild(m_ShooterSprite);
m_Actors.Root().AddChild(m_KeySprite);
m_Actors.Root().AddChild(m_JarSprite);
m_Actors.Root().AddChild(m_BulletSprite);
m_Actors.Root().AddChild(m_TimerText);
```

7. 呼叫 `UpdateTimerText()`。
8. 呼叫 `SpawnPlayers(playerCount)`。
9. 呼叫 `SetupStaticBoundaries()`。
10. 建立並註冊 bullet body：

```cpp
m_BulletBody = std::make_shared<BulletBody>(m_ShooterSprite->GetPosition(), kBulletHalf);
m_World.Register(m_BulletBody);
```

11. 記 log：

```cpp
LOG_INFO("LevelFourScene::OnEnter players={}", playerCount);
```

## OnExit

順序固定：

1. 移除所有玩家 child，清空 `m_Players`。
2. `m_World.UnfreezeAll();`
3. `m_World.Clear();`
4. 移除 OnEnter 加進 root 的所有 child。
5. `m_BulletBody.reset();`
6. 重設全域 Door：

```cpp
if (m_Actors.Door()) {
    m_Actors.Door()->SetVisible(false);
    m_Actors.Door()->SetImage(GA_RESOURCE_DIR "/Image/Background/door_close.png");
}
```

7. 隱藏 Header/Floor/TestBox，並讓 StartupCats 可見，寫法照 LevelTwo。

## Pause / Resume

完全照 LevelTwo：

```cpp
void LevelFourScene::PauseGameplay() {
    m_World.FreezeAll();
}

void LevelFourScene::ResumeGameplay() {
    m_World.UnfreezeAll();
}
```

## Update

每幀執行順序固定：

```cpp
void LevelFourScene::Update() {
    if (ip::IsKeyDown(k::ESCAPE)) {
        RequestSceneOp({SceneOpType::PushOverlay, SceneId::LevelExit});
        return;
    }

    const float dtSec = Util::Time::GetDeltaTimeMs() / 1000.0f;

    HandlePlayerInput();
    m_World.Update();
    SyncBulletSprite();

    UpdateBulletSpawner(dtSec);
    HandleBulletHits();

    m_ElapsedSec += dtSec;
    UpdateTimerText();

    TryPickKey();
    UpdateKeyFollow();
    TryOpenDoorAndClear();
    UpdateDoorEntryAndClear();

    if (m_ClearDone) {
        RequestSceneOp({SceneOpType::ClearToAndGoTo, SceneId::LevelSelect});
    }
}
```

## AppStart.cpp 註冊

修改 `src/app/AppStart.cpp`。

新增 include：

```cpp
#include "scenes/LevelFourScene.hpp"
```

在選關 scene id 設定區補第 4 關：

```cpp
levelSelectScene->SetLevelSceneId(3, SceneId::Level04);
```

在 Level scene 建立區補：

```cpp
auto levelFourScene = make_unique<LevelFourScene>(services);
```

在 register 區補：

```cpp
m_SceneManager->Register(SceneId::Level04, std::move(levelFourScene));
```

## 驗收清單

1. 專案能成功編譯。
2. 選關畫面第 4 格可進入 `LevelFourScene`。
3. Ceiling 貼齊畫面上邊界，Floor 貼齊畫面下邊界。
4. 左右高牆貼齊 Ceiling 下緣，左右低牆貼齊高牆下緣。
5. Jar0 在 LMidWall 右側且站在 Floor 上方，Key 和 Jar0 中心重疊。
6. Shooter 在 RMidWall 左側且站在 Floor 上方，Door 在 Shooter 右方。
7. 玩家出生在 Jar 右方、Door 左方。
8. 子彈從 Shooter 左側往左飛；碰玩家後子彈消失但玩家不死亡。
9. 子彈碰 Jar0 後 Jar 變 Jar1，下一顆子彈速度為原本 1.7 倍。
10. 子彈碰 Jar1 後 Jar 變 Jar2，下一顆子彈速度為 Jar1 的 2 倍。
11. 子彈碰 Jar2 後 Jar 消失，且不再產生子彈。
12. Jar 消失前玩家碰 Key 不會撿起；Jar 消失後玩家碰 Key 才會撿起。
13. 持 Key 玩家碰 Door 後 Door 變 open，Key 隱藏。
14. 所有玩家在 open Door 區域按自己的 up 鍵後通關並回到 LevelSelect。
15. 通關後 `SaveManager::UpdateBestTime(3, playerCount, m_ElapsedSec)` 有被呼叫。
