#ifndef COLLISION_SOLVER_HPP
#define COLLISION_SOLVER_HPP

#include "physics/PhysicsSnapshot.hpp"

class CollisionSolver {
public:
    static glm::vec2 ResolveBody(int idx, glm::vec2 desired, BodyInfos& infos);
    static void ResolveAndApply(BodyInfos& infos, float ridingTolerance);
    static void EmitCollisionCallbacks(const BodyInfos& infos);
};

#endif // COLLISION_SOLVER_HPP

