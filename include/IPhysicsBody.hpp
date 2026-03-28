//
// Created by cody2 on 2026/3/19.
//

#ifndef PHYSICS_BODY_HPP
#define PHYSICS_BODY_HPP

#include "pch.hpp"

class IPhysicsBody;

enum class BodyType {
    CHARACTER,
    PUSHABLE_BOX,
    PATROL_ENEMY,
    MOVING_PLATFORM,
    CONDITIONAL_PLATFORM,
    ROPE_ENDPOINT,
    BULLET,
    STATIC_BOUNDARY,
};

struct CollisionInfo {
    IPhysicsBody* other  = nullptr;
    glm::vec2     normal = {0.f, 0.f};
};

class IPhysicsBody {
public:
    virtual ~IPhysicsBody() = default;

    [[nodiscard]] virtual BodyType  GetBodyType() const = 0;
    [[nodiscard]] virtual glm::vec2 GetPosition() const = 0;
    virtual void                    SetPosition(const glm::vec2& pos) = 0;
    [[nodiscard]] virtual glm::vec2 GetHalfSize() const = 0;

    [[nodiscard]] virtual bool IsSolid()     const = 0;
    [[nodiscard]] virtual bool IsKinematic() const = 0;

    [[nodiscard]] virtual int GetMoveDir() const { return 0; }

    virtual void PhysicsUpdate() {}

    [[nodiscard]] virtual glm::vec2 GetDesiredDelta() const { return {0.f, 0.f}; }

    virtual void ApplyResolvedDelta(const glm::vec2& delta) {
        SetPosition(GetPosition() + delta);
    }

    virtual void OnCollision(const CollisionInfo& /*info*/) {}

    virtual void PostUpdate() {}

    virtual void NotifyPush() {}

    [[nodiscard]] virtual bool IsFrozen() const { return m_Frozen; }
    virtual void Freeze()   { m_Frozen = true;  }
    virtual void Unfreeze() { m_Frozen = false; }

    [[nodiscard]] virtual bool IsActive() const { return m_Active; }
    virtual void SetActive(bool active)         { m_Active = active; }

protected:
    bool m_Active = true;
    bool m_Frozen = false;
};

#endif // PHYSICS_BODY_HPP