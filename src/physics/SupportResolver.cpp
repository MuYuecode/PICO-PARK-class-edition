#include "physics/SupportResolver.hpp"

#include <cmath>

void SupportResolver::DetectRiding(BodyInfos& infos, float ridingTolerance) {
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
        float bestDist = ridingTolerance + 1.0f;
        int bestIdx = -1;
        bool bestIsMovingPlatform = false;

        for (int j = 0; j < n; ++j) {
            if (i == j) continue;
            if (!infos[j].body->IsSolid()) continue;
            if (infos[j].body->GetPhysicsTraits().type == BodyType::PATROL_ENEMY) continue;

            const glm::vec2 bPos  = infos[j].body->GetPosition();
            const glm::vec2 bHalf = infos[j].body->GetHalfSize();
            const float     bTop  = bPos.y + bHalf.y;

            const bool fromAbove = (aPos.y >= bPos.y);
            const bool vertOk = (aBot >= bTop - ridingTolerance) &&
                                (aBot <= bTop + ridingTolerance);
            const bool horizOk = std::abs(aPos.x - bPos.x) < (aHalf.x + bHalf.x) * 0.75f;

            if (fromAbove && vertOk && horizOk) {
                const float dist = std::abs(aBot - bTop);
                const bool candidateIsMovingPlatform =
                    infos[j].body->GetPhysicsTraits().type == BodyType::MOVING_PLATFORM;
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

