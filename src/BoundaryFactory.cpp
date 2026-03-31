#include "BoundaryFactory.hpp"

namespace BoundaryFactory {

void AddStaticRoomBoundaries(PhysicsWorld& world,
                             float floorSurfaceY,
                             std::optional<float> ceilingSurfaceY,
                             const BoundaryPreset& preset) {
    world.AddStaticBoundary(
        {0.0f, floorSurfaceY - preset.floorHalfHeight},
        {preset.worldHalfWidth, preset.floorHalfHeight});

    if (preset.hasCeiling && ceilingSurfaceY.has_value()) {
        world.AddStaticBoundary(
            {0.0f, *ceilingSurfaceY + preset.ceilingHalfHeight},
            {preset.worldHalfWidth, preset.ceilingHalfHeight});
    }

    world.AddStaticBoundary(
        {preset.leftWallCenterX, 0.0f},
        {preset.wallHalfWidth, preset.wallHalfHeight});

    world.AddStaticBoundary(
        {preset.rightWallCenterX, 0.0f},
        {preset.wallHalfWidth, preset.wallHalfHeight});
}

} // namespace BoundaryFactory

