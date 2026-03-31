#include "PhysicsWorld.hpp"
#include <algorithm>
#include <cmath>

void PhysicsWorld::Register(const std::shared_ptr<IPhysicsBody>& body) {
    if (body == nullptr) return;
    m_Bodies.emplace_back(body);
}

void PhysicsWorld::Unregister(const IPhysicsBody* body) {
    m_Bodies.erase(
        std::remove_if(m_Bodies.begin(), m_Bodies.end(),
            [body](const std::weak_ptr<IPhysicsBody>& wp) {
                auto sp = wp.lock();
                return !sp || sp.get() == body;
            }),
        m_Bodies.end());
}

void PhysicsWorld::Clear() {
    m_Bodies.clear();
    m_OwnedStatics.clear();
    m_Ropes.clear();
}

StaticBody* PhysicsWorld::AddStaticBoundary(glm::vec2 center, glm::vec2 halfSize,
                                              BodyType type) {
    auto sb = std::make_shared<StaticBody>(center, halfSize, type);
    StaticBody* raw = sb.get();
    m_OwnedStatics.push_back(sb);
    Register(sb);
    return raw;
}

void PhysicsWorld::AddRope(IPhysicsBody* a, IPhysicsBody* b,
                            float maxLen, float friction) {
    m_Ropes.push_back({a, b, maxLen, friction});
}

void PhysicsWorld::RemoveRope(IPhysicsBody* a, IPhysicsBody* b) {
    m_Ropes.erase(
        std::remove_if(m_Ropes.begin(), m_Ropes.end(),
            [a, b](const RopeConstraint& r) {
                return (r.bodyA == a && r.bodyB == b) ||
                       (r.bodyA == b && r.bodyB == a);
            }),
        m_Ropes.end());
}

std::vector<RopeConstraint*> PhysicsWorld::GetRopesOf(const IPhysicsBody* body) {
    std::vector<RopeConstraint*> result;
    for (auto& r : m_Ropes) {
        if (r.bodyA == body || r.bodyB == body) result.push_back(&r);
    }
    return result;
}

void PhysicsWorld::FreezeAll() const {
    for (auto& wp : m_Bodies) {
        if (auto sp = wp.lock()) sp->Freeze();
    }
}

void PhysicsWorld::UnfreezeAll() const {
    for (auto& wp : m_Bodies) {
        if (auto sp = wp.lock()) sp->Unfreeze();
    }
}

void PhysicsWorld::Update() {
    ++m_FrameCount;
    if (m_FrameCount % kPurgeInterval == 0) PurgeExpired();

    StepPhysicsUpdate();
    StepResolveAndApply();

    for (auto& wp : m_Bodies) {
        auto sp = wp.lock();
        if (sp && sp->IsActive() && !sp->IsFrozen()) {
            sp->PostUpdate();
        }
    }
}

void PhysicsWorld::StepPhysicsUpdate() const {
    // all non-box active bodies.
    // Characters run first so their m_MoveDir is already committed when
    // PushableBox bodies query CountCharactersPushing() in pass 2.
    for (auto& wp : m_Bodies) {
        auto sp = wp.lock();
        if (!sp || !sp->IsActive() || sp->IsFrozen()) continue;
        if (sp->GetBodyType() == BodyType::PUSHABLE_BOX) continue;
        sp->PhysicsUpdate();
    }
    // pushable boxes (characters have set their desired deltas).
    for (auto& wp : m_Bodies) {
        auto sp = wp.lock();
        if (!sp || !sp->IsActive() || sp->IsFrozen()) continue;
        if (sp->GetBodyType() != BodyType::PUSHABLE_BOX) continue;
        sp->PhysicsUpdate();
    }
}

