#ifndef PICOPART_LOCALPLAYGAMESCENE_HPP
#define PICOPART_LOCALPLAYGAMESCENE_HPP

#include <vector>

#include "Scene.hpp"
#include "PlayerCat.hpp"
#include "KeyboardConfigScene.hpp"
#include "CharacterPhysicsSystem.hpp"

class LocalPlayScene;

class LocalPlayGameScene : public Scene {
public:
    LocalPlayGameScene(GameContext& ctx,
                       LocalPlayScene* localPlayScene,
                       KeyboardConfigScene* kbConfigScene);
    ~LocalPlayGameScene() override = default;

    void   OnEnter() override;
    void   OnExit()  override;
    Scene* Update()  override;

private:
    // ── 場景私有：每位玩家多持有一份按鍵設定 ────────────────────────────
    struct PlayerBinding {
        PhysicsAgent    agent;
        PlayerKeyConfig key;
    };

    LocalPlayScene*      m_LocalPlayScene = nullptr;
    KeyboardConfigScene* m_KbConfigScene  = nullptr;

    CharacterPhysicsSystem   m_Physics;
    std::vector<PlayerBinding> m_Players;

    void SpawnPlayers(int count);
    void ApplyInitialFormation();
    void UpdateCooperativePower();

    // 把 PlayerBinding 攤平成 PhysicsAgent 清單（供 System 使用）
    std::vector<PhysicsAgent*> AgentPtrs();
};

#endif // PICOPART_LOCALPLAYGAMESCENE_HPP