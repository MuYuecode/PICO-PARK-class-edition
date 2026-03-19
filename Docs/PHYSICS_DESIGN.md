
# 物理模型設計文件

## 架構總覽

```
IPhysicsBody  (純介面)
│
├── PlayerCat          → BodyType::CHARACTER
├── PushableBox        → BodyType::PUSHABLE_BOX
├── PatrolEnemy        → BodyType::PATROL_ENEMY
├── MovingPlatform     → BodyType::MOVING_PLATFORM
├── ConditionalPlatform → BodyType::CONDITIONAL_PLATFORM
├── Bullet             → BodyType::BULLET
└── (RopeEndpoint 是 IPhysicsBody 的 wrapper，不是獨立類別)

PhysicsWorld  (協調者，Scene 持有一個)
│
├── Register(body)              每次 OnEnter 把物件登記進來
├── Unregister(body)            每次 OnExit 移除
├── Update()                    Scene::Update 每幀呼叫一次
├── CountCharactersPushing()    → 供 PushableBox::PhysicsUpdate 使用
├── CountBodiesOnTop()          → 供 ConditionalPlatform::PhysicsUpdate 使用
├── QueryOverlapping()          → 供 Bullet::PhysicsUpdate 使用
└── AddRope() / StepRopes()     → 供繩索系統使用

CharacterPhysicsSystem  (純算法，Scene 持有一個)
│
└── Update(agents, floor)       只處理 CHARACTER 的移動、重力、踩頭、推擠
                                PhysicsWorld 負責跨類型事件通知
```

---

## 各物理模型與介面的對應

### 1. PlayerCat（CHARACTER）

`PlayerCat` 已完整實作 `IPhysicsBody`（`PlayerCat.hpp`）：

```cpp
class PlayerCat : public AnimatedCharacter, public IPhysicsBody {
    // 已實作：GetBodyType / GetPosition / SetPosition / GetHalfSize
    //         GetVelocity / SetVelocity / IsSolid / IsKinematic / UseGravity
    // OnCollision：目前為空，未來可在此處理「被敵人碰到扣血」等效果
    // PhysicsUpdate：留空，CHARACTER 的移動由 CharacterPhysicsSystem 負責
};
```

`PhysicsAgent` 的結構即為 `shared_ptr<PlayerCat> actor + PhysicsState state`，與 `IPhysicsBody` 整合方式如下：

```cpp
// 未來 LevelScene::OnEnter 登記方式（目前 LocalPlayGameScene 尚未使用 m_World）
m_World.Register(agent.actor);   // actor 已實作 IPhysicsBody，直接傳入即可
```

---

### 2. PushableBox

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

    // 需要 PhysicsWorld 的指標才能查詢「幾人在推」
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
    PhysicsWorld* m_World          = nullptr;
    int           m_RequiredPushers = 2;
    glm::vec2     m_Velocity       = {0, 0};
    static constexpr float kPushSpeed = 2.0f;
};
```

---

### 3. PatrolEnemy

**類別骨架：**
```cpp
// include/PatrolEnemy.hpp
class PatrolEnemy : public AnimatedCharacter, public IPhysicsBody {
public:
    // 巡邏模式
    enum class PatrolMode {
        HORIZONTAL,  // 在 [leftX, rightX] 間來回
        WAYPOINTS,   // 依照給定的路徑點列表移動
        PIPE,        // 從水管出來再回去
    };

    PatrolEnemy(const CatAnimPaths& animPaths, PatrolMode mode);

    BodyType  GetBodyType()  const override { return BodyType::PATROL_ENEMY; }
    glm::vec2 GetPosition()  const override { return m_Transform.translation; }
    void      SetPosition(const glm::vec2& p) override { m_Transform.translation = p; }
    glm::vec2 GetHalfSize()  const override { return glm::abs(GetScaledSize()) * 0.5f; }
    glm::vec2 GetVelocity()  const override { return m_Velocity; }
    void      SetVelocity(const glm::vec2& v) override { m_Velocity = v; }

