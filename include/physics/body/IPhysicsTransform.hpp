#ifndef IPHYSICS_TRANSFORM_HPP
#define IPHYSICS_TRANSFORM_HPP

#include "pch.hpp"

class IPhysicsTransform {
public:
    virtual ~IPhysicsTransform() = default;

    [[nodiscard]] virtual glm::vec2 GetPosition() const = 0;
    virtual void SetPosition(const glm::vec2& pos) = 0;
    [[nodiscard]] virtual glm::vec2 GetHalfSize() const = 0;
};

#endif // IPHYSICS_TRANSFORM_HPP