void PhysicsWorld::DetectRiding(std::vector<BodyInfo>& infos) {
    constexpr float kPlatformSupportBias = 2.0f;
    const int n = static_cast<int>(infos.size());
    for (int i = 0; i < n; ++i) {
        // Kinematic bodies do not ride anything (they define their own path).
        if (infos[i].body->IsKinematic()) continue;
        // Do not attach support while actively moving upward (jumping).
        if (infos[i].desired.y > 0.0f) continue;

        const glm::vec2 aPos   = infos[i].body->GetPosition();
        const glm::vec2 aHalf  = infos[i].body->GetHalfSize();
        const float     aBot   = aPos.y - aHalf.y;
        float bestDist = kRidingTolerance + 1.0f;
        int bestIdx = -1;
        bool bestIsMovingPlatform = false;

        for (int j = 0; j < n; ++j) {
            if (i == j) continue;
            if (!infos[j].body->IsSolid()) continue;
            if (infos[j].body->GetBodyType() == BodyType::PATROL_ENEMY) continue;

            const glm::vec2 bPos  = infos[j].body->GetPosition();
            const glm::vec2 bHalf = infos[j].body->GetHalfSize();
            const float     bTop  = bPos.y + bHalf.y;

            const bool fromAbove = (aPos.y >= bPos.y);
            const bool vertOk = (aBot >= bTop - kRidingTolerance) &&
                                (aBot <= bTop + kRidingTolerance);
            const bool horizOk = std::abs(aPos.x - bPos.x) < (aHalf.x + bHalf.x) * 0.75f;

            if (fromAbove && vertOk && horizOk) {
                const float dist = std::abs(aBot - bTop);
                const bool candidateIsMovingPlatform =
                    infos[j].body->GetBodyType() == BodyType::MOVING_PLATFORM;
                const bool betterDistance = dist < (bestDist - 0.001f);
                const bool tieBreakToPlatform =
                    std::abs(dist - bestDist) <= 0.001f &&
                    candidateIsMovingPlatform && !bestIsMovingPlatform;
                const bool biasToPlatform =
                    candidateIsMovingPlatform && !bestIsMovingPlatform &&
                    dist <= (bestDist + kPlatformSupportBias);

                if (betterDistance || tieBreakToPlatform || biasToPlatform) {
                    bestDist = dist;
                    bestIdx = j;
                    bestIsMovingPlatform = candidateIsMovingPlatform;
                }
            }
        }

        infos[i].supportIdx = bestIdx;
    }
}

