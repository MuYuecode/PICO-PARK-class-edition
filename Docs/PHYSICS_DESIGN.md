# 物理模型設計文件

## 架構總覽

```
IPhysicsBody  (純介面，IPhysicsBody.hpp)
│
├── PlayerCat          → BodyType::CHARACTER         ← 已完整實作
├── PushableBox        → BodyType::PUSHABLE_BOX       ← 骨架，待實作
├── PatrolEnemy        → BodyType::PATROL_ENEMY       ← 骨架，待實作
├── MovingPlatform     → BodyType::MOVING_PLATFORM    ← 骨架，待實作
├── ConditionalPlatform → BodyType::CONDITIONAL_PLATFORM ← 骨架，待實作
└── Bullet             → BodyType::BULLET             ← 骨架，待實作

PhysicsWorld  (協調者，PhysicsWorld.hpp / .cpp)
│
├── Register(body)              每次 OnEnter 把物件登記進來
├── Unregister(body)            每次 OnExit 移除
├── Update()                    Scene::Update 每幀呼叫一次
├── GetBodiesOfType(type)       取得所有指定類型的 body
├── GetRopesOf(body)            取得與指定 body 連結的繩索
└── AddRope() / RemoveRope()    繩索管理(StepRopes 尚未實作)

CharacterPhysicsSystem  (純靜態算法，CharacterPhysicsSystem.hpp / .cpp)
│
└── static Update(agents, floor)  只處理 CHARACTER 的移動、重力、踩頭、推擠
                                  PhysicsWorld 負責跨類型事件通知(待完整實作)
```

---

## 各物理模型與介面的對應

### 1. PlayerCat(CHARACTER)— 已完整實作

`PlayerCat` 已完整實作 `IPhysicsBody`(`PlayerCat.hpp`)：

```cpp
class PlayerCat : public AnimatedCharacter, public IPhysicsBody {
    // 已實作所有純虛擬方法：
    //   GetBodyType()  → CHARACTER
    //   GetPosition()  → m_Transform.translation
    //   SetPosition()  → m_Transform.translation = pos
    //   GetHalfSize()  → abs(GetScaledSize()) * 0.5f
    //   GetVelocity()  → m_Velocity
    //   SetVelocity()  → m_Velocity = vel
    //   IsSolid()      → true
    //   IsKinematic()  → false
    //   UseGravity()   → true
    //   OnCollision()  → 空實作(未來可在此處理「被敵人碰到扣血」等效果)
    //   PhysicsUpdate() → 未覆寫(CHARACTER 的移動由 CharacterPhysicsSystem 負責)
};
```

`PhysicsAgent` 的結構即為 `shared_ptr<PlayerCat> actor + PhysicsState state`，與 `IPhysicsBody` 整合方式如下：

```cpp
// LevelScene::OnEnter 登記方式(LocalPlayGameScene 目前尚未使用 m_World)
m_World.Register(agent.actor);   // actor 已實作 IPhysicsBody，直接傳入即可
```

> **目前狀態**：`LocalPlayGameScene` 使用 `CharacterPhysicsSystem::Update` 處理玩家物理，
> 並未呼叫 `m_World.Update()` `PhysicsWorld` 留待跨類型互動(箱子、敵人、平台)實作時啟用 

---

### 2. PushableBox — 骨架(待實作)

**類別骨架：**
```cpp
// include/PushableBox.hpp
class PushableBox : public Character, public IPhysicsBody {
public:
    PushableBox(const std::string& imagePath, int requiredPushers = 2);

    BodyType  GetBodyType()  const override { return BodyType::PUSHABLE_BOX; }
    glm::vec2 GetPosition()  const override { return Character::GetPosition(); }
    void      SetPosition(const glm::vec2& p) override { Character::SetPosition(p); }
    glm::vec2 GetHalfSize()  const override { return glm::abs(GetScaledSize()) * 0.5f; }
    glm::vec2 GetVelocity()  const override { return m_Velocity; }
    void      SetVelocity(const glm::vec2& v) override { m_Velocity = v; }

    bool IsSolid()     const override { return true;  }
    bool IsKinematic() const override { return false; }
    bool UseGravity()  const override { return true;  }

    void SetWorld(PhysicsWorld* world) { m_World = world; }

    void PhysicsUpdate() override {
        // TODO：
        // int pushersRight = m_World->CountCharactersPushing(this, +1);
        // int pushersLeft  = m_World->CountCharactersPushing(this, -1);
        // if (pushersRight >= m_RequiredPushers) m_Velocity.x = kPushSpeed;
        // else if (pushersLeft >= m_RequiredPushers) m_Velocity.x = -kPushSpeed;
        // else m_Velocity.x = 0;
        // SetPosition(GetPosition() + m_Velocity);
    }

private:
    PhysicsWorld* m_World           = nullptr;
    int           m_RequiredPushers = 2;
    glm::vec2     m_Velocity        = {0, 0};
    static constexpr float kPushSpeed = 2.0f;
};
```

