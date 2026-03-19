//
// Created by cody2 on 2026/3/12.
//

#include "PlayerCat.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// 工具：建立 Util::Animation
// ─────────────────────────────────────────────────────────────────────────────
namespace {

std::shared_ptr<Util::Animation> MakeAnim(const std::vector<std::string>& paths,
                                           int intervalMs,
                                           bool looping) {
    if (paths.empty()) return nullptr;
    // play=false（由 SetCatAnimState 觸發），cooldown=0
    return std::make_shared<Util::Animation>(paths, false, intervalMs, looping, 0);
}

} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// 建構子
// ─────────────────────────────────────────────────────────────────────────────
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
    // 初始顯示 STAND，直接播放
    if (m_StandAnim) {
        m_StandAnim->Play();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 建立各動畫 clip
// ─────────────────────────────────────────────────────────────────────────────
void PlayerCat::BuildClips(const CatAnimPaths& paths) {
    // stand：AnimatedCharacter 建構子已用 stand paths 建立 m_Drawable，直接重用
    m_StandAnim = std::dynamic_pointer_cast<Util::Animation>(m_Drawable);
    if (m_StandAnim) {
        m_StandAnim->SetLooping(true);
        m_StandAnim->SetInterval(400);
    }

    m_RunAnim      = MakeAnim(paths.run,       120, true);
    m_JumpRiseAnim = MakeAnim(paths.jump_rise, 150, true);   // jump_1：循環
    m_JumpFallAnim = MakeAnim(paths.jump_fall, 150, true);   // jump_2：循環
    m_LandAnim     = MakeAnim(paths.land,       80, false);  // land_1：播一次
    m_PushAnim     = MakeAnim(paths.push,      120, true);
}

// ─────────────────────────────────────────────────────────────────────────────
// 切換動畫狀態
// ─────────────────────────────────────────────────────────────────────────────
void PlayerCat::SetCatAnimState(CatAnimState newState) {
    // LAND 播放中不被打斷（除非動畫結束後由外部再次呼叫）
    if (m_CurrentAnimState == CatAnimState::LAND &&
        newState != CatAnimState::LAND &&
        !IfAnimationEnds()) {
        return;
    }

    if (newState == m_CurrentAnimState) return;
    m_CurrentAnimState = newState;

    std::shared_ptr<Util::Animation> target = nullptr;
    switch (newState) {
        case CatAnimState::STAND:     target = m_StandAnim;     break;
        case CatAnimState::RUN:       target = m_RunAnim;       break;
        case CatAnimState::JUMP_RISE: target = m_JumpRiseAnim;  break;
        case CatAnimState::JUMP_FALL: target = m_JumpFallAnim;  break;
        case CatAnimState::LAND:      target = m_LandAnim;      break;
        case CatAnimState::PUSH:      target = m_PushAnim;      break;
    }

    if (target) {
        m_Drawable = target;   // m_Drawable 是 protected，子類可直接寫入
        target->Play();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 舊版簡易物理更新（PlayerCat 自行管理）
// 問題 1 修正：standOffset = floor 半高 + 角色半高（動態）
// 跳躍動畫：上升 → JUMP_RISE，下降 → JUMP_FALL，落地瞬間 → LAND
// ─────────────────────────────────────────────────────────────────────────────
void PlayerCat::Update(std::shared_ptr<Character>& floor) {
    m_WasGrounded = m_IsGrounded;

    bool isMoving = false;

    if (m_InputEnabled) {
        if (Util::Input::IsKeyPressed(m_RightKey)) {
            m_Transform.translation.x += m_MoveSpeed;
            if (m_Transform.scale.x < 0) m_Transform.scale.x = -m_Transform.scale.x;
            isMoving = true;
        }
        else if (Util::Input::IsKeyPressed(m_LeftKey)) {
            m_Transform.translation.x -= m_MoveSpeed;
            if (m_Transform.scale.x > 0) m_Transform.scale.x = -m_Transform.scale.x;
            isMoving = true;
        }

        if (m_IsGrounded && Util::Input::IsKeyDown(m_JumpKey)) {
            m_Velocity.y  = m_JumpForce;
            m_IsGrounded  = false;
        }
    }

    // ── 物理 ──────────────────────────────────────────────────────────────
    m_Velocity.y -= m_Gravity;
    m_Transform.translation.y += m_Velocity.y;

    // ── 地板碰撞（動態 standOffset）───────────────────────────────────────
    if (floor) {
        const float floorHalfH  = floor->GetScaledSize().y / 2.0f;
        const float charHalfH   = this->GetScaledSize().y  / 2.0f;
        const float standOffset = floorHalfH + charHalfH;
        const float landingY    = floor->GetPosition().y + standOffset;

        if (m_Velocity.y <= 0.0f && m_Transform.translation.y <= landingY) {
            m_Transform.translation.y = landingY;
            m_Velocity.y              = 0.0f;
            m_IsGrounded              = true;
        }
        else {
            m_IsGrounded = false;
        }
    }
    else {
        m_IsGrounded = false;
    }

    // ── 動畫狀態決策 ──────────────────────────────────────────────────────
    const bool justLanded = m_IsGrounded && !m_WasGrounded;

    // LAND 播放中由 SetCatAnimState 內部守衛保護，不會被打斷
    if (!m_IsGrounded) {
        // 空中：依速度方向決定 JUMP_RISE / JUMP_FALL
        if (m_Velocity.y > 0.0f) {
            SetCatAnimState(CatAnimState::JUMP_RISE);
        }
        else {
            SetCatAnimState(CatAnimState::JUMP_FALL);
        }
    }
    else if (justLanded) {
        SetCatAnimState(CatAnimState::LAND);
    }
    else if (m_CurrentAnimState == CatAnimState::LAND && IfAnimationEnds()) {
        // LAND 播完 → 回到 STAND / RUN
        SetCatAnimState(isMoving ? CatAnimState::RUN : CatAnimState::STAND);
    }
    else if (isMoving) {
        SetCatAnimState(CatAnimState::RUN);
    }
    else {
        SetCatAnimState(CatAnimState::STAND);
    }
}