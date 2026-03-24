//
// Created by cody2 on 2026/3/19.
//
#include "CharacterPhysicsSystem.hpp"
#include <algorithm>
#include <cmath>

// 主要更新介面
void CharacterPhysicsSystem::Update(std::vector<PhysicsAgent>& agents,
                                     const std::shared_ptr<Character>& floor) {
    const int n = static_cast<int>(agents.size());

    for (int i = 0; i < n; ++i) {
        auto& self = agents[i];
        if (self.actor == nullptr) continue;

        self.state.prevGrounded = self.state.grounded;

        // 被踩角色跟隨支撐者水平移動
        const int sup = self.state.supportIndex;
        if (self.state.grounded && sup >= 0 && sup < n &&
            agents[sup].actor != nullptr) {
            const float carry = agents[sup].state.lastDeltaX;
            self.actor->SetPosition({self.actor->GetPosition().x + carry,
                                     self.actor->GetPosition().y});
        }

        // 頭頂阻擋
        if (self.state.beingStoodOn && self.state.velocityY > 0.0f) {
            self.state.velocityY = 0.0f;
        }

        // 水平移動
        const float moveSpeed = (self.state.grounded && sup >= 0)
                                    ? kRunOnPlayerSpeed
                                    : kGroundMoveSpeed;

        const float prevX     = self.actor->GetPosition().x;
        const float rawX      = prevX + static_cast<float>(self.state.moveDir) * moveSpeed;
        const float resolvedX = ResolveHorizontal(i, rawX, agents);

        // 推動偵測：想移動但被阻擋
        const bool isPushing = (self.state.moveDir != 0) &&
                               (std::abs(resolvedX - rawX) >= moveSpeed * 0.5f);

        // 面向
        const float faceScale = std::abs(self.actor->m_Transform.scale.x);
        if      (self.state.moveDir < 0) self.actor->m_Transform.scale.x = -faceScale;
        else if (self.state.moveDir > 0) self.actor->m_Transform.scale.x =  faceScale;

        self.actor->SetPosition({resolvedX, self.actor->GetPosition().y});

        // 垂直物理
        ResolveVertical(i, agents, floor);

        self.state.lastDeltaX = self.actor->GetPosition().x - prevX;

        // 動畫狀態
        UpdateAnimState(i, agents, isPushing);
    }

    // 計算 beingStoodOn(供下一幀跳躍判斷用)
    for (int i = 0; i < n; ++i) agents[i].state.beingStoodOn = false;
    for (int j = 0; j < n; ++j) {
        const int s = agents[j].state.supportIndex;
        if (s >= 0 && s < n) agents[s].state.beingStoodOn = true;
    }
}

// ResolveHorizontal(public，供 Scene 個別呼叫)
float CharacterPhysicsSystem::ResolveHorizontal(const int idx,
                                                  float targetX,
                                                  const std::vector<PhysicsAgent>& agents) {
    if (idx < 0 || idx >= static_cast<int>(agents.size()) ||
        agents[idx].actor == nullptr) {
        return targetX;
    }

    const glm::vec2 mePos    = agents[idx].actor->GetPosition();
    const float     currentX = mePos.x;

    for (int j = 0; j < static_cast<int>(agents.size()); ++j) {
        if (j == idx || agents[j].actor == nullptr) continue;

        // 跳過垂直堆疊關係(踩頭)，避免支撐者被誤判為水平障礙
        if (agents[idx].state.supportIndex == j) continue;
        if (agents[j].state.supportIndex == idx) continue;

        const glm::vec2 other   = agents[j].actor->GetPosition();
        const float     vertGap = std::abs(other.y - mePos.y);
        const float vertTolerance = HalfHeight() * 1.4f; // ≈ 32.2f
        if (vertGap >= vertTolerance) continue;

        const float minDist = HalfWidth() + HalfWidth();

        if (targetX > currentX && other.x > currentX) {
            targetX = std::min(targetX, other.x - minDist);
        }
        else if (targetX < currentX && other.x < currentX) {
            targetX = std::max(targetX, other.x + minDist);
        }
    }

    const float boundX = kScreenHalfW - HalfWidth();
    targetX = std::clamp(targetX, -boundX, boundX);

    return targetX;
}

// HalfWidth / HalfHeight(固定碰撞尺寸，避免動畫切換導致尺寸突變)
float CharacterPhysicsSystem::HalfWidth()  { return 18.0f; }
float CharacterPhysicsSystem::HalfHeight() { return 23.0f; }