---

### 3. PatrolEnemy — 骨架(待實作)

**類別骨架：**
```cpp
// include/PatrolEnemy.hpp
class PatrolEnemy : public AnimatedCharacter, public IPhysicsBody {
public:
    enum class PatrolMode {
        HORIZONTAL,  // 在 [leftX, rightX] 間來回
        WAYPOINTS,   // 依照給定的路徑點列表移動
        PIPE,        // 從水管出來再回去
    };

    PatrolEnemy(const CatAnimPaths& animPaths, PatrolMode mode);

    BodyType  GetBodyType()  const override { return BodyType::PATROL_ENEMY; }
    bool IsSolid()     const override { return true;  }
    bool IsKinematic() const override { return true;  }
    bool UseGravity()  const override { return false; }

    void SetHorizontalRange(float leftX, float rightX);
    void SetWaypoints(const std::vector<glm::vec2>& pts);

    void PhysicsUpdate() override {
        // TODO：依 m_PatrolMode 更新位置
    }

    void OnCollision(const CollisionInfo& info) override {
        // if (info.other->GetBodyType() == BodyType::CHARACTER) { /* 扣血 */ }
    }

private:
    PatrolMode             m_PatrolMode;
    glm::vec2              m_Velocity   = {0, 0};
    std::vector<glm::vec2> m_Waypoints;
    int                    m_WaypointIdx = 0;
    bool                   m_Reversed   = false;
};
```

---

### 4. MovingPlatform — 骨架(待實作)

**類別骨架：**
```cpp
// include/MovingPlatform.hpp
class MovingPlatform : public Character, public IPhysicsBody {
public:
    enum class MoveAxis { VERTICAL, HORIZONTAL };

    MovingPlatform(const std::string& imagePath,
                   glm::vec2 startPos, glm::vec2 endPos,
                   float speed, MoveAxis axis = MoveAxis::VERTICAL);

    BodyType  GetBodyType()  const override { return BodyType::MOVING_PLATFORM; }
    bool IsSolid()     const override { return true;  }
    bool IsKinematic() const override { return true;  }
    bool UseGravity()  const override { return false; }

    void SetWorld(PhysicsWorld* world) { m_World = world; }

    void PhysicsUpdate() override {
        // TODO：更新自身位置 + 攜帶上方的 body
    }

private:
    PhysicsWorld* m_World    = nullptr;
    glm::vec2     m_Start;
    glm::vec2     m_End;
    glm::vec2     m_Velocity = {0, 0};
    MoveAxis      m_Axis;
};
```

---

### 5. ConditionalPlatform — 骨架(待實作)

**繼承 MovingPlatform，覆寫 PhysicsUpdate：**
```cpp
// include/ConditionalPlatform.hpp
class ConditionalPlatform : public MovingPlatform {
public:
    ConditionalPlatform(const std::string& imagePath,
                        glm::vec2 startPos, glm::vec2 endPos,
                        float speed, int requiredRiders = 2);

    BodyType GetBodyType() const override { return BodyType::CONDITIONAL_PLATFORM; }

    void PhysicsUpdate() override {
        // TODO：
        // int riders = m_World->CountBodiesOnTop(this);
        // if (riders >= m_RequiredRiders) MovingPlatform::PhysicsUpdate();
    }

private:
    int   m_RequiredRiders = 2;
    float m_ReturnSpeed    = 2.0f;
};
```

---

### 6. Bullet — 骨架(待實作)

**類別骨架：**
```cpp
// include/Bullet.hpp
class Bullet : public Character, public IPhysicsBody {
public:
    enum class HitEffect { DISAPPEAR, BOUNCE, TRIGGER_STATE };

    Bullet(const std::string& imagePath,
           glm::vec2 startPos, glm::vec2 direction, float speed);

    BodyType  GetBodyType()  const override { return BodyType::BULLET; }
    bool IsSolid()     const override { return false; }
    bool IsKinematic() const override { return true;  }
    bool UseGravity()  const override { return false; }

    void SetWorld(PhysicsWorld* world) { m_World = world; }

    void PhysicsUpdate() override {
        // TODO：移動 + QueryOverlapping 碰撞偵測
    }

    void OnCollision(const CollisionInfo& info) override {
        if (info.other->GetBodyType() == BodyType::CHARACTER) {
            SetActive(false);
        }
    }

private:
    PhysicsWorld*                           m_World = nullptr;
    glm::vec2                               m_Velocity;
    std::unordered_map<BodyType, HitEffect> m_HitEffects;
};
```

