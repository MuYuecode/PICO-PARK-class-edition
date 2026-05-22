//
// Created by cody2 on 2026/3/26.
//

#ifndef PICOPART_PUSHABLEBOX_HPP
#define PICOPART_PUSHABLEBOX_HPP

#include <memory>
#include <string>

#include "game/Character.hpp"
#include "systems/IPhysicsBody.hpp"
#include "game/GameText.hpp"
#include "systems/IPushQueryService.hpp"

class PushableBox : public Character, public IPhysicsBody {
public:
    static constexpr float kGravity   = 0.5f;
    static constexpr float kPushSpeed = 2.0f;

    PushableBox(const std::string& imagePath, int requiredPushers = 2);

    void SetPushQuery(const IPushQueryService* query) { m_PushQuery = query; }
    void SetRequiredPushers(int requiredPushers);

    [[nodiscard]] std::shared_ptr<GameText> GetTextObject() const { return m_CountText; }

    [[nodiscard]] const PhysicsBodyTraits& GetPhysicsTraits() const override {
        static const PhysicsBodyTraits kTraits{BodyType::PUSHABLE_BOX};
        return kTraits;
    }
    [[nodiscard]] glm::vec2 GetPosition() const override { return m_Transform.translation; }
    void                    SetPosition(const glm::vec2& pos) override ;
    [[nodiscard]] glm::vec2 GetHalfSize() const override
                                { return glm::abs(GetScaledSize()) * 0.5f; }

    [[nodiscard]] bool IsSolid()     const override { return true;  }
    [[nodiscard]] bool IsKinematic() const override { return false; }
    [[nodiscard]] int GetMoveDir() const override { return 0; }

    void PhysicsUpdate() override;

    [[nodiscard]] glm::vec2 GetDesiredDelta() const override { return m_DesiredDelta; }

    void ApplyResolvedDelta(const glm::vec2& delta) override;

    void OnCollision(const CollisionInfo& info) override;

    void PostUpdate() override {}

private:
    void NotifyAdjacentPushers(int activeDir) const;

    const IPushQueryService* m_PushQuery = nullptr;
    int           m_RequiredPushers = 1;
    float         m_VelocityY       = 0.f;
    glm::vec2     m_DesiredDelta    = {0.f, 0.f};

    std::shared_ptr<GameText> m_CountText ;
};

#endif // PICOPART_PUSHABLEBOX_HPP
