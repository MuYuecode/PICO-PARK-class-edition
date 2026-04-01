#include "physics/PhysicsUpdateScheduler.hpp"

void PhysicsUpdateScheduler::StepPhysicsUpdate(const std::vector<std::weak_ptr<IPhysicsBody>>& bodies) {
    // all non-box active bodies.
    // Characters run first so their m_MoveDir is already committed when
    // PushableBox bodies query CountCharactersPushing() in pass 2.
    for (auto& wp : bodies) {
        auto sp = wp.lock();
        if (!sp || !sp->IsActive() || sp->IsFrozen()) continue;
        if (sp->GetPhysicsTraits().type == BodyType::PUSHABLE_BOX) continue;
        sp->PhysicsUpdate();
    }
    // pushable boxes (characters have set their desired deltas).
    for (auto& wp : bodies) {
        auto sp = wp.lock();
        if (!sp || !sp->IsActive() || sp->IsFrozen()) continue;
        if (sp->GetPhysicsTraits().type != BodyType::PUSHABLE_BOX) continue;
        sp->PhysicsUpdate();
    }
}

