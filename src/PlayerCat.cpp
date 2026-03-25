//
// Created by cody2 on 2026/3/12.
//

#include "PlayerCat.hpp"

namespace {

std::shared_ptr<Util::Animation> MakeAnim(const std::vector<std::string>& paths,
                                           int intervalMs,
                                           bool looping) {
    if (paths.empty()) return nullptr;
    // play=false(由 SetCatAnimState 觸發)，cooldown=0
    return std::make_shared<Util::Animation>(paths, false, intervalMs, looping, 0);
}

} // namespace

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

void PlayerCat::SetCatAnimState(CatAnimState newState) {
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
        m_Drawable = target;
        target->Play();
    }
}