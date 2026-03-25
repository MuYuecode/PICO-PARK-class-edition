//
// Created by cody2 on 2026/3/19.
//
#ifndef PHYSICS_WORLD_HPP
#define PHYSICS_WORLD_HPP

#include <memory>
#include <vector>

#include "IPhysicsBody.hpp"

struct RopeConstraint {
    IPhysicsBody* bodyA    = nullptr;
    IPhysicsBody* bodyB    = nullptr;
    float         maxLen   = 200.0f;
    float         friction = 0.5f;

    // TODO 實作時補充：
    //   - 計算目前距離 d = distance(A.pos, B.pos)
    //   - 若 d <= maxLen，不施力
    //   - 若 d >  maxLen：
    //       dir = normalize(A.pos - B.pos)   // 從 B 指向 A
    //       力大小 = (d - maxLen) * stiffness
    //       A.velocity -= dir * 力 * (1 - friction)
    //       B.velocity += dir * 力 * (1 - friction)
};

class PhysicsWorld {
public:
    PhysicsWorld()  = default;
    ~PhysicsWorld() = default;

    PhysicsWorld(const PhysicsWorld&)            = delete;
    PhysicsWorld& operator=(const PhysicsWorld&) = delete;

    void Register(const std::shared_ptr<IPhysicsBody>& body);
    void Unregister(const IPhysicsBody* body);
    void Clear();

    void AddRope(IPhysicsBody* a, IPhysicsBody* b,
                 float maxLen = 200.0f, float friction = 0.5f);
    void RemoveRope(IPhysicsBody* a, IPhysicsBody* b);

    void Update();

    // 計算「有幾個 CHARACTER body 正在往 dir 方向推 target」
    // dir: +1 = 向右推(body 在 target 左側往右移)，-1 = 向左推
    // 用於：PushableBox 判斷是否達到 n 人推動條件
    // int CountCharactersPushing(const IPhysicsBody* target, int dir) const;

    // 計算「有幾個 body 站在 target 頂部」
    // 用於：ConditionalPlatform 判斷是否達到 n 人站上條件
    // int CountBodiesOnTop(const IPhysicsBody* target) const;

    // 回傳所有與 target 的 AABB 有重疊的 body
    // 用於：Bullet 碰撞偵測、敵人傷害範圍
    // std::vector<IPhysicsBody*> QueryOverlapping(const IPhysicsBody* target) const;

    [[nodiscard]] std::vector<IPhysicsBody*> GetBodiesOfType(BodyType type) const;

    std::vector<RopeConstraint*> GetRopesOf(const IPhysicsBody* body);

private:
    static bool AabbOverlaps(const IPhysicsBody* a, const IPhysicsBody* b) ;
    // bool IsOnTop(const IPhysicsBody* rider, const IPhysicsBody* platform) const;

    void PurgeExpired();

    void StepPhysicsUpdate() const ;
    // void StepRopes();
    // void StepCollisions();

    std::vector<std::weak_ptr<IPhysicsBody>> m_Bodies;
    std::vector<RopeConstraint>              m_Ropes;

    int m_FrameCount = 0;
    static constexpr int kPurgeInterval = 60;
};

#endif // PHYSICS_WORLD_HPP