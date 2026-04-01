//
// Created by cody2 on 2026/3/12.
//

#include "gameplay/actors/PlayerCat.hpp"
#include <cmath>

namespace {

std::shared_ptr<Util::Animation> MakeAnim(const std::vector<std::string>& paths,
                                           int intervalMs,
                                           bool looping) {
    if (paths.empty()) return nullptr;
    return std::make_shared<Util::Animation>(paths, false, intervalMs, looping, 0);
}

}
PlayerCat::PlayerCat(const CatAnimPaths& animPaths,
                     Util::Keycode leftKey,
                     Util::Keycode rightKey,
                     Util::Keycode jumpKey)
    : AnimatedCharacter(animPaths.stand.empty()
                            ? std::vector<std::string>{""}
                            : animPaths.stand)
    , m_LeftKey(leftKey)
    , m_RightKey(rightKey)
    , m_JumpKey(jumpKey)
{
    BuildClips(animPaths);
    if (m_StandAnim) m_StandAnim->Play();
}

void PlayerCat::BuildClips(const CatAnimPaths& paths) {
    m_StandAnim = std::dynamic_pointer_cast<Util::Animation>(m_Drawable);
    if (m_StandAnim) {
        m_StandAnim->SetLooping(true);
        m_StandAnim->SetInterval(400);
    }
    m_RunAnim      = MakeAnim(paths.run,       120, true);
    m_JumpRiseAnim = MakeAnim(paths.jump_rise, 150, true);
    m_JumpFallAnim = MakeAnim(paths.jump_fall, 150, true);
    m_LandAnim     = MakeAnim(paths.land,       80, false);
    m_PushAnim     = MakeAnim(paths.push,      120, true);
}

void PlayerCat::Jump() {
    m_VelocityY = kJumpForce;
    m_Grounded  = false;
}

void PlayerCat::PhysicsUpdate() {
    m_PrevGrounded = m_Grounded;
    m_Grounded  = false;
    m_IsPushing = false;
    m_VelocityY -= kGravity;

    // Read input directly when this cat owns real key bindings.
    // For cats with UNKNOWN keys (spawned in LocalPlayGameScene),
    // m_MoveDir is still set by the external SetMoveDir() call in the scene.
    if (m_InputEnabled &&
        (m_LeftKey  != Util::Keycode::UNKNOWN ||
         m_RightKey != Util::Keycode::UNKNOWN)) {
        const bool goLeft  = (m_LeftKey  != Util::Keycode::UNKNOWN) &&
                              Util::Input::IsKeyPressed(m_LeftKey);
        const bool goRight = (m_RightKey != Util::Keycode::UNKNOWN) &&
                              Util::Input::IsKeyPressed(m_RightKey);
        if      (goLeft  && !goRight) m_MoveDir = -1;
        else if (goRight && !goLeft)  m_MoveDir =  1;
        else                          m_MoveDir =  0;
         }

    // Jump is handled here using m_PrevGrounded because m_Grounded
    // has already been reset to false at this point.
    if (m_InputEnabled &&
        m_JumpKey != Util::Keycode::UNKNOWN &&
        m_PrevGrounded &&
        Util::Input::IsKeyDown(m_JumpKey)) {
        Jump();
        }

    const float vx = static_cast<float>(m_MoveDir) * kGroundMoveSpeed;
    m_DesiredDelta = {vx, m_VelocityY};

    SetFacingByDirection(m_MoveDir);
}


void PlayerCat::ApplyResolvedDelta(const glm::vec2& delta) {
    SetPosition(GetPosition() + delta);
}

void PlayerCat::OnCollision(const CollisionInfo& info) {
    if (info.normal.y > 0.5f) {
        m_VelocityY = 0.f;
        m_Grounded  = true;
    }

    // Ceiling contact: normal points down toward this body.
    if (info.normal.y < -0.5f) {
        if (m_VelocityY > 0.f) m_VelocityY = 0.f;
    }

    if (std::abs(info.normal.x) > 0.5f && m_MoveDir != 0) {
        if (info.other != nullptr && info.other->GetPhysicsTraits().type == BodyType::PATROL_ENEMY) {
            return;
        }
        const bool movingRight  = (m_MoveDir > 0);
        const bool blockedRight = (info.normal.x < -0.5f);
        if (movingRight == blockedRight) {
            m_IsPushing = true;
        }
    }
}

void PlayerCat::PostUpdate() {
    UpdateAnimState();
}

void PlayerCat::UpdateAnimState() {
    // Hold LAND animation until the clip finishes.
    if (GetCatAnimState() == CatAnimState::LAND && !IfAnimationEnds()) {
        return;
    }

    const bool justLanded = m_Grounded && !m_PrevGrounded;
    CatAnimState next;

    if (!m_Grounded) {
        next = (m_VelocityY > 0.f) ? CatAnimState::JUMP_RISE : CatAnimState::JUMP_FALL;
    }
    else if (justLanded) {
        next = CatAnimState::LAND;
    }
    else if (m_IsPushing) {
        next = CatAnimState::PUSH;
    }
    else if (m_MoveDir != 0) {
        next = CatAnimState::RUN;
    }
    else {
        next = CatAnimState::STAND;
    }

    SetCatAnimState(next);
}

void PlayerCat::SetCatAnimState(CatAnimState newState) {
    if (newState == m_CurrentAnimState) return ;
    m_CurrentAnimState = newState;

    std::shared_ptr<Util::Animation> target;
    switch (newState) {
        case CatAnimState::STAND:     target = m_StandAnim;    break;
        case CatAnimState::RUN:       target = m_RunAnim;      break;
        case CatAnimState::JUMP_RISE: target = m_JumpRiseAnim; break;
        case CatAnimState::JUMP_FALL: target = m_JumpFallAnim; break;
        case CatAnimState::LAND:      target = m_LandAnim;     break;
        case CatAnimState::PUSH:      target = m_PushAnim;     break;
    }

    if (target) {
        SetDrawable(target);
        target->SetCurrentFrame(0);
        target->Play();
    }
}
