//
// Created by cody2 on 2026/3/12.
//

#ifndef PLAYER_CAT_HPP
#define PLAYER_CAT_HPP

#include "gameplay/actors/AnimatedCharacter.hpp"
#include "gameplay/actors/Character.hpp"
#include "physics/body/IPhysicsBody.hpp"
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
    static constexpr float kGravity          = 0.75f;
    static constexpr float kJumpForce        = 11.0f;
    static constexpr float kGroundMoveSpeed  = 5.0f;
    static constexpr float kRunOnPlayerSpeed = 6.2f;
    static constexpr float kHalfWidth        = 18.0f;
    static constexpr float kHalfHeight       = 23.0f;

    PlayerCat(const CatAnimPaths& animPaths,
              Util::Keycode leftKey,
              Util::Keycode rightKey,
              Util::Keycode jumpKey);

    void SetCatAnimState(CatAnimState newState);
    [[nodiscard]] CatAnimState GetCatAnimState() const { return m_CurrentAnimState; }
    [[nodiscard]] Util::Keycode GetLeftKey()  const { return m_LeftKey;  }
    [[nodiscard]] Util::Keycode GetRightKey() const { return m_RightKey; }
    [[nodiscard]] Util::Keycode GetJumpKey()  const { return m_JumpKey;  }

    void SetMoveDir(int dir)               { m_MoveDir = dir; }

    void Jump();

    [[nodiscard]] bool IsGrounded() const { return m_Grounded; }

    void SetInputEnabled(bool enabled)         { m_InputEnabled = enabled; }
    [[nodiscard]] bool GetInputEnabled() const { return m_InputEnabled;    }

    void SetScale(const glm::vec2& scale) { m_Transform.scale = scale; }

    [[nodiscard]] const PhysicsBodyTraits& GetPhysicsTraits() const override {
        static const PhysicsBodyTraits kTraits{BodyType::CHARACTER, true, true};
        return kTraits;
    }
    [[nodiscard]] glm::vec2 GetPosition() const override { return m_Transform.translation; }
    void                    SetPosition(const glm::vec2& pos) override
                                { m_Transform.translation = pos; }
    [[nodiscard]] glm::vec2 GetHalfSize() const override
                                { return {kHalfWidth, kHalfHeight}; }

    [[nodiscard]] bool IsSolid()     const override { return true;  }
    [[nodiscard]] bool IsKinematic() const override { return false; }
    [[nodiscard]] int  GetMoveDir()  const override { return m_MoveDir; }

    void PhysicsUpdate() override;

    [[nodiscard]] glm::vec2 GetDesiredDelta() const override { return m_DesiredDelta; }

    void ApplyResolvedDelta(const glm::vec2& delta) override;

    void OnCollision(const CollisionInfo& info) override;

    void PostUpdate() override;

    void NotifyPush() override { m_IsPushing = true; }

private:
    void BuildClips(const CatAnimPaths& paths);
    void UpdateAnimState();

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

    int       m_MoveDir      = 0;
    float     m_VelocityY    = 0.f;
    bool      m_Grounded     = true;
    bool      m_PrevGrounded = true;
    bool      m_IsPushing    = false;

    glm::vec2 m_DesiredDelta = {0.f, 0.f};
};

#endif // PLAYER_CAT_HPP
