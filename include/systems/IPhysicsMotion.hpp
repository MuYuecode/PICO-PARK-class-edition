#ifndef IPHYSICS_MOTION_HPP
#define IPHYSICS_MOTION_HPP

#include "pch.hpp"

class IPhysicsMotion {
public:
    virtual ~IPhysicsMotion() = default;

    virtual void PhysicsUpdate() = 0;
    [[nodiscard]] virtual glm::vec2 GetDesiredDelta() const = 0;
    virtual void ApplyResolvedDelta(const glm::vec2& delta) = 0;
};

#endif // IPHYSICS_MOTION_HPP

