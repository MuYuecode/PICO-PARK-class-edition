#ifndef PHYSICS_BODY_TRAITS_HPP
#define PHYSICS_BODY_TRAITS_HPP

enum class BodyType {
    CHARACTER,
    PUSHABLE_BOX,
    PATROL_ENEMY,
    MOVING_PLATFORM,
    BULLET,
    JAR,
    STATIC_BOUNDARY,
};

struct PhysicsBodyTraits {
    BodyType type;
};

#endif // PHYSICS_BODY_TRAITS_HPP

