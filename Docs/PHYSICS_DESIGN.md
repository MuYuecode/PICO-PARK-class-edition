# Physics System Design

## Architecture Overview

```
IPhysicsBody  (pure interface, IPhysicsBody.hpp)
│
├── PlayerCat            → BodyType::CHARACTER          ✅ fully implemented
├── PushableBox          → BodyType::PUSHABLE_BOX       ⬜ skeleton, TODO
├── PatrolEnemy          → BodyType::PATROL_ENEMY       ⬜ skeleton, TODO
├── MovingPlatform       → BodyType::MOVING_PLATFORM    ⬜ skeleton, TODO
├── ConditionalPlatform  → BodyType::CONDITIONAL_PLATFORM ⬜ skeleton, TODO
└── Bullet               → BodyType::BULLET             ⬜ skeleton, TODO

PhysicsWorld  (coordinator, PhysicsWorld.hpp / .cpp)
├── Register / Unregister / Clear
├── Update()                  call once per frame from Scene::Update
├── GetBodiesOfType(type)
├── GetRopesOf(body)
└── AddRope / RemoveRope      rope constraints (StepRopes not yet implemented)

CharacterPhysicsSystem  (pure-static algorithm, CharacterPhysicsSystem.hpp / .cpp)
└── static Update(agents, floor)   handles CHARACTER movement, gravity, stacking, pushing
                                   PhysicsWorld handles cross-type events (partial)
```

---

## Body Types and Interface Implementation

### 1. PlayerCat (CHARACTER) — Fully Implemented

`PlayerCat` implements all `IPhysicsBody` pure-virtual methods:

```cpp
class PlayerCat : public AnimatedCharacter, public IPhysicsBody {
    // GetBodyType()  → CHARACTER
    // GetPosition()  → m_Transform.translation
    // SetPosition()  → m_Transform.translation = pos
    // GetHalfSize()  → abs(GetScaledSize()) * 0.5f
    // GetVelocity()  → m_Velocity
    // SetVelocity()  → m_Velocity = vel
    // IsSolid()      → true
    // IsKinematic()  → false
    // UseGravity()   → true
    // OnCollision()  → empty (extend for damage, etc.)
    // PhysicsUpdate() → not overridden; CHARACTER movement handled by CharacterPhysicsSystem
};
```

> **Current status:** `LocalPlayGameScene` uses `CharacterPhysicsSystem::Update` directly for player physics. `PhysicsWorld::Update()` is not called from `LocalPlayGameScene` — it is reserved for future cross-type interactions (boxes, enemies, platforms).

Registration example (for future level scenes):
```cpp
m_World.Register(agent.actor);   // PlayerCat already implements IPhysicsBody
```

---

### 2. PushableBox — Skeleton (TODO)

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
        // TODO:
        // int r = m_World->CountCharactersPushing(this, +1);
        // int l = m_World->CountCharactersPushing(this, -1);
        // if (r >= m_RequiredPushers)      m_Velocity.x =  kPushSpeed;
        // else if (l >= m_RequiredPushers) m_Velocity.x = -kPushSpeed;
        // else                             m_Velocity.x =  0;
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

### 3. PatrolEnemy — Skeleton (TODO)

```cpp
// include/PatrolEnemy.hpp
class PatrolEnemy : public AnimatedCharacter, public IPhysicsBody {
public:
    enum class PatrolMode { HORIZONTAL, WAYPOINTS, PIPE };

    PatrolEnemy(const CatAnimPaths& animPaths, PatrolMode mode);

    BodyType GetBodyType()  const override { return BodyType::PATROL_ENEMY; }
    bool IsSolid()     const override { return true;  }
    bool IsKinematic() const override { return true;  }
    bool UseGravity()  const override { return false; }

    void SetHorizontalRange(float leftX, float rightX);
    void SetWaypoints(const std::vector<glm::vec2>& pts);

    void PhysicsUpdate() override {
        // TODO: update position by m_PatrolMode
    }

    void OnCollision(const CollisionInfo& info) override {
        // if (info.other->GetBodyType() == BodyType::CHARACTER) { /* damage */ }
    }

private:
    PatrolMode             m_PatrolMode;
    glm::vec2              m_Velocity    = {0, 0};
    std::vector<glm::vec2> m_Waypoints;
    int                    m_WaypointIdx = 0;
    bool                   m_Reversed    = false;
};
```

---

### 4. MovingPlatform — Skeleton (TODO)

```cpp
// include/MovingPlatform.hpp
class MovingPlatform : public Character, public IPhysicsBody {
public:
    enum class MoveAxis { VERTICAL, HORIZONTAL };

    MovingPlatform(const std::string& imagePath,
                   glm::vec2 startPos, glm::vec2 endPos,
                   float speed, MoveAxis axis = MoveAxis::VERTICAL);

    BodyType GetBodyType()  const override { return BodyType::MOVING_PLATFORM; }
    bool IsSolid()     const override { return true;  }
    bool IsKinematic() const override { return true;  }
    bool UseGravity()  const override { return false; }

    void SetWorld(PhysicsWorld* world) { m_World = world; }

    void PhysicsUpdate() override {
        // TODO: move between m_Start / m_End + carry riders
    }

private:
    PhysicsWorld* m_World    = nullptr;
    glm::vec2     m_Start, m_End;
    glm::vec2     m_Velocity = {0, 0};
    MoveAxis      m_Axis;
};
```

