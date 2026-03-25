//
// Created by cody2 on 2026/3/12.
//

#ifndef PLAYER_CAT_HPP
#define PLAYER_CAT_HPP

#include "AnimatedCharacter.hpp"
#include "Character.hpp"
#include "IPhysicsBody.hpp"
#include "Util/Animation.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"

enum class CatAnimState {
    STAND,
    RUN,
    JUMP_RISE,
    JUMP_FALL,
    LAND,
    PUSH,
};

struct CatAnimPaths {
    std::vector<std::string> stand;
    std::vector<std::string> run;
    std::vector<std::string> jump_rise;
    std::vector<std::string> jump_fall;
    std::vector<std::string> land;
    std::vector<std::string> push;
};

class PlayerCat : public AnimatedCharacter, public IPhysicsBody {
public:
    PlayerCat(const CatAnimPaths& animPaths,
              Util::Keycode leftKey,
              Util::Keycode rightKey,
              Util::Keycode jumpKey);

    void SetCatAnimState(CatAnimState newState);
    [[nodiscard]] CatAnimState GetCatAnimState() const { return m_CurrentAnimState; }

    void SetInputEnabled(bool enabled)         { m_InputEnabled = enabled; }
    [[nodiscard]] bool GetInputEnabled() const { return m_InputEnabled;    }

    void SetPosition(const glm::vec2& pos) override { m_Transform.translation = pos; }
    void SetScale(const glm::vec2& scale)   { m_Transform.scale = scale; }
    void SetVelocity(const glm::vec2& vel) override { m_Velocity = vel; }

    [[nodiscard]] BodyType GetBodyType() const override { return BodyType::CHARACTER; }
    [[nodiscard]] glm::vec2 GetPosition() const override { return m_Transform.translation; }
    [[nodiscard]] glm::vec2 GetHalfSize() const override { return glm::abs(GetScaledSize()) * 0.5f; }
    [[nodiscard]] glm::vec2 GetVelocity() const override { return m_Velocity; }

    [[nodiscard]] bool IsSolid() const override { return true; }
    [[nodiscard]] bool IsKinematic() const override { return false; }
    [[nodiscard]] bool UseGravity() const override { return true; }
    void OnCollision(const CollisionInfo& /*info*/) override {}

    [[nodiscard]] Util::Keycode GetLeftKey()  const { return m_LeftKey;  }
    [[nodiscard]] Util::Keycode GetRightKey() const { return m_RightKey; }
    [[nodiscard]] Util::Keycode GetJumpKey()  const { return m_JumpKey;  }

private:
    void BuildClips(const CatAnimPaths& paths);

    std::shared_ptr<Util::Animation> m_StandAnim;
    std::shared_ptr<Util::Animation> m_RunAnim;
    std::shared_ptr<Util::Animation> m_JumpRiseAnim;
    std::shared_ptr<Util::Animation> m_JumpFallAnim;
    std::shared_ptr<Util::Animation> m_LandAnim;
    std::shared_ptr<Util::Animation> m_PushAnim;

    CatAnimState m_CurrentAnimState = CatAnimState::STAND;

    bool          m_InputEnabled = true;
    Util::Keycode m_LeftKey;
    Util::Keycode m_RightKey;
    Util::Keycode m_JumpKey;

    glm::vec2 m_Velocity = {0.0f, 0.0f};
};

#endif // PLAYER_CAT_HPP