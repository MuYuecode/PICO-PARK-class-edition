//
// Created by cody2 on 2026/3/19.
//
#ifndef PHYSICS_BODY_HPP
#define PHYSICS_BODY_HPP

#include "pch.hpp"

class IPhysicsBody;

enum class BodyType {
    CHARACTER,              // PlayerCat
    PUSHABLE_BOX,
    PATROL_ENEMY,
    MOVING_PLATFORM,
    CONDITIONAL_PLATFORM,
    ROPE_ENDPOINT,
    BULLET,
};

struct CollisionInfo {
    IPhysicsBody* other    = nullptr;
    glm::vec2     normal   = {0, 0};
    float         depth    = 0.0f;
};

class IPhysicsBody {
public:
    virtual ~IPhysicsBody() = default;

    [[nodiscard]] virtual BodyType GetBodyType() const = 0;
    [[nodiscard]] virtual glm::vec2 GetPosition() const = 0;
    virtual void          SetPosition(const glm::vec2& pos) = 0;

    [[nodiscard]] virtual glm::vec2 GetHalfSize() const = 0;

    [[nodiscard]] virtual glm::vec2 GetVelocity() const = 0;
    virtual void      SetVelocity(const glm::vec2& vel) = 0;

    [[nodiscard]] virtual bool IsSolid() const = 0;
    [[nodiscard]] virtual bool IsKinematic() const = 0;
    [[nodiscard]] virtual bool UseGravity() const = 0;

    virtual void PhysicsUpdate() {}

    virtual void OnCollision(const CollisionInfo& /*info*/) {}

    [[nodiscard]] virtual bool IsActive() const { return m_Active; }
    virtual void SetActive(bool active) { m_Active = active; }

protected:
    bool m_Active = true;
};

#endif // PHYSICS_BODY_HPP
