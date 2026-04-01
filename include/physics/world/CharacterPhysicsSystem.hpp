//
// Created by cody2 on 2026/3/19.
//
#ifndef CHARACTER_PHYSICS_SYSTEM_HPP
#define CHARACTER_PHYSICS_SYSTEM_HPP

struct CharacterPhysicsSystem {
    static constexpr float kGravity          = 0.75f;
    static constexpr float kJumpForce        = 11.0f;
    static constexpr float kGroundMoveSpeed  = 5.0f;
    static constexpr float kRunOnPlayerSpeed = 6.2f;
    static constexpr float kScreenHalfW      = 640.0f;
    static constexpr float kScreenHalfH      = 360.0f;
};

#endif // CHARACTER_PHYSICS_SYSTEM_HPP