// StandOffset(動態：floor 半高 + 角色半高)
float CharacterPhysicsSystem::StandOffset(const std::shared_ptr<Character>& floor) {
    const float floorHalfH = (floor != nullptr)
                                 ? std::abs(floor->GetScaledSize().y) / 2.0f
                                 : 0.0f;
    // 固定角色物理半高，不要使用 GetScaledSize()，避免切換動畫時高度突變導致穿板
    const float catHalfH = HalfHeight();
    return floorHalfH + catHalfH;
}

// ResolveVertical：重力 + 地板碰撞 + 踩在角色頭上
void CharacterPhysicsSystem::ResolveVertical(int idx,
                                              std::vector<PhysicsAgent>& agents,
                                              const std::shared_ptr<Character>& floor) {
    auto& self = agents[idx];
    if (self.actor == nullptr) return;

    const float oldY = self.actor->GetPosition().y;

    self.state.velocityY -= kGravity;
    const float targetY = oldY + self.state.velocityY;

    float bestLandingY = -1e9f;
    int   bestSupport  = -2;   // -2=none, -1=floor, >=0=another character

    // 地板(StandOffset 已精簡簽名，不再傳入 idx/agents)
    if (floor != nullptr) {
        const float landingY = floor->GetPosition().y + StandOffset(floor);
        if (oldY >= landingY - 1.0f && targetY <= landingY) {
            bestLandingY = landingY;
            bestSupport  = -1;
        }
    }

    // 踩在其他角色頭上
    const glm::vec2 mePos = self.actor->GetPosition();
    for (int j = 0; j < static_cast<int>(agents.size()); ++j) {
        if (j == idx || agents[j].actor == nullptr) continue;

        const glm::vec2 other = agents[j].actor->GetPosition();
        const float bandX     = (HalfWidth() + HalfWidth()) * 0.6f;
        if (std::abs(other.x - mePos.x) >= bandX) continue;

        const float landingY = other.y + HalfHeight() * 2.0f;

        if (oldY >= landingY - 1.0f && targetY <= landingY && landingY > bestLandingY) {
            bestLandingY = landingY;
            bestSupport  = j;
        }
    }

    if (bestSupport != -2) {
        self.actor->SetPosition({mePos.x, bestLandingY});
        self.state.velocityY    = 0.0f;
        self.state.grounded     = true;
        self.state.supportIndex = bestSupport;
    }
    else {
        self.actor->SetPosition({mePos.x, targetY});
        self.state.grounded     = false;
        self.state.supportIndex = -1;
    }
}

bool CharacterPhysicsSystem::IsBeingPushed(int idx,
                                            const std::vector<PhysicsAgent>& agents,
                                            const std::shared_ptr<Character>& /*floor*/) {
    if (idx < 0 || idx >= static_cast<int>(agents.size()) || agents[idx].actor == nullptr) {
        return false;
    }

    const glm::vec2 mePos = agents[idx].actor->GetPosition();

    for (int j = 0; j < static_cast<int>(agents.size()); ++j) {
        if (j == idx || agents[j].actor == nullptr) continue;
        if (agents[j].state.moveDir == 0) continue;

        const glm::vec2 other   = agents[j].actor->GetPosition();
        const float     vertGap = std::abs(other.y - mePos.y);
        if (vertGap >= HalfHeight()) continue;

        const float minDist = HalfWidth() + HalfWidth();
        const float dx      = mePos.x - other.x;

        if (agents[j].state.moveDir > 0 && dx > 0.0f && std::abs(dx) <= minDist * 1.1f)
            return true;
        if (agents[j].state.moveDir < 0 && dx < 0.0f && std::abs(dx) <= minDist * 1.1f)
            return true;
    }
    return false;
}

// UpdateAnimState
void CharacterPhysicsSystem::UpdateAnimState(int idx,
                                              const std::vector<PhysicsAgent>& agents,
                                              bool isPushing) {
    auto& self = agents[idx];
    if (self.actor == nullptr) return;

    const bool         justLanded = self.state.grounded && !self.state.prevGrounded;
    const CatAnimState cur        = self.actor->GetCatAnimState();

    CatAnimState next;

    if (!self.state.grounded) {
        next = (self.state.velocityY > 0.0f)
                   ? CatAnimState::JUMP_RISE
                   : CatAnimState::JUMP_FALL;
    }
    else if (justLanded || (cur == CatAnimState::LAND && !self.actor->IfAnimationEnds())) {
        next = CatAnimState::LAND;
    }
    else if (isPushing) {
        next = CatAnimState::PUSH;
    }
    else if (self.state.moveDir != 0) {
        next = CatAnimState::RUN;
    }
    else {
        next = CatAnimState::STAND;
    }

    self.actor->SetCatAnimState(next);
}