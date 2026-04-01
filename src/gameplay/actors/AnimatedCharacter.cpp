#include "gameplay/actors/AnimatedCharacter.hpp"

#include <cmath>

AnimatedCharacter::AnimatedCharacter(const std::vector<std::string>& AnimationPaths) {
    m_Drawable = std::make_shared<Util::Animation>(AnimationPaths, false, 500, false, 0);
}

glm::vec2 AnimatedCharacter::GetPosition() const {
    return m_Transform.translation;
}

void AnimatedCharacter::SetPosition(const glm::vec2& position) {
    m_Transform.translation = position;
}

glm::vec2 AnimatedCharacter::GetScale() const {
    return m_Transform.scale;
}

void AnimatedCharacter::SetScale(const glm::vec2& scale) {
    m_Transform.scale = scale;
}

float AnimatedCharacter::GetScaleX() const {
    return m_Transform.scale.x;
}

void AnimatedCharacter::SetScaleX(float scaleX) {
    m_Transform.scale.x = scaleX;
}

void AnimatedCharacter::SetFacingByDirection(int dir) {
    if (dir == 0) return;
    const float absX = std::abs(m_Transform.scale.x);
    m_Transform.scale.x = (dir < 0) ? -absX : absX;
}

std::shared_ptr<Util::Animation> AnimatedCharacter::GetAnimation() const {
    return std::dynamic_pointer_cast<Util::Animation>(m_Drawable);
}

bool AnimatedCharacter::IfAnimationEnds() const {
    const auto animation = GetAnimation();
    if (animation == nullptr) {
        return false;
    }
    return animation->GetCurrentFrameIndex() == animation->GetFrameCount() - 1;
}

