#ifndef BOUNDARY_FACTORY_HPP
#define BOUNDARY_FACTORY_HPP

#include <optional>

#include "physics/PhysicsWorld.hpp"

struct BoundaryPreset {
    float worldHalfWidth;
    float wallHalfWidth;
    float wallHalfHeight;
    float floorHalfHeight;
    bool hasCeiling;
    float ceilingHalfHeight;
    float leftWallCenterX;
    float rightWallCenterX;
};

namespace LevelGeometryPreset {
inline constexpr BoundaryPreset kSharedMenuLikeNoCeiling = {
    690.0f,
    50.0f,
    400.0f,
    40.0f,
    false,
    0.0f,
    -690.0f,
    690.0f,
};

inline constexpr BoundaryPreset kSharedMenuLikeWithCeiling = {
    690.0f,
    50.0f,
    400.0f,
    40.0f,
    true,
    40.0f,
    -690.0f,
    690.0f,
};

inline constexpr BoundaryPreset kLevelOneRoom = {
    600.0f,
    32.0f,
    360.0f,
    35.0f,
    true,
    35.0f,
    -639.0f,
    639.0f,
};
} // namespace LevelGeometryPreset

namespace BoundaryFactory {
void AddStaticRoomBoundaries(PhysicsWorld& world,
                             float floorSurfaceY,
                             std::optional<float> ceilingSurfaceY,
                             const BoundaryPreset& preset);
} // namespace BoundaryFactory

#endif // BOUNDARY_FACTORY_HPP