---

### 7. Rope(繩索約束)

繩索不是獨立的 `IPhysicsBody`，而是由 `PhysicsWorld` 管理的**約束(Constraint)**，
以 `RopeConstraint` 結構記錄 

**Scene 端的使用方式：**
```cpp
// 兩個角色建立繩索連接
m_World.AddRope(playerA.actor.get(), playerB.actor.get(),
                /*maxLen=*/200.0f, /*friction=*/0.5f);

// 解除
m_World.RemoveRope(playerA.actor.get(), playerB.actor.get());
```

---

## PhysicsWorld 目前實作狀態

| 功能 | 狀態 |
|------|------|
| `Register` / `Unregister` / `Clear` | ✅ 已實作 |
| `AddRope` / `RemoveRope` / `GetRopesOf` | ✅ 結構已實作 |
| `GetBodiesOfType` | ✅ 已實作 |
| `PurgeExpired`(清理失效弱引用) | ✅ 已實作 |
| `StepPhysicsUpdate`(呼叫各 body 的 `PhysicsUpdate`) | ✅ 已實作 |
| `StepRopes`(繩索力解析) | ❌ TODO |
| `StepCollisions`(AABB 碰撞廣播) | ❌ TODO |
| `CountCharactersPushing` | ❌ TODO(已有骨架) |
| `CountBodiesOnTop` | ❌ TODO(已有骨架) |
| `QueryOverlapping` | ❌ TODO(已有骨架) |

---

## Scene 使用方式範例(以未來的 LevelOneScene 為例)

```cpp
class LevelOneScene : public Scene {
    PhysicsWorld              m_World;        // 管理跨類型互動
    std::vector<PhysicsAgent> m_Agents;

    std::vector<std::shared_ptr<PushableBox>>        m_Boxes;
    std::vector<std::shared_ptr<MovingPlatform>>     m_Platforms;
    std::vector<std::shared_ptr<PatrolEnemy>>        m_Enemies;

    float m_ElapsedSeconds = 0.0f;   // 計時(用於最佳時間紀錄)

    void OnEnter() override {
        m_ElapsedSeconds = 0.0f;
        // 登記所有物件
        for (auto& a : m_Agents)    m_World.Register(a.actor);
        for (auto& b : m_Boxes)     { b->SetWorld(&m_World); m_World.Register(b); }
        for (auto& p : m_Platforms) { p->SetWorld(&m_World); m_World.Register(p); }
        for (auto& e : m_Enemies)   m_World.Register(e);
    }

    void OnExit() override {
        m_World.Clear();
        m_Agents.clear();
    }

    Scene* Update() override {
        m_ElapsedSeconds += /* deltaTime */;

        // 1. 讀取輸入 → 寫入 agent.state.moveDir / 呼叫 ApplyJump
        // 2. CHARACTER 的細緻物理(推擠、踩頭)，直接呼叫靜態方法
        CharacterPhysicsSystem::Update(m_Agents, m_Ctx.Floor);
        // 3. 其他物件的自驅動 + 跨類型碰撞
        m_World.Update();

        // 4. 關卡完成判定
        if (/* 全員到達終點 */) {
            // 更新最佳時間並持久化
            SaveManager::UpdateBestTime(
                0,                              // levelIdx(0 = Level 1)
                m_Ctx.SelectedPlayerCount,      // playerCount
                m_ElapsedSeconds);              // 本次耗時(秒)
            return m_LevelSelectScene;          // 返回關卡選擇
        }

        return nullptr;
    }
};
```

> **注意**：`LevelSelectScene` 不使用物理系統，它是純 UI 選單場景，
> 只做 2D 網格選擇、滑鼠懸停偵測和存檔讀取，不持有任何 `PhysicsWorld` 或 `CharacterPhysicsSystem` 

---

## CharacterPhysicsSystem 物理常數

| 常數 | 值 | 說明 |
|------|-----|------|
| `kGroundMoveSpeed` | 5.0f | 地面移動速度(像素/幀) |
| `kRunOnPlayerSpeed` | 6.2f | 站在其他角色頭上時的移動速度 |
| `kJumpForce` | 11.0f | 起跳初速(velocityY) |
| `kGravity` | 0.75f | 每幀重力加速度(velocityY -= kGravity) |
| `kScreenHalfW` | 640.0f | 畫面半寬(1280 × 720) |
| `kScreenHalfH` | 360.0f | 畫面半高 |
| `HalfWidth()` | 18.0f | 碰撞箱半寬(固定值，避免動畫切換造成突變) |
| `HalfHeight()` | 23.0f | 碰撞箱半高 |
