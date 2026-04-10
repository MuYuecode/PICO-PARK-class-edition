#include "physics/PhysicsWorld.hpp"
#include <algorithm>

#include "physics/CollisionSolver.hpp"
#include "physics/PhysicsUpdateScheduler.hpp"
#include "physics/PushForceResolver.hpp"

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
    m_OwnedStatics.clear();
    m_Ropes.clear();
}

StaticBody* PhysicsWorld::AddStaticBoundary(glm::vec2 center, glm::vec2 halfSize,
                                              BodyType type) {
    auto sb = std::make_shared<StaticBody>(center, halfSize, type);
    StaticBody* raw = sb.get();
    m_OwnedStatics.push_back(sb);
    Register(sb);
    return raw;
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
        if (r.bodyA == body || r.bodyB == body) result.push_back(&r);
    }
    return result;
}

void PhysicsWorld::FreezeAll() const {
    for (auto& wp : m_Bodies) {
        if (auto sp = wp.lock()) sp->Freeze();
    }
}

void PhysicsWorld::UnfreezeAll() const {
    for (auto& wp : m_Bodies) {
        if (auto sp = wp.lock()) sp->Unfreeze();
    }
}

void PhysicsWorld::Update() {
    ++m_FrameCount;
    if (m_FrameCount % kPurgeInterval == 0) PurgeExpired();

    PhysicsUpdateScheduler::StepPhysicsUpdate(m_Bodies);
    StepResolveAndApply();

    for (auto& wp : m_Bodies) {
        auto sp = wp.lock();
        if (sp && sp->IsActive() && !sp->IsFrozen()) {
            sp->PostUpdate();
        }
    }
}

void PhysicsWorld::StepResolveAndApply() const {
    // Snapshot live bodies.
    BodyInfos infos;
    infos.reserve(m_Bodies.size());
    for (auto& wp : m_Bodies) {
        auto sp = wp.lock();
        if (!sp || !sp->IsActive()) continue;
        BodyInfo bi;
        bi.body    = sp;
        bi.desired = sp->IsFrozen() ? glm::vec2{0.f, 0.f} : sp->GetDesiredDelta();
        bi.resolved    = bi.desired;
        bi.resolvedFlag = false;
        bi.supportIdx   = -1;
        infos.push_back(std::move(bi));
    }
    CollisionSolver::ResolveAndApply(infos, kRidingTolerance);
}

int PhysicsWorld::CountCharactersPushing(const IPhysicsBody* target, int dir) const {
    return PushForceResolver::CountCharactersPushing(target, dir, m_Bodies);
}

std::vector<IPhysicsBody*> PhysicsWorld::GetBodiesOfType(BodyType type) const {
    std::vector<IPhysicsBody*> result;
    for (auto& wp : m_Bodies) {
        auto sp = wp.lock();
        if (sp && sp->IsActive() && sp->GetPhysicsTraits().type == type) {
            result.push_back(sp.get());
        }
    }
    return result;
}


void PhysicsWorld::PurgeExpired() {
    m_Bodies.erase(
        std::remove_if(m_Bodies.begin(), m_Bodies.end(),
            [](const std::weak_ptr<IPhysicsBody>& wp) { return wp.expired(); }),
        m_Bodies.end());
}