glm::vec2 PhysicsWorld::ResolveBody(int idx, glm::vec2 desired,
                                     std::vector<BodyInfo>& infos) {
    auto& self     = infos[idx];
    glm::vec2 selfPos  = self.body->GetPosition();
    glm::vec2 selfHalf = self.body->GetHalfSize();

    glm::vec2 resolved = desired;
    self.collidedH = nullptr;
    self.collidedV = nullptr;
    self.normalH   = {0.f, 0.f};
    self.normalV   = {0.f, 0.f};

    const int n = static_cast<int>(infos.size());

    // --- Horizontal pass ---
    {
        glm::vec2 testPos = selfPos + glm::vec2{resolved.x, 0.f};

        for (int j = 0; j < n; ++j) {
            if (j == idx || !infos[j].body->IsSolid()) continue;

            // Use the other body's final resting position if already resolved,
            // otherwise use its current (pre-move) position.
            glm::vec2 otherPos = infos[j].body->GetPosition();
            if (infos[j].resolvedFlag) otherPos += infos[j].resolved;

            const glm::vec2 otherHalf = infos[j].body->GetHalfSize();
            const glm::vec2 delta     = testPos - otherPos;
            const glm::vec2 overlap   = (selfHalf + otherHalf) - glm::abs(delta);
            const bool isMovingPlatform =
                infos[j].body->GetBodyType() == BodyType::MOVING_PLATFORM;
            const glm::vec2 otherDelta = infos[j].resolvedFlag ? infos[j].resolved : infos[j].desired;
            const bool platformRising = otherDelta.y > 0.05f;
            const bool unsupportedUpwardJump = desired.y > 0.0f && self.supportIdx != j;
            // Prevent lateral ejection when the player starts a jump on an ascending lift.
            const bool skipHardHorizontalSeparation =
                isMovingPlatform && platformRising && unsupportedUpwardJump;

            if (overlap.x > 0.f && overlap.y > 0.f) {
                if (skipHardHorizontalSeparation) {
                    continue;
                }

                // Choose the nearest separating edge among left / right / top.
                // Top is only allowed when this body is above the other body.
                const float sumHalfX = selfHalf.x + otherHalf.x;
                const float corrLeft = (otherPos.x - sumHalfX) - testPos.x;
                const float corrRight = (otherPos.x + sumHalfX) - testPos.x;

                float bestCorr = corrLeft;
                float bestAbs = std::abs(corrLeft);
                bool chooseTop = false;

                if (std::abs(corrRight) < bestAbs) {
                    bestCorr = corrRight;
                    bestAbs = std::abs(corrRight);
                }

                const bool fromAbove = testPos.y >= otherPos.y;
                const bool allowTopSeparation = fromAbove &&
                                                (desired.y <= 0.0f || self.supportIdx == j);
                if (allowTopSeparation) {
                    const float targetTopY = otherPos.y + otherHalf.y + selfHalf.y;
                    const float corrTop = targetTopY - testPos.y;
                    if (corrTop >= 0.f && std::abs(corrTop) < bestAbs) {
                        bestCorr = corrTop;
                        chooseTop = true;
                    }
                }

                if (chooseTop) {
                    resolved.y += bestCorr;
                    testPos.y   = selfPos.y + resolved.y;
                    self.collidedV = infos[j].body.get();
                    self.normalV   = {0.f, 1.f};
                } else {
                    resolved.x += bestCorr;
                    testPos.x   = selfPos.x + resolved.x;

                    // Record contact (take the most recent collision; usually one matters).
                    self.collidedH = infos[j].body.get();
                    self.normalH   = {(bestCorr >= 0.f) ? 1.f : -1.f, 0.f};
                }
            }
        }
    }

    // --- Vertical pass (using the resolved horizontal position) ---
    {
        glm::vec2 testPos = selfPos + glm::vec2{resolved.x, resolved.y};
        bool snappedToMovingPlatform = false;
        constexpr float kContactEps = 0.6f;
        constexpr float kEndpointSnapIgnore = 2.5f;
        constexpr float kMinPlatformRiseDy = 0.05f;
        constexpr float kMaxPlatformSnapAdjust = 12.0f;

        for (int j = 0; j < n; ++j) {
            if (j == idx || !infos[j].body->IsSolid()) continue;

            const glm::vec2 otherStart = infos[j].body->GetPosition();
            const glm::vec2 otherDelta = infos[j].resolvedFlag ? infos[j].resolved : infos[j].desired;
            const glm::vec2 otherFinal = otherStart + otherDelta;
            const glm::vec2 otherHalf  = infos[j].body->GetHalfSize();

            // Scheme 3: continuous vertical hit test for moving platforms.
            if (infos[j].body->GetBodyType() == BodyType::MOVING_PLATFORM) {
                const float sumHalfX = selfHalf.x + otherHalf.x;
                const float selfX = selfPos.x + resolved.x;
                const float dx0 = selfX - otherStart.x;
                const float dx1 = selfX - otherFinal.x;
                const float minAbsDx = (dx0 * dx1 <= 0.0f)
                                           ? 0.0f
                                           : std::min(std::abs(dx0), std::abs(dx1));

                const float selfBot0 = selfPos.y - selfHalf.y;
                const float selfBot1 = selfPos.y + resolved.y - selfHalf.y;
                const float platTop0 = otherStart.y + otherHalf.y;
                const float platTop1 = otherFinal.y + otherHalf.y;
                const float rel0 = selfBot0 - platTop0;
                const float rel1 = selfBot1 - platTop1;

                const bool horizPathOverlap = minAbsDx < sumHalfX;
                const bool fromAbove = rel0 >= -kContactEps;
                const bool closing = rel1 < rel0;
                const bool crosses = rel0 >= -kContactEps && rel1 <= kContactEps;
                const bool isSupportedPlatform = (self.supportIdx == j);
                const bool stickyContact =
                    std::abs(rel0) <= kContactEps && rel1 <= kContactEps &&
                    (desired.y <= 0.0f || isSupportedPlatform);
                const bool platformRising = otherDelta.y > kMinPlatformRiseDy;
                const bool jumpingUpAndUnsupported = desired.y > 0.0f && !isSupportedPlatform;
                const bool allowPlatformSnap = !jumpingUpAndUnsupported &&
                                               (platformRising || isSupportedPlatform || stickyContact);

                const bool shouldSnapToPlatform = (crosses && closing) || stickyContact;
                if (horizPathOverlap && fromAbove && shouldSnapToPlatform && allowPlatformSnap) {
                    const float denom = rel0 - rel1;
                    float toi = (denom > 1e-5f) ? (rel0 / denom) : 0.0f;
                    toi = std::clamp(toi, 0.0f, 1.0f);

                    const float platformYAtToi = otherStart.y + otherDelta.y * toi;
                    const float platformYFinal = otherFinal.y;
                    const float targetY = (stickyContact ? platformYFinal : platformYAtToi) +
                                          selfHalf.y + otherHalf.y;
                    float snappedY = targetY - selfPos.y;

                    // Limit first-contact correction spikes to avoid visible teleport.
                    if (!isSupportedPlatform) {
                        const float adjust = snappedY - desired.y;
                        if (adjust > kMaxPlatformSnapAdjust) {
                            snappedY = desired.y + kMaxPlatformSnapAdjust;
                        } else if (adjust < -kMaxPlatformSnapAdjust) {
                            snappedY = desired.y - kMaxPlatformSnapAdjust;
                        }
                    }

                    resolved.y = snappedY;
                    testPos.y = selfPos.y + resolved.y;
                    snappedToMovingPlatform = true;

                    self.collidedV = infos[j].body.get();
                    self.normalV = {0.f, 1.f};
                    continue;
                }
            }

            glm::vec2 otherPos = infos[j].body->GetPosition();
            if (infos[j].resolvedFlag) otherPos += infos[j].resolved;

            const glm::vec2 delta     = testPos - otherPos;
            const glm::vec2 overlap   = (selfHalf + otherHalf) - glm::abs(delta);

            if (overlap.x > 0.f && overlap.y > 0.f) {
                const float sign = (delta.y >= 0.f) ? 1.f : -1.f;

                // If we already snapped to a moving platform this frame, ignore
                // tiny upward corrections from static geometry at lift endpoints.
                if (snappedToMovingPlatform &&
                    infos[j].body->GetBodyType() == BodyType::STATIC_BOUNDARY &&
                    sign > 0.f) {
                    const float feetY = selfPos.y + resolved.y - selfHalf.y;
                    const float otherTop = otherPos.y + otherHalf.y;
                    const float upwardSnap = otherTop - feetY;
                    if (upwardSnap >= 0.0f && upwardSnap <= kEndpointSnapIgnore) {
                        continue;
                    }
                }

                resolved.y = (otherPos.y + (selfHalf.y + otherHalf.y) * sign) - selfPos.y;
                testPos.y  = selfPos.y + resolved.y;

                self.collidedV = infos[j].body.get();
                self.normalV   = {0.f, sign};
            }
        }
    }

    return resolved;
}

