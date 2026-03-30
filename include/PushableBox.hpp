//
// Created by cody2 on 2026/3/26.
//

#ifndef PICOPART_PUSHABLEBOX_HPP
#define PICOPART_PUSHABLEBOX_HPP

#include <memory>
#include <string>

#include "Character.hpp"
#include "IPhysicsBody.hpp"
#include "IPushable.hpp"
#include "GameText.hpp"

class PhysicsWorld;
class PushableBox : public Character, public IPhysicsBody, public IPushable {
public:
    static constexpr float kGravity   = 0.5f;
    static constexpr float kPushSpeed = 2.0f;

    PushableBox(const std::string& imagePath, int requiredPushers = 2);

    void SetWorld(PhysicsWorld* world) { m_World = world; }
    void SetRequiredPushers(int requiredPushers);

    [[nodiscard]] int GetRequiredPushers() const override { return m_RequiredPushers; }

    [[nodiscard]] std::shared_ptr<GameText> GetTextObject() const { return m_CountText; }

    [[nodiscard]] BodyType  GetBodyType() const override { return BodyType::PUSHABLE_BOX; }
    [[nodiscard]] glm::vec2 GetPosition() const override { return m_Transform.translation; }
    void                    SetPosition(const glm::vec2& pos) override ;
    [[nodiscard]] glm::vec2 GetHalfSize() const override
                                { return glm::abs(GetScaledSize()) * 0.5f; }

    [[nodiscard]] bool IsSolid()     const override { return true;  }
    [[nodiscard]] bool IsKinematic() const override { return false; }

    void PhysicsUpdate() override;

    [[nodiscard]] glm::vec2 GetDesiredDelta() const override { return m_DesiredDelta; }

    void ApplyResolvedDelta(const glm::vec2& delta) override;

    void OnCollision(const CollisionInfo& info) override;

private:
    void NotifyAdjacentPushers(int activeDir) const;

    PhysicsWorld* m_World           = nullptr;
    int           m_RequiredPushers = 1;
    float         m_VelocityY       = 0.f;
    glm::vec2     m_DesiredDelta    = {0.f, 0.f};

    std::shared_ptr<GameText> m_CountText ;
};

#endif // PICOPART_PUSHABLEBOX_HPP