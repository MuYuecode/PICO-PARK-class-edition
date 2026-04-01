#ifndef PHYSICS_SNAPSHOT_HPP
#define PHYSICS_SNAPSHOT_HPP

#include <memory>
#include <vector>

#include "physics/body/IPhysicsBody.hpp"

struct BodyInfo {
    std::shared_ptr<IPhysicsBody> body;
    glm::vec2 desired      = {0.f, 0.f};
    glm::vec2 resolved     = {0.f, 0.f};
    int       supportIdx   = -1;    // index in the snapshot vector (-1 = none)
    bool      resolvedFlag = false; // true once this body's delta is finalised
    IPhysicsBody* collidedH  = nullptr; // body that caused horizontal separation
    IPhysicsBody* collidedV  = nullptr; // body that caused vertical separation
    glm::vec2     normalH    = {0.f, 0.f};
    glm::vec2     normalV    = {0.f, 0.f};
};

using BodyInfos = std::vector<BodyInfo>;

#endif // PHYSICS_SNAPSHOT_HPP