---

### 5. ConditionalPlatform — Skeleton (TODO)

Extends `MovingPlatform`; only activates when enough riders are on top.

```cpp
// include/ConditionalPlatform.hpp
class ConditionalPlatform : public MovingPlatform {
public:
    ConditionalPlatform(const std::string& imagePath,
                        glm::vec2 startPos, glm::vec2 endPos,
                        float speed, int requiredRiders = 2);

    BodyType GetBodyType() const override { return BodyType::CONDITIONAL_PLATFORM; }

    void PhysicsUpdate() override {
        // TODO:
        // if (m_World->CountBodiesOnTop(this) >= m_RequiredRiders)
        //     MovingPlatform::PhysicsUpdate();
    }

private:
    int   m_RequiredRiders = 2;
    float m_ReturnSpeed    = 2.0f;
};
```

---

### 6. Bullet — Skeleton (TODO)

```cpp
// include/Bullet.hpp
class Bullet : public Character, public IPhysicsBody {
public:
    enum class HitEffect { DISAPPEAR, BOUNCE, TRIGGER_STATE };

    Bullet(const std::string& imagePath,
           glm::vec2 startPos, glm::vec2 direction, float speed);

    BodyType GetBodyType()  const override { return BodyType::BULLET; }
    bool IsSolid()     const override { return false; }
    bool IsKinematic() const override { return true;  }
    bool UseGravity()  const override { return false; }

    void SetWorld(PhysicsWorld* world) { m_World = world; }

    void PhysicsUpdate() override {
        // TODO: move + QueryOverlapping for collision detection
    }

    void OnCollision(const CollisionInfo& info) override {
        if (info.other->GetBodyType() == BodyType::CHARACTER)
            SetActive(false);
    }

private:
    PhysicsWorld*                           m_World = nullptr;
    glm::vec2                               m_Velocity;
    std::unordered_map<BodyType, HitEffect> m_HitEffects;
};
```

---

### 7. Rope Constraint

Ropes are **constraints**, not `IPhysicsBody` instances. Managed by `PhysicsWorld` as `RopeConstraint` structs.

```cpp
// Add rope between two bodies
m_World.AddRope(playerA.actor.get(), playerB.actor.get(),
                /*maxLen=*/200.0f, /*friction=*/0.5f);

m_World.RemoveRope(playerA.actor.get(), playerB.actor.get());
```

`StepRopes()` is not yet implemented (see status table below).

---

## PhysicsWorld Implementation Status

| Feature | Status |
|---------|--------|
| `Register` / `Unregister` / `Clear` | ✅ |
| `AddRope` / `RemoveRope` / `GetRopesOf` | ✅ (structure only) |
| `GetBodiesOfType` | ✅ |
| `PurgeExpired` (clean stale weak_ptrs, every 60 frames) | ✅ |
| `StepPhysicsUpdate` (calls `PhysicsUpdate()` on each active body) | ✅ |
| `StepRopes` (rope force resolution) | ❌ TODO |
| `StepCollisions` (AABB broadcast via `OnCollision`) | ❌ TODO |
| `CountCharactersPushing` | ❌ TODO (skeleton commented in source) |
| `CountBodiesOnTop` | ❌ TODO (skeleton commented in source) |
| `QueryOverlapping` | ❌ TODO (skeleton commented in source) |

---

## Future Level Scene Usage Pattern

```cpp
class LevelOneScene : public Scene {
    PhysicsWorld              m_World;
    std::vector<PhysicsAgent> m_Agents;
    std::vector<std::shared_ptr<PushableBox>>    m_Boxes;
    std::vector<std::shared_ptr<MovingPlatform>> m_Platforms;
    std::vector<std::shared_ptr<PatrolEnemy>>    m_Enemies;
    float m_ElapsedSeconds = 0.0f;

    void OnEnter() override {
        m_ElapsedSeconds = 0.0f;
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
        m_ElapsedSeconds += deltaTime;

        // Step 1: read input → set agent.state.moveDir / ApplyJump
        // Step 2: CHARACTER fine physics (pushing, stacking)
        CharacterPhysicsSystem::Update(m_Agents, m_Ctx.Floor);
        // Step 3: other object self-drive + cross-type collisions
        m_World.Update();

        if (/* all players at goal */) {
            SaveManager::UpdateBestTime(0, m_Ctx.SelectedPlayerCount, m_ElapsedSeconds);
            return m_LevelSelectScene;
        }
        return nullptr;
    }
};
```

> `LevelSelectScene` is a pure UI scene: 2D grid navigation, mouse hover, save-data read. It holds no `PhysicsWorld` or `CharacterPhysicsSystem`.

---

## CharacterPhysicsSystem Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `kGroundMoveSpeed` | 5.0f | ground movement speed (px/frame) |
| `kRunOnPlayerSpeed` | 6.2f | speed when standing on another character |
| `kJumpForce` | 11.0f | initial jump velocityY |
| `kGravity` | 0.75f | velocityY -= kGravity each frame |
| `kScreenHalfW` | 640.0f | half of 1280×720 screen width |
| `kScreenHalfH` | 360.0f | half of 1280×720 screen height |
| `HalfWidth()` | 18.0f | collision box half-width (fixed, avoids jitter on animation switch) |
| `HalfHeight()` | 23.0f | collision box half-height |