    bool IsSolid()     const override { return true;  }
    bool IsKinematic() const override { return true;  }  // 外部控制路徑
    bool UseGravity()  const override { return false; }

    // 設定巡邏範圍
    void SetHorizontalRange(float leftX, float rightX);
    void SetWaypoints(const std::vector<glm::vec2>& pts);

    void PhysicsUpdate() override {
        // TODO：依 m_PatrolMode 更新位置
        // HORIZONTAL：到達邊界時翻轉方向
        // WAYPOINTS：依序前進到下一個路徑點
        // PIPE：從出口出來移動一段後回去
    }

    // 碰到玩家 → 觸發傷害（由遊戲邏輯決定實際效果）
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

### 4. MovingPlatform

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
    glm::vec2 GetPosition()  const override { return Character::GetPosition(); }
    void      SetPosition(const glm::vec2& p) override { Character::SetPosition(p); }
    glm::vec2 GetHalfSize()  const override { return glm::abs(GetScaledSize()) * 0.5f; }
    glm::vec2 GetVelocity()  const override { return m_Velocity; }
    void      SetVelocity(const glm::vec2& v) override { m_Velocity = v; }

    bool IsSolid()     const override { return true;  }
    bool IsKinematic() const override { return true;  }
    bool UseGravity()  const override { return false; }

    // 需要 PhysicsWorld 取得「站在上面的 body」
    void SetWorld(PhysicsWorld* world) { m_World = world; }

    void PhysicsUpdate() override {
        // TODO：
        // 1. 更新自身位置（在 start/end 間來回）
        // glm::vec2 delta = m_Velocity;
        // SetPosition(GetPosition() + delta);
        // 在端點反轉 m_Velocity
        //
        // 2. 攜帶上方的 body（和 CharacterPhysicsSystem 的 supportIndex 機制相同）
        // auto riders = m_World->GetBodiesOfType + IsOnTop 過濾
        // for (auto* rider : riders) rider->SetPosition(rider->GetPosition() + delta);
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

### 5. ConditionalPlatform

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
        // if (riders >= m_RequiredRiders) {
        //     MovingPlatform::PhysicsUpdate();  // 正常移動
        // } else {
        //     // 條件不達成：回到原點
        //     glm::vec2 toOrigin = m_Start - GetPosition();
        //     if (glm::length(toOrigin) > 1.0f) {
        //         SetPosition(GetPosition() + normalize(toOrigin) * m_ReturnSpeed);
        //     }
        // }
    }

private:
    int   m_RequiredRiders = 2;
    float m_ReturnSpeed    = 2.0f;
};
```

---

### 6. Bullet

**類別骨架：**
```cpp
// include/Bullet.hpp
class Bullet : public Character, public IPhysicsBody {
public:
    // 碰到不同物件的效果
    enum class HitEffect { DISAPPEAR, BOUNCE, TRIGGER_STATE };

    Bullet(const std::string& imagePath,
           glm::vec2 startPos, glm::vec2 direction, float speed);

    BodyType  GetBodyType()  const override { return BodyType::BULLET; }
    glm::vec2 GetPosition()  const override { return Character::GetPosition(); }
    void      SetPosition(const glm::vec2& p) override { Character::SetPosition(p); }
    glm::vec2 GetHalfSize()  const override { return glm::abs(GetScaledSize()) * 0.5f; }
    glm::vec2 GetVelocity()  const override { return m_Velocity; }
    void      SetVelocity(const glm::vec2& v) override { m_Velocity = v; }

    bool IsSolid()     const override { return false; }  // 穿透式偵測
    bool IsKinematic() const override { return true;  }
    bool UseGravity()  const override { return false; }

    // 設定對不同 BodyType 的碰撞效果
    void SetHitEffect(BodyType targetType, HitEffect effect);

    void SetWorld(PhysicsWorld* world) { m_World = world; }

