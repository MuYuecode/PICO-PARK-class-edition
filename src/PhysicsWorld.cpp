//
// Created by cody2 on 2026/3/19.
//

#include "PhysicsWorld.hpp"
#include <algorithm>
#include <cmath>

void PhysicsWorld::Register(const std::shared_ptr<IPhysicsBody>& body) {
    if (body == nullptr) return;
    m_Bodies.emplace_back(body);
}

void PhysicsWorld::Unregister(const IPhysicsBody* body) {
    m_Bodies.erase(
        std::remove_if(m_Bodies.begin(), m_Bodies.end(),
            [body](const std::weak_ptr<IPhysicsBody>& wp) {
                auto sp = wp.lock();
                return !sp || sp.get() == body;
            }),
        m_Bodies.end());
}

void PhysicsWorld::Clear() {
    m_Bodies.clear();
    m_Ropes.clear();
}

void PhysicsWorld::AddRope(IPhysicsBody* a, IPhysicsBody* b,
                            float maxLen, float friction) {
    m_Ropes.push_back({a, b, maxLen, friction});
}

void PhysicsWorld::RemoveRope(IPhysicsBody* a, IPhysicsBody* b) {
    m_Ropes.erase(
        std::remove_if(m_Ropes.begin(), m_Ropes.end(),
            [a, b](const RopeConstraint& r) {
                return (r.bodyA == a && r.bodyB == b) ||
                       (r.bodyA == b && r.bodyB == a);
            }),
        m_Ropes.end());
}

std::vector<RopeConstraint*> PhysicsWorld::GetRopesOf(const IPhysicsBody* body) {
    std::vector<RopeConstraint*> result;
    for (auto& r : m_Ropes) {
        if (r.bodyA == body || r.bodyB == body) {
            result.push_back(&r);
        }
    }
    return result;
}

void PhysicsWorld::Update() {
    ++m_FrameCount;

    if (m_FrameCount % kPurgeInterval == 0) {
        PurgeExpired();
    }

    StepPhysicsUpdate();
    // StepRopes();
    // StepCollisions();
}

// StepPhysicsUpdate：呼叫所有 active body 的自驅動邏輯
void PhysicsWorld::StepPhysicsUpdate() const {
    for (auto& wp : m_Bodies) {
        auto sp = wp.lock();
        if (sp && sp->IsActive()) {
            sp->PhysicsUpdate();
        }
    }
}

// StepRopes：解析繩索約束，對兩端施加力
// void PhysicsWorld::StepRopes() {
//     // TODO：實作繩索力解析
//     //
//     // for (auto& rope : m_Ropes) {
//     //     if (!rope.bodyA || !rope.bodyB) continue;
//     //
//     //     glm::vec2 posA = rope.bodyA->GetPosition();
//     //     glm::vec2 posB = rope.bodyB->GetPosition();
//     //     glm::vec2 delta = posA - posB;
//     //     float dist = glm::length(delta);
//     //
//     //     if (dist <= rope.maxLen) continue;  // 繩索未繃緊，不施力
//     //
//     //     // 方向：從 B 指向 A
//     //     glm::vec2 dir = delta / dist;
//     //
//     //     // 力大小（可調整 stiffness）
//     //     constexpr float stiffness = 0.8f;
//     //     float forceMag = (dist - rope.maxLen) * stiffness;
//     //
//     //     // 施加到速度（乘上 1-friction 模擬阻力）
//     //     float factor = 1.0f - rope.friction;
//     //     glm::vec2 velA = rope.bodyA->GetVelocity();
//     //     glm::vec2 velB = rope.bodyB->GetVelocity();
//     //
//     //     rope.bodyA->SetVelocity(velA - dir * forceMag * factor);
//     //     rope.bodyB->SetVelocity(velB + dir * forceMag * factor);
//     // }
// }

// StepCollisions：廣播 AABB 碰撞事件
// void PhysicsWorld::StepCollisions() {
//     // TODO：實作碰撞廣播
//     //
//     // 使用暴力 O(n²) 先行，之後可改為空間分割（Sweep-and-Prune 等）
//     //
//     // std::vector<std::shared_ptr<IPhysicsBody>> live;
//     // for (auto& wp : m_Bodies) {
//     //     if (auto sp = wp.lock(); sp && sp->IsActive()) live.push_back(sp);
//     // }
//     //
//     // for (int i = 0; i < (int)live.size(); ++i) {
//     //     for (int j = i + 1; j < (int)live.size(); ++j) {
//     //         if (!AabbOverlaps(live[i].get(), live[j].get())) continue;
//     //
//     //         // 計算法向量與穿透深度
//     //         glm::vec2 delta   = live[j]->GetPosition() - live[i]->GetPosition();
//     //         glm::vec2 overlap = live[i]->GetHalfSize() + live[j]->GetHalfSize()
//     //                             - glm::abs(delta);
//     //         glm::vec2 normal  = (overlap.x < overlap.y)
//     //                                 ? glm::vec2{(delta.x < 0 ? -1.f : 1.f), 0}
//     //                                 : glm::vec2{0, (delta.y < 0 ? -1.f : 1.f)};
//     //         float depth = std::min(overlap.x, overlap.y);
//     //
//     //         live[i]->OnCollision({live[j].get(),  normal, depth});
//     //         live[j]->OnCollision({live[i].get(), -normal, depth});
//     //     }
//     // }
// }

