#ifndef IPHYSICS_COLLISION_LISTENER_HPP
#define IPHYSICS_COLLISION_LISTENER_HPP

struct CollisionInfo;

class IPhysicsCollisionListener {
public:
    virtual ~IPhysicsCollisionListener() = default;

    virtual void OnCollision(const CollisionInfo& info) = 0;
};

#endif // IPHYSICS_COLLISION_LISTENER_HPP