void PhysicsWorld::StepResolveAndApply() const {
    // Snapshot live bodies.
    std::vector<BodyInfo> infos;
    infos.reserve(m_Bodies.size());
    for (auto& wp : m_Bodies) {
        auto sp = wp.lock();
        if (!sp || !sp->IsActive()) continue;
        BodyInfo bi;
        bi.body    = sp;
        bi.desired = sp->IsFrozen() ? glm::vec2{0.f, 0.f} : sp->GetDesiredDelta();
        bi.resolved    = bi.desired;
        bi.resolvedFlag = false;
        bi.supportIdx   = -1;
        infos.push_back(std::move(bi));
    }

    const int n = static_cast<int>(infos.size());

    // Detect riding relationships before any movement.
    DetectRiding(infos);

    // Mark kinematic / frozen bodies as immediately resolved (they define
    // their own motion and need no collision resolution against geometry).
    for (int i = 0; i < n; ++i) {
        if (infos[i].body->IsKinematic() || infos[i].body->IsFrozen()) {
            // Kinematic bodies follow their desired path; static bodies have {0,0}.
            infos[i].resolved    = infos[i].desired;
            infos[i].resolvedFlag = true;
        }
    }

    // Iterative topological resolution.
    // Each pass resolves every dynamic body whose support is already resolved.
    bool anyProgress = true;
    while (anyProgress) {
        anyProgress = false;

        for (int i = 0; i < n; ++i) {
            if (infos[i].resolvedFlag) continue;

            const int supIdx = infos[i].supportIdx;

            // Wait until support is resolved (handles stacking chains).
            if (supIdx >= 0 && !infos[supIdx].resolvedFlag) continue;

            // Compute the effective desired delta for this body.
            glm::vec2 effective = infos[i].desired;

            // Stacking carry: inherit support motion.
            // Vertical carry is only applied for moving platforms and only when
            // DetectRiding marks a valid top-contact support.
            if (supIdx >= 0) {
                effective.x += infos[supIdx].resolved.x;
                if (infos[supIdx].body->GetBodyType() == BodyType::MOVING_PLATFORM) {
                    effective.y += infos[supIdx].resolved.y;
                }
            }

            // Resolve against all solid bodies.
            infos[i].resolved    = ResolveBody(i, effective, infos);

            infos[i].resolvedFlag = true;
            anyProgress           = true;
        }
    }

    // Safety: if any body is still unresolved (cycle or orphan), resolve it now
    // without support carry.
    for (int i = 0; i < n; ++i) {
        if (!infos[i].resolvedFlag) {
            infos[i].resolved    = ResolveBody(i, infos[i].desired, infos);
            infos[i].resolvedFlag = true;
        }
    }

    // Phase 2: apply resolved deltas.
    for (auto& bi : infos) {
        bi.body->ApplyResolvedDelta(bi.resolved);
    }

    // Fire OnCollision callbacks.
    StepCollisionCallbacks(infos);

}

