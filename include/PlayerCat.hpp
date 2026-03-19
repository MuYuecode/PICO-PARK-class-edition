//
// Created by cody2 on 2026/3/12.
//

#ifndef PLAYER_CAT_HPP
#define PLAYER_CAT_HPP

#include "AnimatedCharacter.hpp"
#include "Character.hpp"
#include "IPhysicsBody.hpp"
#include "Util/Animation.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// 角色動畫狀態
// ─────────────────────────────────────────────────────────────────────────────
enum class CatAnimState {
    STAND,      // 站立（循環）
    RUN,        // 跑步（循環）
    JUMP_RISE,  // 起跳到最高點 — jump_1.png（循環）
    JUMP_FALL,  // 最高點到落地前 — jump_2.png（循環）
    LAND,       // 落地（播一次）
    PUSH,       // 推動（循環）
};

// ─────────────────────────────────────────────────────────────────────────────
// 各動畫所需的圖片路徑集合
// ─────────────────────────────────────────────────────────────────────────────
struct CatAnimPaths {
    std::vector<std::string> stand;      // e.g. { blue_cat_stand_1.png, stand_2.png }
    std::vector<std::string> run;        // e.g. { run_1.png, run_2.png }
    std::vector<std::string> jump_rise;  // e.g. { jump_1.png }  ← 起跳到最高點
    std::vector<std::string> jump_fall;  // e.g. { jump_2.png }  ← 最高點到落地前
    std::vector<std::string> land;       // e.g. { land_1.png }
    std::vector<std::string> push;       // e.g. { push_1.png, push_2.png }
};

// ─────────────────────────────────────────────────────────────────────────────
// PlayerCat
// ─────────────────────────────────────────────────────────────────────────────
class PlayerCat : public AnimatedCharacter, public IPhysicsBody {
public:
    PlayerCat(const CatAnimPaths& animPaths,
              Util::Keycode leftKey,
              Util::Keycode rightKey,
              Util::Keycode jumpKey);

    // 舊版簡易物理 Update（PlayerCat 自行處理重力與地板碰撞）
    // 修正問題 1：standOffset 改為 floor 半高 + 角色半高（動態）
    void Update(std::shared_ptr<Character>& floor);

    // ── 動畫狀態管理 ─────────────────────────────────────────────────────
    void SetCatAnimState(CatAnimState newState);
    [[nodiscard]] CatAnimState GetCatAnimState() const { return m_CurrentAnimState; }

    // ── 輸入控制 ─────────────────────────────────────────────────────────
    void SetInputEnabled(bool enabled)         { m_InputEnabled = enabled; }
    [[nodiscard]] bool GetInputEnabled() const { return m_InputEnabled;    }

    // ── IPhysicsBody 實作 / 位置 / 縮放 ──────────────────────────────────
    void SetPosition(const glm::vec2& pos) override { m_Transform.translation = pos; }
    void SetScale(const glm::vec2& scale)   { m_Transform.scale = scale; }
    void SetVelocity(const glm::vec2& vel) override { m_Velocity = vel; }

    [[nodiscard]] BodyType GetBodyType() const override { return BodyType::CHARACTER; }
    [[nodiscard]] glm::vec2 GetPosition() const override { return m_Transform.translation; }
    [[nodiscard]] glm::vec2 GetHalfSize() const override { return glm::abs(GetScaledSize()) * 0.5f; }
    [[nodiscard]] glm::vec2 GetVelocity() const override { return m_Velocity; }
    [[nodiscard]] bool IsSolid() const override { return true; }
    [[nodiscard]] bool IsKinematic() const override { return false; }
    [[nodiscard]] bool UseGravity() const override { return true; }
    void OnCollision(const CollisionInfo& /*info*/) override {}

    // ── 按鍵讀取（供 TitleScene / LocalPlayGameScene 使用）────────────────
    [[nodiscard]] Util::Keycode GetLeftKey()  const { return m_LeftKey;  }
    [[nodiscard]] Util::Keycode GetRightKey() const { return m_RightKey; }
    [[nodiscard]] Util::Keycode GetJumpKey()  const { return m_JumpKey;  }

private:
    void BuildClips(const CatAnimPaths& paths);

    // ── 六種動畫 clip ─────────────────────────────────────────────────────
    std::shared_ptr<Util::Animation> m_StandAnim;
    std::shared_ptr<Util::Animation> m_RunAnim;
    std::shared_ptr<Util::Animation> m_JumpRiseAnim;  // jump_1
    std::shared_ptr<Util::Animation> m_JumpFallAnim;  // jump_2
    std::shared_ptr<Util::Animation> m_LandAnim;
    std::shared_ptr<Util::Animation> m_PushAnim;

    CatAnimState m_CurrentAnimState = CatAnimState::STAND;

    // ── 按鍵 ─────────────────────────────────────────────────────────────
    bool          m_InputEnabled = true;
    Util::Keycode m_LeftKey;
    Util::Keycode m_RightKey;
    Util::Keycode m_JumpKey;

    // ── 物理狀態（PlayerCat::Update 使用）────────────────────────────────
    float     m_MoveSpeed   = 5.0f;
    glm::vec2 m_Velocity    = {0.0f, 0.0f};
    float     m_JumpForce   = 11.0f;
    float     m_Gravity     = 0.75f;
    bool      m_IsGrounded  = true;
    bool      m_WasGrounded = true;  // 前幀接地狀態，用於落地偵測
};

#endif // PLAYER_CAT_HPP