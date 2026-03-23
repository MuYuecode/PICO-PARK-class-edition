//
// Created by cody2 on 2026/3/19.
//
#ifndef PHYSICS_WORLD_HPP
#define PHYSICS_WORLD_HPP

#include <memory>
#include <vector>

#include "IPhysicsBody.hpp"

// 繩索約束（成對記錄，由 PhysicsWorld 統一解析）
struct RopeConstraint {
    IPhysicsBody* bodyA    = nullptr;
    IPhysicsBody* bodyB    = nullptr;
    float         maxLen   = 200.0f;  // 繩索最大長度（像素）
    float         friction = 0.5f;    // 0=無阻力，1=完全鎖死

    // TODO 實作時補充：
    //   - 計算目前距離 d = distance(A.pos, B.pos)
    //   - 若 d <= maxLen，不施力
    //   - 若 d >  maxLen：
    //       dir = normalize(A.pos - B.pos)   // 從 B 指向 A
    //       力大小 = (d - maxLen) * stiffness
    //       A.velocity -= dir * 力 * (1 - friction)
    //       B.velocity += dir * 力 * (1 - friction)
};

// PhysicsWorld：物理物件的協調者
//
// 職責：
//   1. 持有場景中所有 IPhysicsBody 的弱引用（不擁有生命週期）
//   2. 每 frame 按順序執行：
//       a. 呼叫各 body 的 PhysicsUpdate()（自驅動邏輯）
//       b. 解析繩索約束
//       c. 廣播 AABB 碰撞事件（呼叫 OnCollision）
//   3. 提供查詢工具，供各 System 使用（計算「有幾人推箱子」等）
//
// 不負責的事：
//   - 玩家輸入（Scene 的責任）
//   - 動畫切換（各物件自行處理）
//   - 渲染（GameObject / Renderer 的責任）
class PhysicsWorld {
public:
    PhysicsWorld()  = default;
    ~PhysicsWorld() = default;

    PhysicsWorld(const PhysicsWorld&)            = delete;
    PhysicsWorld& operator=(const PhysicsWorld&) = delete;

    // 登記 / 移除
    void Register(const std::shared_ptr<IPhysicsBody>& body);
    void Unregister(const IPhysicsBody* body);
    void Clear();

    // 繩索
    void AddRope(IPhysicsBody* a, IPhysicsBody* b,
                 float maxLen = 200.0f, float friction = 0.5f);
    void RemoveRope(IPhysicsBody* a, IPhysicsBody* b);

    // 主要更新(每 frame 由 Scene 呼叫一次)
    void Update();

    // 查詢工具(供各 System / 物件的 PhysicsUpdate 使用)

    // 計算「有幾個 CHARACTER body 正在往 dir 方向推 target」
    // dir: +1 = 向右推（body 在 target 左側往右移），-1 = 向左推
    // 用於：PushableBox 判斷是否達到 n 人推動條件
    // int CountCharactersPushing(const IPhysicsBody* target, int dir) const;

    // 計算「有幾個 body 站在 target 頂部」
    // 用於：ConditionalPlatform 判斷是否達到 n 人站上條件
    // int CountBodiesOnTop(const IPhysicsBody* target) const;

    // 回傳所有與 target 的 AABB 有重疊的 body
    // 用於：Bullet 碰撞偵測、敵人傷害範圍
    // std::vector<IPhysicsBody*> QueryOverlapping(const IPhysicsBody* target) const;

    // 回傳所有指定類型的 body
    [[nodiscard]] std::vector<IPhysicsBody*> GetBodiesOfType(BodyType type) const;

    // 取得與指定 body 連結的所有繩索
    std::vector<RopeConstraint*> GetRopesOf(const IPhysicsBody* body);

private:
    // 工具
    static bool AabbOverlaps(const IPhysicsBody* a, const IPhysicsBody* b) ;
    // bool IsOnTop(const IPhysicsBody* rider, const IPhysicsBody* platform) const;

    // 清理已失效的弱引用(每 N frame 執行一次)
    void PurgeExpired();

    // 內部更新步驟
    void StepPhysicsUpdate() const ;   // 呼叫所有 body 的 PhysicsUpdate()
    // void StepRopes();           // 解析繩索約束
    // void StepCollisions();      // 廣播 AABB 碰撞事件

    // 資料
    std::vector<std::weak_ptr<IPhysicsBody>> m_Bodies;
    std::vector<RopeConstraint>              m_Ropes;

    int m_FrameCount = 0;
    static constexpr int kPurgeInterval = 60;  // 每 60 frame 清理一次失效引用
};

#endif // PHYSICS_WORLD_HPP