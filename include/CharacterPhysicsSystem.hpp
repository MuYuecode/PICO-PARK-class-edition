//
// Created by cody2 on 2026/3/19.
//
#ifndef CHARACTER_PHYSICS_SYSTEM_HPP
#define CHARACTER_PHYSICS_SYSTEM_HPP

#include <memory>
#include <vector>

#include "PlayerCat.hpp"
#include "Character.hpp"

// PhysicsState：每隻角色的物理狀態(純資料，由 Scene 持有)
struct PhysicsState {
    float velocityY     = 0.0f;
    float lastDeltaX    = 0.0f;
    int   supportIndex  = -1;   // 站在哪隻角色上：-1 = 無
    int   moveDir       = 0;    // -1 / 0 / +1
    bool  grounded      = true;
    bool  prevGrounded  = true; // 前一幀是否接地(落地偵測用)
    bool  beingStoodOn  = false; // 有其他角色踩在自己頭上
};

// PhysicsAgent：actor + 物理狀態打包成一組(Scene 持有 vector<PhysicsAgent>)
struct PhysicsAgent {
    std::shared_ptr<PlayerCat> actor;
    PhysicsState               state;
};

// CharacterPhysicsSystem：純算法，無成員狀態
// 使用方式(Scene::Update 中)：
//   m_Physics.Update(m_Agents, m_Ctx.Floor);
// 動畫狀態由 System 內部直接呼叫 agent.actor->SetCatAnimState()，
// 場景完全不需要自行判斷動畫
class CharacterPhysicsSystem {
public:
    // 物理常數 public，供 Scene 需要時讀取
    static constexpr float kGroundMoveSpeed  = 5.0f;
    static constexpr float kRunOnPlayerSpeed = 6.2f;
    static constexpr float kJumpForce        = 11.0f;
    static constexpr float kGravity          = 0.75f;

    // 畫面邊界(1280×720，以原點為中心)
    static constexpr float kScreenHalfW = 640.0f;   // ← 原本推算錯誤的 400.0f
    static constexpr float kScreenHalfH = 360.0f;   // ← 原本推算錯誤的 300.0f

    static void ApplyJump(PhysicsState& state) {
        state.velocityY    = kJumpForce;
        state.grounded     = false;
        state.supportIndex = -1;
    }

    // 主要更新介面
    // 每 frame 呼叫一次；內部依序處理：
    //   1. 攜帶(被踩角色跟隨支撐者移動)
    //   2. 水平移動 + 碰撞解析
    //   3. 垂直物理(重力 + 地板 + 踩頭)
    //   4. 動畫狀態切換
    static void Update(std::vector<PhysicsAgent>& agents,
                const std::shared_ptr<Character>& floor) ;

    // 單一角色的水平位移(不含物理，只移動 + 碰撞限制)
    // 供需要「只移動一隻」的場景使用
    static float ResolveHorizontal(int idx,
                                          float targetX,
                                          const std::vector<PhysicsAgent>& agents) ;

private:
    // 角色尺寸(假設所有角色尺寸相同，且不會變動)
    static float HalfWidth()  ;
    static float HalfHeight() ;

    // 動態站立偏移：floor 半高 + 角色半高
    static float StandOffset(const std::shared_ptr<Character>& floor) ;

    // 物理解析(private，由 Update 統一呼叫)
    static void  ResolveVertical(int idx,
                          std::vector<PhysicsAgent>& agents,
                          const std::shared_ptr<Character>& floor);

    static bool  IsBeingPushed(int idx,
                                      const std::vector<PhysicsAgent>& agents,
                                      const std::shared_ptr<Character>& floor) ;

    // 動畫決策(private，由 Update 統一呼叫)
    static void  UpdateAnimState(int idx,
                          const std::vector<PhysicsAgent>& agents,
                          bool isPushing) ;
};

#endif // CHARACTER_PHYSICS_SYSTEM_HPP