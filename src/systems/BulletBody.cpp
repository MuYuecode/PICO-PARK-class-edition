#include "systems/BulletBody.hpp"

#include <algorithm>

#include "Util/Time.hpp"

BulletBody::BulletBody(const glm::vec2& pos, const glm::vec2& half)
    : m_Pos(pos), m_Half(glm::abs(half)) {
    SetActive(false);
}

const PhysicsBodyTraits& BulletBody::GetPhysicsTraits() const {
    static const PhysicsBodyTraits kTraits{BodyType::BULLET, false, false};
    return kTraits;
}

glm::vec2 BulletBody::GetPosition() const {
    return m_Pos;
}

void BulletBody::SetPosition(const glm::vec2& pos) {
    m_Pos = pos;
}

glm::vec2 BulletBody::GetHalfSize() const {
    return m_Half;
}

bool BulletBody::IsSolid() const {
    return false;
}

bool BulletBody::IsKinematic() const {
    return false;
}

int BulletBody::GetMoveDir() const {
    return 0;
}

void BulletBody::SetSpeed(float speed) {
    m_Speed = std::max(0.0f, speed);
}

BulletBody::HitType BulletBody::ConsumeHitType() {
    const HitType hitType = m_HitType;
    m_HitType = HitType::None;
    return hitType;
}

void BulletBody::PhysicsUpdate() {
    const float dtSec = Util::Time::GetDeltaTimeMs() / 1000.0f;
    m_Desired = {-m_Speed * std::max(0.0f, dtSec), 0.0f};
}

glm::vec2 BulletBody::GetDesiredDelta() const {
    return m_Desired;
}

void BulletBody::ApplyResolvedDelta(const glm::vec2& delta) {
    m_Pos += delta;
}

void BulletBody::OnCollision(const CollisionInfo& info) {
    if (info.other == nullptr) {
        if (m_HitType == HitType::None) {
            m_HitType = HitType::Solid;
        }
        return;
    }

    switch (info.other->GetPhysicsTraits().type) {
        case BodyType::CHARACTER:
            m_HitType = HitType::Character;
            break;
        case BodyType::JAR:
            if (m_HitType != HitType::Character) {
                m_HitType = HitType::Jar;
            }
            break;
        default:
            if (m_HitType == HitType::None) {
                m_HitType = HitType::Solid;
            }
            break;
    }
}

void BulletBody::PostUpdate() {}