void PhysicsWorld::StepCollisionCallbacks(const std::vector<BodyInfo>& infos) {
    for (const auto& bi : infos) {
        if (bi.collidedH != nullptr) {
            CollisionInfo ci;
            ci.other  = bi.collidedH;
            ci.normal = bi.normalH;
            bi.body->OnCollision(ci);
        }
        if (bi.collidedV != nullptr) {
            CollisionInfo ci;
            ci.other  = bi.collidedV;
            ci.normal = bi.normalV;
            bi.body->OnCollision(ci);
        }
    }
}

int PhysicsWorld::CountCharactersPushing(const IPhysicsBody* target, int dir) const {
    std::vector<const IPhysicsBody*> visited;
    return CountCharactersPushingImpl(target, dir, visited);
}

int PhysicsWorld::CountCharactersPushingImpl(
    const IPhysicsBody* target,
    int dir,
    std::vector<const IPhysicsBody*>& visited) const
{
    if (std::find(visited.begin(), visited.end(), target) != visited.end()) return 0;
    visited.push_back(target);

    int count = 0;
    const glm::vec2 tPos  = target->GetPosition();
    const glm::vec2 tHalf = target->GetHalfSize();

    for (auto& wp : m_Bodies) {
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

        if (sp->GetBodyType() == BodyType::CHARACTER) {
            // An actively-moving character contributes +1 to the push count.
            if (sp->GetMoveDir() == dir) {
                ++count;
            }
            // Always recurse regardless of this character's own moveDir.
            // Rationale: a passive character (not pressing movement keys)
            // sandwiched between an active pusher and the target still
            // transmits the upstream force through the chain.
            //
            // Scenario verification:
            //   [A(dir) -> B(0) -> C(box)] : B is passive (moveDir=0), but A
            //   is found when we recurse into B → C sees count=1.
            //   [A(dir) -> B(dir) -> C(box)]: B is active, count++ for B, then
            //   A found in recursion → C sees count=2.
            //   [A(+1) -> C <- B(-1)]: each direction counted
            //   independently; net=0.
            count += CountCharactersPushingImpl(sp.get(), dir, visited);
        }
        else if (sp->GetBodyType() == BodyType::PUSHABLE_BOX) {
            // A box transmits force passively; recurse to discover its own pushers.
            count += CountCharactersPushingImpl(sp.get(), dir, visited);
        }
    }
    return count;
}

std::vector<IPhysicsBody*> PhysicsWorld::GetBodiesOfType(BodyType type) const {
    std::vector<IPhysicsBody*> result;
    for (auto& wp : m_Bodies) {
        auto sp = wp.lock();
        if (sp && sp->IsActive() && sp->GetBodyType() == type) {
            result.push_back(sp.get());
        }
    }
    return result;
}

bool PhysicsWorld::AabbOverlaps(const IPhysicsBody* a, const IPhysicsBody* b) {
    const glm::vec2 delta = a->GetPosition() - b->GetPosition();
    const glm::vec2 sum   = a->GetHalfSize() + b->GetHalfSize();
    return std::abs(delta.x) < sum.x && std::abs(delta.y) < sum.y;
}

void PhysicsWorld::PurgeExpired() {
    m_Bodies.erase(
        std::remove_if(m_Bodies.begin(), m_Bodies.end(),
            [](const std::weak_ptr<IPhysicsBody>& wp) { return wp.expired(); }),
        m_Bodies.end());
}