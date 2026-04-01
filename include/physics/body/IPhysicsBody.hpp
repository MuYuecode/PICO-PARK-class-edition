//
// Created by cody2 on 2026/3/19.
//

#ifndef PHYSICS_BODY_HPP
#define PHYSICS_BODY_HPP

#include "pch.hpp"
#include "physics/body/IPhysicsCollisionListener.hpp"
#include "physics/body/IPhysicsLifecycle.hpp"
#include "physics/body/IPhysicsMaterial.hpp"
#include "physics/body/IPhysicsMotion.hpp"
#include "physics/body/IPhysicsPushReactive.hpp"
#include "physics/body/IPhysicsTransform.hpp"
#include "physics/body/PhysicsBodyTraits.hpp"

class IPhysicsBody;

struct CollisionInfo {
    IPhysicsBody* other  = nullptr;
    glm::vec2     normal = {0.f, 0.f};
};

class IPhysicsBody : public IPhysicsTransform,
                     public IPhysicsMaterial,
                     public IPhysicsMotion,
                     public IPhysicsCollisionListener,
                     public IPhysicsPushReactive,
                     public IPhysicsLifecycle {
public:
    virtual ~IPhysicsBody() = default;

    [[nodiscard]] virtual const PhysicsBodyTraits& GetPhysicsTraits() const = 0;

    [[deprecated("Use GetPhysicsTraits().type")]]
    [[nodiscard]] virtual BodyType GetBodyType() const { return GetPhysicsTraits().type; }

    virtual void NotifyPush() override {}

    [[nodiscard]] bool IsFrozen() const override { return m_Frozen; }
    void Freeze() override   { m_Frozen = true;  }
    void Unfreeze() override { m_Frozen = false; }

    [[nodiscard]] bool IsActive() const override { return m_Active; }
    void SetActive(bool active) override         { m_Active = active; }

protected:
    bool m_Active = true;
    bool m_Frozen = false;
};

#endif // PHYSICS_BODY_HPP
