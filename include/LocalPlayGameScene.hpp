#ifndef PICOPART_LOCALPLAYGAMESCENE_HPP
#define PICOPART_LOCALPLAYGAMESCENE_HPP

#include <vector>
#include <memory>

#include "Scene.hpp"
#include "PlayerCat.hpp"
#include "PhysicsWorld.hpp"
#include "KeyboardConfigScene.hpp"

class LocalPlayScene;
class LevelSelectScene;

class LocalPlayGameScene : public Scene {
public:
    LocalPlayGameScene(GameContext& ctx,
                       LocalPlayScene*      localPlayScene,
                       KeyboardConfigScene* kbConfigScene);
    ~LocalPlayGameScene() override = default;

    void   OnEnter() override;
    void   OnExit()  override;
    Scene* Update()  override;

    void SetLevelSelectScene(LevelSelectScene* s) { m_LevelSelectScene = s; }

private:
    // One entry per active player.
    struct PlayerBinding {
        std::shared_ptr<PlayerCat> cat;
        PlayerKeyConfig            key;
        bool                       entered = false;
    };

    // Registers floor, walls, and ceiling as StaticBodies in m_World.
    void SetupStaticBoundaries();

    // Creates PlayerCat instances, assigns key configs, registers into m_World.
    void SpawnPlayers(int count);

    // Places spawned cats in the initial side formation.
    void ApplyInitialFormation();

    // Reads touch/direction state and writes m_Ctx.CooperativePushPower.
    // Pure statistics; does not move any body.
    void UpdateCooperativePower() const;

    // Updates the "N/total" label above the door.
    void UpdateDoorCountText() const;

    std::shared_ptr<GameText> m_DoorCountText;
    int                       m_EnteredCount = 0;

    LevelSelectScene*    m_LevelSelectScene = nullptr;
    LocalPlayScene*      m_LocalPlayScene   = nullptr;
    KeyboardConfigScene* m_KbConfigScene    = nullptr;

    PhysicsWorld               m_World;
    std::vector<PlayerBinding> m_Players;
};

#endif // PICOPART_LOCALPLAYGAMESCENE_HPP