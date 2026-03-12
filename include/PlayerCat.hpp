//
// Created by cody2 on 2026/3/12.
//

#ifndef PLAYER_CAT_HPP
#define PLAYER_CAT_HPP

#include "AnimatedCharacter.hpp"
#include "Character.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"

class PlayerCat : public AnimatedCharacter {
public:
    // 建構子：傳入動畫路徑，以及專屬的控制按鍵
    PlayerCat(const std::vector<std::string>& animationPaths,
              Util::Keycode leftKey, Util::Keycode rightKey, Util::Keycode jumpKey)
        : AnimatedCharacter(animationPaths), m_LeftKey(leftKey), m_RightKey(rightKey), m_JumpKey(jumpKey) {}

    // 給外部呼叫的更新函式
    void Update(std::shared_ptr<Character> floor) ;

    void SetInputEnabled(bool enabled) { m_InputEnabled = enabled; }

    [[nodiscard]] const glm::vec2& GetPosition() const { return m_Transform.translation; }
    void SetPosition(const glm::vec2& Position) { m_Transform.translation = Position; }
    void SetScale(const glm::vec2& Scale) { m_Transform.scale = Scale; }

private:
    bool m_InputEnabled = true;
    Util::Keycode m_LeftKey;
    Util::Keycode m_RightKey;
    Util::Keycode m_JumpKey;

    // Manually tracked state for the jump key.
    bool m_JumpKeyIsHeld = false;

    float m_MoveSpeed = 5.0f;
    glm::vec2 m_Velocity = {0, 0};
    float m_JumpForce = 11.0f;
    float m_Gravity = 0.75f;
    bool m_IsGrounded = true; // Start on the ground
};

#endif