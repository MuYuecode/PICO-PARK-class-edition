#ifndef ANIMATED_CHARACTER_HPP
#define ANIMATED_CHARACTER_HPP

#include <vector>
#include <string>

#include "Util/Animation.hpp"
#include "Util/GameObject.hpp"


class AnimatedCharacter : public Util::GameObject {

public:
    explicit AnimatedCharacter(const std::vector<std::string>& AnimationPaths);

    [[nodiscard]] glm::vec2 GetPosition() const;
    void SetPosition(const glm::vec2& position);

    [[nodiscard]] glm::vec2 GetScale() const;
    void SetScale(const glm::vec2& scale);

    [[nodiscard]] float GetScaleX() const;
    void SetScaleX(float scaleX);

    void SetFacingByDirection(int dir);

    [[nodiscard]] bool IsLooping() const {
        return std::dynamic_pointer_cast<Util::Animation>(m_Drawable)->GetLooping();
    }

    [[nodiscard]] bool IsPlaying() const {
        return std::dynamic_pointer_cast<Util::Animation>(m_Drawable)->GetState() == Util::Animation::State::PLAY;
    }

    void SetLooping(bool looping) const {
        auto temp = std::dynamic_pointer_cast<Util::Animation>(m_Drawable) ;
        temp->SetLooping(looping);
    }

    void Play() const {
        auto temp = std::dynamic_pointer_cast<Util::Animation>(m_Drawable);
        temp->Play();
    }
    [[nodiscard]] bool IfAnimationEnds() const;

};

#endif //ANIMATED_CHARACTER_HPP
