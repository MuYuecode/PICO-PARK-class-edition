#ifndef BULLET_BODY_HPP
#define BULLET_BODY_HPP

#include "physics/IPhysicsBody.hpp"

class BulletBody final : public IPhysicsBody {
public:
    enum class HitType {
        None,
        Solid,
        Jar,
        Character,
    };

    BulletBody(const glm::vec2& pos, const glm::vec2& half);

    [[nodiscard]] const PhysicsBodyTraits& GetPhysicsTraits() const override;
    [[nodiscard]] glm::vec2 GetPosition() const override;
    void SetPosition(const glm::vec2& pos) override;
    [[nodiscard]] glm::vec2 GetHalfSize() const override;

    [[nodiscard]] bool IsSolid() const override;
    [[nodiscard]] bool IsKinematic() const override;
    [[nodiscard]] int GetMoveDir() const override;

    void SetSpeed(float speed);
    [[nodiscard]] HitType ConsumeHitType();

    void PhysicsUpdate() override;
    [[nodiscard]] glm::vec2 GetDesiredDelta() const override;
    void ApplyResolvedDelta(const glm::vec2& delta) override;
    void OnCollision(const CollisionInfo& info) override;
    void PostUpdate() override;

private:
    glm::vec2 m_Pos;
    glm::vec2 m_Half;
    glm::vec2 m_Desired = {0.0f, 0.0f};
    HitType m_HitType = HitType::None;
    float m_Speed = 0.0f;
};

#endif // BULLET_BODY_HPP
