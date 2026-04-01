#ifndef SUPPORT_RESOLVER_HPP
#define SUPPORT_RESOLVER_HPP

#include "physics/PhysicsSnapshot.hpp"

class SupportResolver {
public:
    static void DetectRiding(BodyInfos& infos, float ridingTolerance);
};

#endif // SUPPORT_RESOLVER_HPP

