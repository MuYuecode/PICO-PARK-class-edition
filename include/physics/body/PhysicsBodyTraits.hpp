#ifndef PHYSICS_BODY_TRAITS_HPP
#define PHYSICS_BODY_TRAITS_HPP

enum class BodyType {
    CHARACTER,
    PUSHABLE_BOX,
    PATROL_ENEMY,
    MOVING_PLATFORM,
    CONDITIONAL_PLATFORM,
    ROPE_ENDPOINT,
    BULLET,
    STATIC_BOUNDARY,
};

struct PhysicsBodyTraits {
    BodyType type;
    bool supportsPush;
    bool emitsCollisionCallbacks;
};

#endif // PHYSICS_BODY_TRAITS_HPP

