//
// Created by cody2 on 2026/3/19.
//
#ifndef PHYSICS_BODY_HPP
#define PHYSICS_BODY_HPP

#include "pch.hpp"

class IPhysicsBody;

// ─────────────────────────────────────────────────────────────────────────────
// 物理物件類型
// ─────────────────────────────────────────────────────────────────────────────
enum class BodyType {
    CHARACTER,              // 玩家角色（PlayerCat）
    PUSHABLE_BOX,           // 有條件可被推動的箱子
    PATROL_ENEMY,           // 固定巡邏路徑的敵人
    MOVING_PLATFORM,        // 固定軌跡移動的平台
    CONDITIONAL_PLATFORM,   // 有條件才移動的平台
    ROPE_ENDPOINT,          // 繩索端點（成對出現）
    BULLET,                 // 子彈
};

// ─────────────────────────────────────────────────────────────────────────────
// 碰撞資訊（傳入 OnCollision 的完整情境）
// ─────────────────────────────────────────────────────────────────────────────
struct CollisionInfo {
    IPhysicsBody* other    = nullptr;  // 碰到誰
    glm::vec2     normal   = {0, 0};   // 碰撞法向量（從 other 指向 self）
    float         depth    = 0.0f;     // 穿透深度
};

// ─────────────────────────────────────────────────────────────────────────────
// IPhysicsBody：所有可參與物理互動的物件必須實作此介面
//
// 設計原則：
//   - 介面只定義「PhysicsWorld 需要知道的最小資訊」
//   - 具體的動畫、渲染、AI 邏輯留在子類別
//   - PhysicsUpdate() 是物件的「自驅動」邏輯（敵人移動、平台軌跡等）
//   - OnCollision()   是物件對「外部碰撞事件」的反應
// ─────────────────────────────────────────────────────────────────────────────
class IPhysicsBody {
public:
    virtual ~IPhysicsBody() = default;

    // ── 身分識別 ─────────────────────────────────────────────────────────
    virtual BodyType GetBodyType() const = 0;

    // ── 位置與尺寸（AABB 碰撞基礎）───────────────────────────────────────
    virtual glm::vec2 GetPosition() const = 0;
    virtual void      SetPosition(const glm::vec2& pos) = 0;

    // 回傳碰撞箱半寬、半高（不一定等於圖片大小）
    virtual glm::vec2 GetHalfSize() const = 0;

    // ── 速度 ─────────────────────────────────────────────────────────────
    virtual glm::vec2 GetVelocity() const = 0;
    virtual void      SetVelocity(const glm::vec2& vel) = 0;

    // ── 物理屬性 ─────────────────────────────────────────────────────────

    // 是否為實體（true = 阻擋其他物體穿越，false = 可穿透，如子彈觸發區）
    virtual bool IsSolid() const = 0;

    // 是否由外部控制位置（true = PhysicsWorld 不對其施加重力/推力）
    // 平台、固定敵人路徑皆為 true；玩家、箱子為 false
    virtual bool IsKinematic() const = 0;

    // 是否受重力影響（角色/箱子=true，平台/子彈=false）
    virtual bool UseGravity() const = 0;

    // ── 生命週期 ─────────────────────────────────────────────────────────

    // 每 frame 由 PhysicsWorld 呼叫，物件可在此更新自身狀態
    // （敵人 AI、平台軌跡計算、子彈移動等）
    // 若需查詢其他物件，應透過 PhysicsWorld 的查詢工具（CountCharactersPushing 等），而非直接存取其他 body 的位置
    virtual void PhysicsUpdate() {}

    // 碰撞解析完成後，PhysicsWorld 通知此物件與誰發生了碰撞
    // 子類別在這裡實作「碰到玩家扣血」「碰到子彈改變狀態」等邏輯
    virtual void OnCollision(const CollisionInfo& /*info*/) {}

    // ── 啟用 / 停用 ──────────────────────────────────────────────────────
    virtual bool IsActive() const { return m_Active; }
    virtual void SetActive(bool active) { m_Active = active; }

protected:
    bool m_Active = true;
};

#endif // PHYSICS_BODY_HPP