    void PhysicsUpdate() override {
        // TODO：
        // 1. 移動
        // SetPosition(GetPosition() + m_Velocity);
        //
        // 2. 查詢是否碰到東西
        // auto hits = m_World->QueryOverlapping(this);
        // for (auto* hit : hits) {
        //     auto effect = m_HitEffects[hit->GetBodyType()];
        //     switch (effect) {
        //         case HitEffect::DISAPPEAR:     SetActive(false); break;
        //         case HitEffect::BOUNCE:        m_Velocity = -m_Velocity; break;
        //         case HitEffect::TRIGGER_STATE: /* 通知目標改變狀態 */ break;
        //     }
        // }
    }

    void OnCollision(const CollisionInfo& info) override {
        // 碰到 CHARACTER → 立即停用自己
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

### 7. Rope（繩索約束）

繩索不是獨立的 `IPhysicsBody`，而是由 `PhysicsWorld` 管理的**約束（Constraint）**。

**Scene 端的使用方式：**
```cpp
// 兩個角色建立繩索連接
m_World.AddRope(playerA.actor.get(), playerB.actor.get(),
                /*maxLen=*/200.0f, /*friction=*/0.5f);

// 解除
m_World.RemoveRope(playerA.actor.get(), playerB.actor.get());
```

**PhysicsWorld::StepRopes 的力分解邏輯（TODO 中已有骨架）：**
```
dir  = normalize(posA - posB)     // 從 B 指向 A 的方向
拉力 = (distance - maxLen) * stiffness  // 僅在超出時才有
A 受到 -dir 方向的力（被 B 往 B 的方向拉）
B 受到 +dir 方向的力（被 A 往 A 的方向拉）
速度衰減 *= (1 - friction)         // friction 越高，移動越慢
```

x 軸分量處理水平拖拽，y 軸分量處理懸崖拉力，不需要額外區分。

---

## Scene 使用方式範例（以未來的 LevelScene 為例）

```cpp
class LevelScene : public Scene {
    CharacterPhysicsSystem   m_CharPhysics;  // 只處理 CHARACTER
    PhysicsWorld             m_World;        // 管理跨類型互動
    std::vector<PhysicsAgent> m_Agents;

    std::vector<std::shared_ptr<PushableBox>>        m_Boxes;
    std::vector<std::shared_ptr<MovingPlatform>>     m_Platforms;
    std::vector<std::shared_ptr<PatrolEnemy>>        m_Enemies;

    void OnEnter() override {
        // 登記所有物件
        for (auto& a : m_Agents)    m_World.Register(a.actor);
        for (auto& b : m_Boxes)     { b->SetWorld(&m_World); m_World.Register(b); }
        for (auto& p : m_Platforms) { p->SetWorld(&m_World); m_World.Register(p); }
        for (auto& e : m_Enemies)   m_World.Register(e);
    }

    Scene* Update() override {
        // 1. 讀取輸入 → 寫入 agent.state.moveDir / velocityY
        // 2. CHARACTER 的細緻物理（推擠、踩頭）
        m_CharPhysics.Update(m_Agents, m_Ctx.Floor);
        // 3. 其他物件的自驅動 + 跨類型碰撞
        m_World.Update();
        return nullptr;
    }
};
```

---

## 補充事項（實作時需要決定）

- **PhysicsAgent 與 IPhysicsBody 的整合**：目前 `PlayerCat` 尚未實作 `IPhysicsBody`，需在 `PlayerCat.hpp` 補上繼承（見上方 CHARACTER 段落）。
- **`SetWorld()` 注入**：PushableBox / MovingPlatform / Bullet 需要 PhysicsWorld 指標，採用 setter 注入而非建構子注入，避免初始化順序問題。
- **CMakeLists.txt**：新增 `PushableBox.cpp`、`PatrolEnemy.cpp`、`MovingPlatform.cpp`、`ConditionalPlatform.cpp`、`Bullet.cpp`、`PhysicsWorld.cpp` 到 source 清單。