// 查詢：計算幾個 CHARACTER 正在往 dir 方向推 target
// int PhysicsWorld::CountCharactersPushing(const IPhysicsBody* target, int dir) const {
//     // TODO：實作
//     //
//     // int count = 0;
//     // glm::vec2 tPos    = target->GetPosition();
//     // glm::vec2 tHalf   = target->GetHalfSize();
//     //
//     // for (auto& wp : m_Bodies) {
//     //     auto sp = wp.lock();
//     //     if (!sp || !sp->IsActive()) continue;
//     //     if (sp.get() == target) continue;
//     //     if (sp->GetBodyType() != BodyType::CHARACTER) continue;
//     //
//     //     glm::vec2 cPos  = sp->GetPosition();
//     //     glm::vec2 cHalf = sp->GetHalfSize();
//     //
//     //     // 垂直上需在同層（避免把頭上的角色也算進去）
//     //     float vertGap = std::abs(cPos.y - tPos.y);
//     //     if (vertGap > (tHalf.y + cHalf.y) * 0.65f) continue;
//     //
//     //     // 側面緊貼判斷
//     //     float minDistX = tHalf.x + cHalf.x;
//     //     float dx       = cPos.x - tPos.x;
//     //     if (std::abs(dx) > minDistX * 1.05f) continue;
//     //
//     //     // dir > 0：推者在左邊往右推
//     //     glm::vec2 vel = sp->GetVelocity();
//     //     if (dir > 0 && dx < 0 && vel.x > 0) ++count;
//     //     if (dir < 0 && dx > 0 && vel.x < 0) ++count;
//     // }
//     // return count;
//
//     (void)target;
//     (void)dir;
//     return 0;
// }

// 查詢：計算幾個 body 站在 target 頂部
// int PhysicsWorld::CountBodiesOnTop(const IPhysicsBody* target) const {
//     // TODO：實作（呼叫 IsOnTop）
//     //
//     // int count = 0;
//     // for (auto& wp : m_Bodies) {
//     //     auto sp = wp.lock();
//     //     if (!sp || !sp->IsActive()) continue;
//     //     if (sp.get() == target) continue;
//     //     if (IsOnTop(sp.get(), target)) ++count;
//     // }
//     // return count;
//
//     (void)target;
//     return 0;
// }

// 查詢：AABB 重疊的所有 body
// std::vector<IPhysicsBody*> PhysicsWorld::QueryOverlapping(const IPhysicsBody* target) const {
//     std::vector<IPhysicsBody*> result;
//
//     // TODO：實作
//     //
//     // for (auto& wp : m_Bodies) {
//     //     auto sp = wp.lock();
//     //     if (!sp || !sp->IsActive()) continue;
//     //     if (sp.get() == target) continue;
//     //     if (AabbOverlaps(target, sp.get())) result.push_back(sp.get());
//     // }
//
//     (void)target;
//     return result;
// }


// 查詢：所有指定類型的 body
std::vector<IPhysicsBody*> PhysicsWorld::GetBodiesOfType(BodyType type) const {
    std::vector<IPhysicsBody*> result;
    for (auto& wp : m_Bodies) {
        auto sp = wp.lock();
        if (sp && sp->IsActive() && sp->GetBodyType() == type) {
            result.push_back(sp.get());
        }
    }
    return result;
}

// 工具：AABB 重疊判斷
bool PhysicsWorld::AabbOverlaps(const IPhysicsBody* a, const IPhysicsBody* b) {
    glm::vec2 delta = a->GetPosition() - b->GetPosition();
    glm::vec2 sum   = a->GetHalfSize() + b->GetHalfSize();
    return std::abs(delta.x) < sum.x && std::abs(delta.y) < sum.y;
}

// 工具：rider 是否站在 platform 頂部
// bool PhysicsWorld::IsOnTop(const IPhysicsBody* rider, const IPhysicsBody* platform) const {
//     // TODO：實作
//     //
//     // glm::vec2 rPos   = rider->GetPosition();
//     // glm::vec2 pPos   = platform->GetPosition();
//     // glm::vec2 rHalf  = rider->GetHalfSize();
//     // glm::vec2 pHalf  = platform->GetHalfSize();
//     //
//     // float platformTop = pPos.y + pHalf.y;
//     // float riderBottom = rPos.y - rHalf.y;
//     //
//     // bool horizOk = std::abs(rPos.x - pPos.x) < (rHalf.x + pHalf.x) * 0.8f;
//     // bool vertOk  = std::abs(riderBottom - platformTop) < 6.0f;
//     // return horizOk && vertOk;
//
//     (void)rider;
//     (void)platform;
//     return false;
// }

// 工具：清理失效的弱引用
void PhysicsWorld::PurgeExpired() {
    m_Bodies.erase(
        std::remove_if(m_Bodies.begin(), m_Bodies.end(),
            [](const std::weak_ptr<IPhysicsBody>& wp) { return wp.expired(); }),
        m_Bodies.end());
}