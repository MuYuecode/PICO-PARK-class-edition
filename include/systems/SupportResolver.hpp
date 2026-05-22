#ifndef SUPPORT_RESOLVER_HPP
#define SUPPORT_RESOLVER_HPP

#include "systems/PhysicsSnapshot.hpp"

class SupportResolver {
public:
    static void DetectRiding(BodyInfos& infos, float ridingTolerance);
};

#endif // SUPPORT_RESOLVER_HPP

