//
// Created by cody2 on 2026/3/12.
//

#include "PlayerCat.hpp"

void PlayerCat::Update(std::shared_ptr<Character> floor) {

    // --- Actions based on State ---
    bool isMoving = false;
    if (m_InputEnabled) {
        // Horizontal Movement (IsKeyDown is fine here)
        if (Util::Input::IsKeyPressed(m_RightKey)) {
            m_Transform.translation.x += m_MoveSpeed;
            if (m_Transform.scale.x < 0) m_Transform.scale.x = -m_Transform.scale.x; // Face right
            isMoving = true;
        }
        else if (Util::Input::IsKeyPressed(m_LeftKey)) {
            m_Transform.translation.x -= m_MoveSpeed;
            if (m_Transform.scale.x > 0) m_Transform.scale.x = -m_Transform.scale.x; // Face left
            isMoving = true;
        }

        // Jump Action (using our reliable tracked state)
        if (m_IsGrounded && Util::Input::IsKeyDown(m_JumpKey)) {
            m_Velocity.y = m_JumpForce;
            m_IsGrounded = false; // We are now leaving the ground
        }
    }

    // Update animation based on movement
    if (isMoving) {
        SetLooping(true);
        Play();
    } else {
        SetLooping(false);
    }

    // --- 3. Apply Physics ---
    m_Velocity.y -= m_Gravity;
    m_Transform.translation.y += m_Velocity.y;

    // --- 4. Collision Detection & State Update for Next Frame ---
    float standOffset = 58.5f;
    if (m_Velocity.y <= 0 && floor && m_Transform.translation.y <= floor->GetPosition().y + standOffset) {
        // We have collided with the floor.
        m_Transform.translation.y = floor->GetPosition().y + standOffset;
        m_Velocity.y = 0;
        m_IsGrounded = true;
    } else {
        // We are not colliding with the floor, so we must be in the air.
        m_IsGrounded = false;
    }
}
