#ifndef PICOPART_LOCALPLAYGAMESCENE_HPP
#define PICOPART_LOCALPLAYGAMESCENE_HPP

#include <vector>
#include <memory>
#include "core/scene/Scene.hpp"
#include "gameplay/actors/PlayerCat.hpp"
#include "physics/world/PhysicsWorld.hpp"
#include "scenes/menu/KeyboardConfigScene.hpp"


class LocalPlayGameScene : public Scene {
public:
    explicit LocalPlayGameScene(SceneServices services);
    ~LocalPlayGameScene() override = default;

    void   OnEnter() override;
    void   OnExit()  override;
    void Update()  override;

private:
    // One entry per active player.
    struct PlayerBinding {
        std::shared_ptr<PlayerCat> cat;
        PlayerKeyConfig            key;
        bool                       entered = false;
    };

    void SetupStaticBoundaries();
    void SpawnPlayers(int count);
    void ApplyInitialFormation();
    void UpdateCooperativePower() const;
    void UpdateDoorCountText() const;

    std::shared_ptr<GameText> m_DoorCountText;
    int                       m_EnteredCount = 0;


    PhysicsWorld               m_World;
    std::vector<PlayerBinding> m_Players;
};

#endif // PICOPART_LOCALPLAYGAMESCENE_HPP
