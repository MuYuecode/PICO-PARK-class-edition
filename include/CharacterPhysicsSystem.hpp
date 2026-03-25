//
// Created by cody2 on 2026/3/19.
//
#ifndef CHARACTER_PHYSICS_SYSTEM_HPP
#define CHARACTER_PHYSICS_SYSTEM_HPP

#include <memory>
#include <vector>

#include "PlayerCat.hpp"
#include "Character.hpp"

struct PhysicsState {
    float velocityY     = 0.0f;
    float lastDeltaX    = 0.0f;
    int   supportIndex  = -1;   // Stand on which character ：-1 = None
    int   moveDir       = 0;    // -1 / 0 / +1
    bool  grounded      = true;
    bool  prevGrounded  = true; // pre time grounded
    bool  beingStoodOn  = false; // any one stand on head
};

struct PhysicsAgent {
    std::shared_ptr<PlayerCat> actor;
    PhysicsState               state;
};

class CharacterPhysicsSystem {
public:
    static constexpr float kGroundMoveSpeed  = 5.0f;
    static constexpr float kRunOnPlayerSpeed = 6.2f;
    static constexpr float kJumpForce        = 11.0f;
    static constexpr float kGravity          = 0.75f;

    static constexpr float kScreenHalfW = 640.0f;
    static constexpr float kScreenHalfH = 360.0f;

    static void ApplyJump(PhysicsState& state) {
        state.velocityY    = kJumpForce;
        state.grounded     = false;
        state.supportIndex = -1;
    }

    static void Update(std::vector<PhysicsAgent>& agents,const std::shared_ptr<Character>& floor) ;
    static float ResolveHorizontal(int idx,float targetX,const std::vector<PhysicsAgent>& agents) ;

private:
    static float HalfWidth()  ;
    static float HalfHeight() ;

    static float StandOffset(const std::shared_ptr<Character>& floor) ;
    static void  ResolveVertical(int idx,std::vector<PhysicsAgent>& agents,const std::shared_ptr<Character>& floor);
    static bool  IsBeingPushed(int idx,const std::vector<PhysicsAgent>& agents,const std::shared_ptr<Character>& floor) ;
    static void  UpdateAnimState(int idx,const std::vector<PhysicsAgent>& agents,bool isPushing) ;
};

#endif // CHARACTER_PHYSICS_SYSTEM_HPP