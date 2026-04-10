#ifndef PUSH_FORCE_RESOLVER_HPP
#define PUSH_FORCE_RESOLVER_HPP

#include <memory>
#include <vector>

#include "physics/IPhysicsBody.hpp"

class PushForceResolver {
public:
    static int CountCharactersPushing(const IPhysicsBody* target,
                                      int dir,
                                      const std::vector<std::weak_ptr<IPhysicsBody>>& bodies);
};

#endif // PUSH_FORCE_RESOLVER_HPP

