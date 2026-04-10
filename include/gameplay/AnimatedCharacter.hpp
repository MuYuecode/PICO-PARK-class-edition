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
        const auto animation = GetAnimation();
        return animation != nullptr && animation->GetLooping();
    }

    [[nodiscard]] bool IsPlaying() const {
        const auto animation = GetAnimation();
        return animation != nullptr &&
               animation->GetState() == Util::Animation::State::PLAY;
    }

    void SetLooping(bool looping) const {
        const auto animation = GetAnimation();
        if (animation != nullptr) {
            animation->SetLooping(looping);
        }
    }

    void Play() const {
        const auto animation = GetAnimation();
        if (animation != nullptr) {
            animation->Play();
        }
    }

    [[nodiscard]] bool IfAnimationEnds() const;

private:
    [[nodiscard]] std::shared_ptr<Util::Animation> GetAnimation() const;
};

#endif //ANIMATED_CHARACTER_HPP

