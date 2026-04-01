#ifndef IPUSH_QUERY_SERVICE_HPP
#define IPUSH_QUERY_SERVICE_HPP

#include <vector>

#include "physics/body/IPhysicsBody.hpp"

class IPushQueryService {
public:
    virtual ~IPushQueryService() = default;

    [[nodiscard]] virtual int CountCharactersPushing(const IPhysicsBody* target, int dir) const = 0;
    [[nodiscard]] virtual std::vector<IPhysicsBody*> GetBodiesOfType(BodyType type) const = 0;
};

#endif // IPUSH_QUERY_SERVICE_HPP

