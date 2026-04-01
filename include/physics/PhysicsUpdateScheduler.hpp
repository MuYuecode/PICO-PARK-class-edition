#ifndef PHYSICS_UPDATE_SCHEDULER_HPP
#define PHYSICS_UPDATE_SCHEDULER_HPP

#include <memory>
#include <vector>

#include "physics/body/IPhysicsBody.hpp"

class PhysicsUpdateScheduler {
public:
    static void StepPhysicsUpdate(const std::vector<std::weak_ptr<IPhysicsBody>>& bodies);
};

#endif // PHYSICS_UPDATE_SCHEDULER_HPP

