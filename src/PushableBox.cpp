//
// Created by cody2 on 2026/3/27.
//

#include "PushableBox.hpp"
#include "PhysicsWorld.hpp"
#include "Util/Logger.hpp"
#include <algorithm>
#include <cmath>
#include <string>

PushableBox::PushableBox(const std::string& imagePath, int requiredPushers)
    : Character(imagePath)
    , m_RequiredPushers(requiredPushers)
{
    const Util::Color orange = Util::Color::FromRGB(255, 140, 0, 255);
    m_CountText = std::make_shared<GameText>(
        std::to_string(requiredPushers), 30, orange);
}

void PushableBox::SetPosition(const glm::vec2& pos) {
    m_Transform.translation = pos;
}

void PushableBox::PhysicsUpdate() {
    m_VelocityY -= kGravity;

    int netRight = 0;
    int netLeft  = 0;
    if (m_World != nullptr) {
        netRight = m_World->CountCharactersPushing(this, +1);
        netLeft  = m_World->CountCharactersPushing(this, -1);
    }
    const int net = netRight - netLeft;

    const int deficit = std::max(0, m_RequiredPushers - std::abs(net));
    if (m_CountText != nullptr) {
        m_CountText->SetText(std::to_string(deficit));
    }

    float vx = 0.f;
    if (std::abs(net) >= m_RequiredPushers) {
        vx = (net > 0) ? kPushSpeed : -kPushSpeed;
    }

    m_DesiredDelta = {vx, m_VelocityY};

    if (net > 0) {
        NotifyAdjacentPushers(+1);
    }
    else if (net < 0) {
        NotifyAdjacentPushers(-1);
    }
    else {
        if (netRight > 0) NotifyAdjacentPushers(+1);
        if (netLeft  > 0) NotifyAdjacentPushers(-1);
    }
}

void PushableBox::NotifyAdjacentPushers(int activeDir) const {
    if (m_World == nullptr) return;

    const auto chars = m_World->GetBodiesOfType(BodyType::CHARACTER);

    const glm::vec2 myPos  = GetPosition();
    const glm::vec2 myHalf = GetHalfSize();

    constexpr float kTouchTolerance      = 1.15f;
    constexpr float kVerticalBandFactor  = 0.90f;

    for (auto* ch : chars) {
        if (!ch->IsActive() || ch->IsFrozen()) continue;

        const int moveDir = ch->GetMoveDir();
        if (moveDir == 0) continue;

        const glm::vec2 chPos  = ch->GetPosition();
        const glm::vec2 chHalf = ch->GetHalfSize();

        if (std::abs(chPos.y - myPos.y) >= (chHalf.y + myHalf.y) * kVerticalBandFactor) {
            continue;
        }

        const float dx      = chPos.x - myPos.x;
        const float minDist = chHalf.x + myHalf.x;
        if (std::abs(dx) > minDist * kTouchTolerance) continue;

        const bool charIsOnPushSide =
            (activeDir > 0) ? (dx < 0 && moveDir > 0)
                            : (dx > 0 && moveDir < 0);

        if (charIsOnPushSide) {
            ch->NotifyPush();
        }
    }
}

void PushableBox::ApplyResolvedDelta(const glm::vec2& delta) {
    const glm::vec2 newPos = GetPosition() + delta;

    if (newPos != GetPosition()) SetPosition(newPos);

    if (m_CountText != nullptr) {
        m_CountText->SetPosition({newPos.x+10.f, newPos.y});
    }
}

void PushableBox::OnCollision(const CollisionInfo& info) {
    if (info.normal.y > 0.5f) {
        m_VelocityY = 0.f;
    }
    if (info.normal.y < -0.5f) {
        if (m_VelocityY > 0.f) m_VelocityY = 0.f;
    }
}