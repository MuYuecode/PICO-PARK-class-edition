//
// Created by cody2 on 2026/3/28.
//

#ifndef STATIC_BODY_HPP
#define STATIC_BODY_HPP

#include "physics/body/IPhysicsBody.hpp"
class StaticBody final : public IPhysicsBody {
public:
    StaticBody(glm::vec2 center, glm::vec2 halfSize,
               BodyType type = BodyType::STATIC_BOUNDARY);

    [[nodiscard]] const PhysicsBodyTraits& GetPhysicsTraits() const override { return m_Traits; }
    [[nodiscard]] glm::vec2 GetPosition() const override { return m_Position; }
    [[nodiscard]] glm::vec2 GetHalfSize() const override { return m_HalfSize; }
    [[nodiscard]] bool      IsSolid()     const override { return true; }
    [[nodiscard]] bool      IsKinematic() const override { return true; }
    [[nodiscard]] int       GetMoveDir()  const override { return 0; }

    void SetPosition(const glm::vec2& /*pos*/) override {}

    void PhysicsUpdate() override {}

    [[nodiscard]] glm::vec2 GetDesiredDelta() const override { return {0.f, 0.f}; }

    void ApplyResolvedDelta(const glm::vec2& /*delta*/) override {}

    void OnCollision(const CollisionInfo& /*info*/) override {}

    void PostUpdate() override {}

private:
    glm::vec2 m_Position;
    glm::vec2 m_HalfSize;
    PhysicsBodyTraits m_Traits;
};

#endif // STATIC_BODY_HPP
