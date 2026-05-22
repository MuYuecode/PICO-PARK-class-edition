#ifndef LEVEL_THREE_SCENE_PHYSICS_BODIES_HPP
#define LEVEL_THREE_SCENE_PHYSICS_BODIES_HPP

#include <algorithm>

#include "Util/Time.hpp"
#include "scenes/LevelThreeScene.hpp"

class LevelThreeScene::MovingLiftBody final : public IPhysicsBody {
public:
    MovingLiftBody(const glm::vec2& pos, const glm::vec2& half,
                   float lowY, float highY, float speed)
        : m_Pos(pos),
          m_Half(glm::abs(half)),
          m_LowY(std::min(lowY, highY)),
          m_HighY(std::max(lowY, highY)),
          m_Speed(std::max(1.0f, speed)) {}

    [[nodiscard]] const PhysicsBodyTraits& GetPhysicsTraits() const override {
        static const PhysicsBodyTraits kTraits{BodyType::MOVING_PLATFORM, false, false};
        return kTraits;
    }
    [[nodiscard]] glm::vec2 GetPosition() const override { return m_Pos; }
    void SetPosition(const glm::vec2& pos) override { m_Pos = pos; }
    [[nodiscard]] glm::vec2 GetHalfSize() const override { return m_Half; }

    [[nodiscard]] bool IsSolid() const override { return true; }
    [[nodiscard]] bool IsKinematic() const override { return true; }
    [[nodiscard]] int GetMoveDir() const override { return 0; }

    [[nodiscard]] bool IsActive() const override { return IPhysicsBody::IsActive(); }
    void SetActive(bool active) override { IPhysicsBody::SetActive(active); }
    [[nodiscard]] bool IsFrozen() const override { return IPhysicsBody::IsFrozen(); }
    void Freeze() override { IPhysicsBody::Freeze(); }
    void Unfreeze() override { IPhysicsBody::Unfreeze(); }

    void PhysicsUpdate() override {
        const float dtSec = Util::Time::GetDeltaTimeMs() / 1000.0f;
        const float step  = m_Speed * std::max(0.0f, dtSec) * m_Direction;
        float targetY = m_Pos.y + step;

        if (targetY >= m_HighY) {
            targetY = m_HighY;
            m_Direction = -1.0f;
        } else if (targetY <= m_LowY) {
            targetY = m_LowY;
            m_Direction = 1.0f;
        }

        m_Desired = {0.0f, targetY - m_Pos.y};
    }

    [[nodiscard]] glm::vec2 GetDesiredDelta() const override { return m_Desired; }

    void ApplyResolvedDelta(const glm::vec2& delta) override {
        m_Pos += delta;
    }

    void OnCollision(const CollisionInfo& /*info*/) override {}

    void PostUpdate() override {}

private:
    glm::vec2 m_Pos;
    glm::vec2 m_Half;
    glm::vec2 m_Desired = {0.0f, 0.0f};

    float m_LowY = 0.0f;
    float m_HighY = 0.0f;
    float m_Speed = 80.0f;
    float m_Direction = 1.0f;
};

class LevelThreeScene::PatrolMobBody final : public IPhysicsBody {
public:
    PatrolMobBody(const glm::vec2& pos, const glm::vec2& half,
                  float minX, float maxX, float speed)
        : m_Pos(pos),
          m_Half(glm::abs(half)),
          m_MinX(std::min(minX, maxX)),
          m_MaxX(std::max(minX, maxX)),
          m_Speed(std::max(1.0f, speed)) {}

    [[nodiscard]] const PhysicsBodyTraits& GetPhysicsTraits() const override {
        static const PhysicsBodyTraits kTraits{BodyType::PATROL_ENEMY, false, false};
        return kTraits;
    }
    [[nodiscard]] glm::vec2 GetPosition() const override { return m_Pos; }
    void SetPosition(const glm::vec2& pos) override { m_Pos = pos; }
    [[nodiscard]] glm::vec2 GetHalfSize() const override { return m_Half; }

    [[nodiscard]] bool IsSolid() const override { return true; }
    [[nodiscard]] bool IsKinematic() const override { return true; }
    [[nodiscard]] int GetMoveDir() const override { return 0; }

    [[nodiscard]] bool IsActive() const override { return IPhysicsBody::IsActive(); }
    void SetActive(bool active) override { IPhysicsBody::SetActive(active); }
    [[nodiscard]] bool IsFrozen() const override { return IPhysicsBody::IsFrozen(); }
    void Freeze() override { IPhysicsBody::Freeze(); }
    void Unfreeze() override { IPhysicsBody::Unfreeze(); }

