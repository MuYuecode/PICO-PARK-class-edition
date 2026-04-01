#include "physics/CollisionSolver.hpp"

#include <algorithm>
#include <cmath>

#include "physics/SupportResolver.hpp"

glm::vec2 CollisionSolver::ResolveBody(int idx, glm::vec2 desired, BodyInfos& infos) {
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
                infos[j].body->GetPhysicsTraits().type == BodyType::MOVING_PLATFORM;
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
            if (infos[j].body->GetPhysicsTraits().type == BodyType::MOVING_PLATFORM) {
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
                    infos[j].body->GetPhysicsTraits().type == BodyType::STATIC_BOUNDARY &&
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

void CollisionSolver::ResolveAndApply(BodyInfos& infos, float ridingTolerance) {
    const int n = static_cast<int>(infos.size());

    // Detect riding relationships before any movement.
    SupportResolver::DetectRiding(infos, ridingTolerance);

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
                if (infos[supIdx].body->GetPhysicsTraits().type == BodyType::MOVING_PLATFORM) {
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

    EmitCollisionCallbacks(infos);
}

void CollisionSolver::EmitCollisionCallbacks(const BodyInfos& infos) {
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

