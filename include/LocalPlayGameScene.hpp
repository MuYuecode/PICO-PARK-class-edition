#ifndef PICOPART_LOCALPLAYGAMESCENE_HPP
#define PICOPART_LOCALPLAYGAMESCENE_HPP

#include <vector>

#include "Scene.hpp"
#include "KeyboardConfigScene.hpp"
#include "CharacterPhysicsSystem.hpp"

class LocalPlayScene;
class LevelSelectScene;

class LocalPlayGameScene : public Scene {
public:
    LocalPlayGameScene(GameContext& ctx,
                       LocalPlayScene* localPlayScene,
                       KeyboardConfigScene* kbConfigScene);
    ~LocalPlayGameScene() override = default;

    void   OnEnter() override;
    void   OnExit()  override;
    Scene* Update()  override;

    void SetLevelSelectScene(LevelSelectScene* s) { m_LevelSelectScene = s; }

private:
    // 場景私有：每位玩家持有一份按鍵設定
    struct PlayerBinding {
        PhysicsAgent    agent;
        PlayerKeyConfig key;
        bool            entered = false;   // 是否已進入門
    };

    std::shared_ptr<GameText> m_DoorCountText;
    int                       m_EnteredCount = 0;
    void                      UpdateDoorCountText() const ;

    LevelSelectScene*    m_LevelSelectScene = nullptr;
    LocalPlayScene*      m_LocalPlayScene   = nullptr;
    KeyboardConfigScene* m_KbConfigScene    = nullptr;

    CharacterPhysicsSystem     m_Physics;
    std::vector<PlayerBinding> m_Players;

    void SpawnPlayers(int count);
    void ApplyInitialFormation();
    void UpdateCooperativePower() const ;
};

#endif // PICOPART_LOCALPLAYGAMESCENE_HPP