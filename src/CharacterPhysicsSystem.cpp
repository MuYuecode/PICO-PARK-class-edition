//
// Created by cody2 on 2026/3/19.
//

#include "CharacterPhysicsSystem.hpp"

#include <algorithm>
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
// Update：主要更新介面
// ─────────────────────────────────────────────────────────────────────────────
void CharacterPhysicsSystem::Update(std::vector<PhysicsAgent>& agents,
                                     const std::shared_ptr<Character>& floor) const {
    const int n = static_cast<int>(agents.size());

    for (int i = 0; i < n; ++i) {
        auto& self = agents[i];
        if (self.actor == nullptr) continue;

        self.state.prevGrounded = self.state.grounded;

        // ── 1. 攜帶：被踩角色跟隨支撐者水平移動 ──────────────────────────
        const int sup = self.state.supportIndex;
        if (self.state.grounded && sup >= 0 && sup < n &&
            agents[sup].actor != nullptr) {
            const float carry = agents[sup].state.lastDeltaX;
            self.actor->SetPosition({self.actor->GetPosition().x + carry,
                                     self.actor->GetPosition().y});
        }

        // ── 2. 跳躍 ───────────────────────────────────────────────────────
        // 按鍵讀取由場景負責設定 state.moveDir；跳躍同理。
        // 這裡只處理「已請求跳躍」且條件符合的情況。
        // ※ 呼叫前 Scene 必須把 wantJump 寫入 agent.state，
        //   或直接在 Update 前設好 velocityY。
        // （見下方 NOTE）

        // ── 3. 水平移動 ───────────────────────────────────────────────────
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

        // ── 4. 垂直物理 ───────────────────────────────────────────────────
        ResolveVertical(i, agents, floor);

        self.state.lastDeltaX = self.actor->GetPosition().x - prevX;

        // ── 5. 被推偵測 ───────────────────────────────────────────────────
        const bool beingPushed = IsBeingPushed(i, agents, floor);

        // ── 6. 動畫狀態 ───────────────────────────────────────────────────
        UpdateAnimState(i, agents, isPushing, beingPushed);
    }

    // ── 7. 計算 beingStoodOn（供下一幀跳躍判斷用）────────────────────────
    // 先全部重置
    for (int i = 0; i < n; ++i) agents[i].state.beingStoodOn = false;
    // 若 j 以 sup 為支撐，則 sup 正被踩
    for (int j = 0; j < n; ++j) {
        const int sup = agents[j].state.supportIndex;
        if (sup >= 0 && sup < n) agents[sup].state.beingStoodOn = true;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// ResolveHorizontal（public，供 Scene 個別呼叫）
// ─────────────────────────────────────────────────────────────────────────────
float CharacterPhysicsSystem::ResolveHorizontal(int idx,
                                                  float targetX,
                                                  const std::vector<PhysicsAgent>& agents) const {
    if (idx < 0 || idx >= static_cast<int>(agents.size()) ||
        agents[idx].actor == nullptr) {
        return targetX;
    }

    const glm::vec2 mePos    = agents[idx].actor->GetPosition();
    const float     currentX = mePos.x;

    for (int j = 0; j < static_cast<int>(agents.size()); ++j) {
        if (j == idx || agents[j].actor == nullptr) continue;

        // ── 修正問題 3/4：跳過垂直堆疊關係（踩頭），避免支撐者被誤判為水平障礙 ──
        // idx 站在 j 上，或 j 站在 idx 上，均不做水平碰撞
        if (agents[idx].state.supportIndex == j) continue;
        if (agents[j].state.supportIndex == idx) continue;

        const glm::vec2 other   = agents[j].actor->GetPosition();
        const float     vertGap = std::abs(other.y - mePos.y);
        // 縮小垂直容差（原 50.0f 超過兩人半高差 46.0f，導致踩頭時支撐者被納入水平碰撞）
        const float vertTolerance = HalfHeight() * 1.4f; // ≈ 32.2f，安全低於 46.0f
        if (vertGap >= vertTolerance) continue;

        const float minDist = HalfWidth() + HalfWidth();

        if (targetX > currentX && other.x > currentX) {
            targetX = std::min(targetX, other.x - minDist);
        }
        else if (targetX < currentX && other.x < currentX) {
            targetX = std::max(targetX, other.x + minDist);
        }
    }
    return targetX;
}

// ─────────────────────────────────────────────────────────────────────────────
// HalfWidth
// ─────────────────────────────────────────────────────────────────────────────
float CharacterPhysicsSystem::HalfWidth() const {
    // 固定碰撞半寬，避免動畫切換導致寬度變動而發生推擠 Bug
    return 18.0f;
}

float CharacterPhysicsSystem::HalfHeight() const {
    return 23.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
// StandOffset（動態：floor 半高 + 角色半高）
// 修正問題 1：角色腳底正確接觸地板/對方頭頂
// ─────────────────────────────────────────────────────────────────────────────
float CharacterPhysicsSystem::StandOffset(int idx,
                                           const std::vector<PhysicsAgent>& agents,
                                           const std::shared_ptr<Character>& floor) const {
    const float floorHalfH = (floor != nullptr)
                                 ? std::abs(floor->GetScaledSize().y) / 2.0f
                                 : 0.0f;
    // 固定角色物理半高，不要使用 GetScaledSize()，避免切換動畫時高度突變導致穿板！
    const float catHalfH = HalfHeight();
    return floorHalfH + catHalfH;
}

// ─────────────────────────────────────────────────────────────────────────────
// HasHeadBlock：頭上是否有人站著（被踩 → 不能跳）
// ─────────────────────────────────────────────────────────────────────────────
bool CharacterPhysicsSystem::HasHeadBlock(int idx,
                                           const std::vector<PhysicsAgent>& agents,
                                           const std::shared_ptr<Character>& floor) const {
    if (idx < 0 || idx >= static_cast<int>(agents.size()) || agents[idx].actor == nullptr) {
        return false;
    }

    const glm::vec2 mePos    = agents[idx].actor->GetPosition();
    const float expectedHeadY = mePos.y + HalfHeight()*2.0f ;

    for (int j = 0; j < static_cast<int>(agents.size()); ++j) {
        if (j == idx || agents[j].actor == nullptr) continue;

        const glm::vec2 other      = agents[j].actor->GetPosition();
        const float     bandX      = (HalfWidth() + HalfWidth()) * 0.6f;
        const bool      horizOk    = std::abs(other.x - mePos.x) < bandX;
        const bool      heightOk   = (other.y > expectedHeadY - 6.0f) &&
                                     (other.y < expectedHeadY + 18.0f);
        if (horizOk && heightOk) return true;
    }
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// ResolveVertical：重力 + 地板碰撞 + 踩在角色頭上
// ─────────────────────────────────────────────────────────────────────────────
void CharacterPhysicsSystem::ResolveVertical(int idx,
                                              std::vector<PhysicsAgent>& agents,
                                              const std::shared_ptr<Character>& floor) const {
    auto& self = agents[idx];
    if (self.actor == nullptr) return;

    const float oldY = self.actor->GetPosition().y;

    self.state.velocityY -= kGravity;
    const float targetY = oldY + self.state.velocityY;

    float bestLandingY = -1e9f;
    int   bestSupport  = -2;   // -2=none, -1=floor, >=0=another character

    // 地板
    if (floor != nullptr) {
        const float landingY = floor->GetPosition().y + StandOffset(idx, agents, floor);
        if (oldY >= landingY - 1.0f && targetY <= landingY) {
            bestLandingY = landingY;
            bestSupport  = -1;
        }
    }

    // 踩在其他角色頭上
    const glm::vec2 mePos = self.actor->GetPosition();
    for (int j = 0; j < static_cast<int>(agents.size()); ++j) {
        if (j == idx || agents[j].actor == nullptr) continue;

        const glm::vec2 other    = agents[j].actor->GetPosition();
        const float     bandX    = (HalfWidth() + HalfWidth()) * 0.6f;
        if (std::abs(other.x - mePos.x) >= bandX) continue;

        // 修正：站在別人頭上，Y 座標應該是「對方的 Y + 兩人的半高總和(56.0f)」
        const float landingY = other.y + HalfHeight()*2.0f ;

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

// ─────────────────────────────────────────────────────────────────────────────
// IsBeingPushed：自己沒動，但旁邊有人正在推（用於推動動畫）
// ─────────────────────────────────────────────────────────────────────────────
bool CharacterPhysicsSystem::IsBeingPushed(int idx,
                                            const std::vector<PhysicsAgent>& agents,
                                            const std::shared_ptr<Character>& floor) const {
    if (idx < 0 || idx >= static_cast<int>(agents.size()) || agents[idx].actor == nullptr) {
        return false;
    }

    const glm::vec2 mePos = agents[idx].actor->GetPosition();

    for (int j = 0; j < static_cast<int>(agents.size()); ++j) {
        if (j == idx || agents[j].actor == nullptr) continue;
        if (agents[j].state.moveDir == 0) continue; // j 靜止，不算推

        const glm::vec2 other   = agents[j].actor->GetPosition();
        const float     vertGap = std::abs(other.y - mePos.y);

        // 修正：同一水平面的判斷不該依賴含有地板厚度的 StandOffset，用角色的物理半高即可
        if (vertGap >= HalfHeight()) continue;

        const float minDist = HalfWidth() + HalfWidth();
        const float dx      = mePos.x - other.x;

        // j 在 me 左邊往右推
        if (agents[j].state.moveDir > 0 && dx > 0.0f && std::abs(dx) <= minDist * 1.1f)
            return true;
        // j 在 me 右邊往左推
        if (agents[j].state.moveDir < 0 && dx < 0.0f && std::abs(dx) <= minDist * 1.1f)
            return true;
    }
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateAnimState：依物理狀態切換動畫
//   空中 → JUMP_RISE（上升）/ JUMP_FALL（下降）
//   落地瞬間 → LAND（播完自動回 STAND/RUN）
//   推動 → PUSH
//   移動 → RUN
//   靜止 → STAND
// ─────────────────────────────────────────────────────────────────────────────
void CharacterPhysicsSystem::UpdateAnimState(int idx,
                                              std::vector<PhysicsAgent>& agents,
                                              bool isPushing,
                                              bool beingPushed) const {
    auto& self = agents[idx];
    if (self.actor == nullptr) return;

    const bool         justLanded = self.state.grounded && !self.state.prevGrounded;
    const CatAnimState cur        = self.actor->GetCatAnimState();

    CatAnimState next;

    if (!self.state.grounded) {
        // 空中：依速度方向
        next = (self.state.velocityY > 0.0f)
                   ? CatAnimState::JUMP_RISE
                   : CatAnimState::JUMP_FALL;
    }
    else if (justLanded) {
        next = CatAnimState::LAND;
    }
    else if (cur == CatAnimState::LAND && !self.actor->IfAnimationEnds()) {
        next = CatAnimState::LAND; // LAND 播放中不打斷
    }
    else if (isPushing) {
        // 修正問題 2：只有主動推動者（isPushing）才切換成 PUSH 動畫；
        // 被推的一方（beingPushed）保持原本動畫，不強制改成 PUSH。
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