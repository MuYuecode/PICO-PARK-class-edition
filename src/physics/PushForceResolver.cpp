#include "physics/PushForceResolver.hpp"

#include <algorithm>
#include <cmath>

namespace {

int CountCharactersPushingImpl(const IPhysicsBody* target,
                               int dir,
                               const std::vector<std::weak_ptr<IPhysicsBody>>& bodies,
                               std::vector<const IPhysicsBody*>& visited) {
    if (std::find(visited.begin(), visited.end(), target) != visited.end()) return 0;
    visited.push_back(target);

    int count = 0;
    const glm::vec2 tPos  = target->GetPosition();
    const glm::vec2 tHalf = target->GetHalfSize();

    for (auto& wp : bodies) {
        auto sp = wp.lock();
        if (!sp || !sp->IsActive() || sp.get() == target) continue;

        const glm::vec2 cPos  = sp->GetPosition();
        const glm::vec2 cHalf = sp->GetHalfSize();

        // Must share the same horizontal band.
        if (std::abs(cPos.y - tPos.y) > (tHalf.y + cHalf.y) * 0.9f) continue;

        // Must be adjacent (directly touching on the push side).
        const float minDistX   = tHalf.x + cHalf.x;
        const float dxToTarget = cPos.x - tPos.x;
        if (std::abs(dxToTarget) > minDistX + 4.0f) continue;

        // Pusher must be on the origin side of the force:
        //   pushing right (dir=+1): pusher is to the LEFT  (dxToTarget < 0)
        //   pushing left  (dir=-1): pusher is to the RIGHT (dxToTarget > 0)
        if (!((dir > 0 && dxToTarget < 0) || (dir < 0 && dxToTarget > 0))) continue;

        if (sp->GetPhysicsTraits().type == BodyType::CHARACTER) {
            // An actively-moving character contributes +1 to the push count.
            if (sp->GetMoveDir() == dir) {
                ++count;
            }
            // Always recurse regardless of this character's own moveDir.
            // Rationale: a passive character (not pressing movement keys)
            // sandwiched between an active pusher and the target still
            // transmits the upstream force through the chain.
            count += CountCharactersPushingImpl(sp.get(), dir, bodies, visited);
        } else if (sp->GetPhysicsTraits().type == BodyType::PUSHABLE_BOX) {
            // A box transmits force passively; recurse to discover its own pushers.
            count += CountCharactersPushingImpl(sp.get(), dir, bodies, visited);
        }
    }
    return count;
}

} // namespace

int PushForceResolver::CountCharactersPushing(const IPhysicsBody* target,
                                              int dir,
                                              const std::vector<std::weak_ptr<IPhysicsBody>>& bodies) {
    std::vector<const IPhysicsBody*> visited;
    return CountCharactersPushingImpl(target, dir, bodies, visited);
}