    void PhysicsUpdate() override {
        const float dtSec = Util::Time::GetDeltaTimeMs() / 1000.0f;
        const float step  = m_Speed * std::max(0.0f, dtSec) * m_Direction;
        float targetX = m_Pos.x + step;

        if (targetX >= m_MaxX) {
            targetX = m_MaxX;
            m_Direction = -1.0f;
        } else if (targetX <= m_MinX) {
            targetX = m_MinX;
            m_Direction = 1.0f;
        }

        m_Desired = {targetX - m_Pos.x, 0.0f};
    }

    [[nodiscard]] glm::vec2 GetDesiredDelta() const override { return m_Desired; }
    [[nodiscard]] int GetFacingDir() const { return (m_Direction >= 0.0f) ? 1 : -1; }

    void ApplyResolvedDelta(const glm::vec2& delta) override {
        m_Pos += delta;
    }

    void OnCollision(const CollisionInfo& /*info*/) override {}

    void PostUpdate() override {}

private:
    glm::vec2 m_Pos;
    glm::vec2 m_Half;
    glm::vec2 m_Desired = {0.0f, 0.0f};

    float m_MinX = 0.0f;
    float m_MaxX = 0.0f;
    float m_Speed = 90.0f;
    float m_Direction = -1.0f;
};

class LevelThreeScene::PipeMobBody final : public IPhysicsBody {
public:
    PipeMobBody(const glm::vec2& pos, const glm::vec2& half,
                float hiddenY, float visibleY, float speed)
        : m_Pos(pos),
          m_Half(glm::abs(half)),
          m_HiddenY(hiddenY),
          m_VisibleY(visibleY),
          m_Speed(std::max(1.0f, speed)) {
        m_Pos.y = m_HiddenY;
    }

    [[nodiscard]] const PhysicsBodyTraits& GetPhysicsTraits() const override {
        static const PhysicsBodyTraits kTraits{BodyType::PATROL_ENEMY, false, false};
        return kTraits;
    }
    [[nodiscard]] glm::vec2 GetPosition() const override { return m_Pos; }
    void SetPosition(const glm::vec2& pos) override { m_Pos = pos; }
    [[nodiscard]] glm::vec2 GetHalfSize() const override { return m_Half; }

    [[nodiscard]] bool IsSolid() const override { return true; }
    [[nodiscard]] bool IsKinematic() const override { return true; }
    [[nodiscard]] int GetMoveDir() const override { return 0; }

    [[nodiscard]] bool IsActive() const override { return IPhysicsBody::IsActive(); }
    void SetActive(bool active) override { IPhysicsBody::SetActive(active); }
    [[nodiscard]] bool IsFrozen() const override { return IPhysicsBody::IsFrozen(); }
    void Freeze() override { IPhysicsBody::Freeze(); }
    void Unfreeze() override { IPhysicsBody::Unfreeze(); }

    void PhysicsUpdate() override {
        const float dtSec = Util::Time::GetDeltaTimeMs() / 1000.0f;
        const float maxStep = m_Speed * std::max(0.0f, dtSec);
        m_Desired = {0.0f, 0.0f};

        switch (m_State) {
            case State::HiddenWait:
                m_Timer += dtSec;
                if (m_Timer >= 5.0f) {
                    m_Timer = 0.0f;
                    m_State = State::Rising;
                }
                break;
            case State::Rising: {
                const float dy = std::min(maxStep, m_VisibleY - m_Pos.y);
                m_Desired.y = std::max(0.0f, dy);
                if (m_Pos.y + m_Desired.y >= m_VisibleY - 0.01f) {
                    m_State = State::VisibleWait;
                    m_Timer = 0.0f;
                }
                break;
            }
            case State::VisibleWait:
                m_Timer += dtSec;
                if (m_Timer >= 3.0f) {
                    m_Timer = 0.0f;
                    m_State = State::Descending;
                }
                break;
            case State::Descending: {
                const float dy = std::min(maxStep, m_Pos.y - m_HiddenY);
                m_Desired.y = -std::max(0.0f, dy);
                if (m_Pos.y + m_Desired.y <= m_HiddenY + 0.01f) {
                    m_State = State::HiddenWait;
                    m_Timer = 0.0f;
                }
                break;
            }
        }
    }

    [[nodiscard]] glm::vec2 GetDesiredDelta() const override { return m_Desired; }

    void ApplyResolvedDelta(const glm::vec2& delta) override {
        m_Pos += delta;
    }

    void OnCollision(const CollisionInfo& /*info*/) override {}

    void PostUpdate() override {}

private:
    enum class State {
        HiddenWait,
        Rising,
        VisibleWait,
        Descending,
    };

    glm::vec2 m_Pos;
    glm::vec2 m_Half;
    glm::vec2 m_Desired = {0.0f, 0.0f};

    float m_HiddenY = 0.0f;
    float m_VisibleY = 0.0f;
    float m_Speed = 35.0f;

    State m_State = State::HiddenWait;
    float m_Timer = 0.0f;
};

#endif // LEVEL_THREE_SCENE_PHYSICS_BODIES_HPP


