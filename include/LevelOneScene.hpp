#ifndef LEVEL_ONE_SCENE_HPP
#define LEVEL_ONE_SCENE_HPP

#include <memory>
#include <vector>

#include "Scene.hpp"
#include "PhysicsWorld.hpp"
#include "Character.hpp"
#include "PlayerCat.hpp"
#include "PushableBox.hpp"
#include "GameText.hpp"
#include "PlayerKeyConfig.hpp"

class LevelOneScene : public Scene {
public:
    explicit LevelOneScene(GameContext& ctx);
    ~LevelOneScene() override = default;

    void OnEnter() override;
    void OnExit() override;
    SceneId Update() override;

private:
    struct PlayerBinding {
        std::shared_ptr<PlayerCat> cat;
        PlayerKeyConfig            key;
    };

    void SetupStaticBoundaries();
    void SpawnPlayers(int count);
    void ApplyInitialFormation();

    void HandlePlayerInput() const;
    void UpdateTimerText() const;

    void TryPickKey();
    void UpdateKeyFollow() const;
    void TryOpenDoorAndClear();

    static bool AabbOverlap(const glm::vec2& aPos, const glm::vec2& aHalf,
                            const glm::vec2& bPos, const glm::vec2& bHalf);

    static constexpr int   kLevelIndex = 0;
    static constexpr float kRoomLeftX  = -624.0f;
    static constexpr float kRoomRightX =  624.0f;
    static constexpr float kRoomTopY   =  337.0f;
    static constexpr float kRoomFloorY = -337.0f;

    static constexpr glm::vec2 kKeyHalf  = {20.0f, 26.0f};
    static constexpr glm::vec2 kDoorHalf = {31.0f, 34.0f};

    PhysicsWorld m_World;

    std::shared_ptr<Character>   m_FloorSprite;
    std::shared_ptr<Character>   m_LeftWallSprite;
    std::shared_ptr<Character>   m_RightWallSprite;
    std::shared_ptr<Character>   m_CeilingSprite;
    std::shared_ptr<Character>   m_KeySprite;
    std::shared_ptr<PushableBox> m_BoxA;
    std::shared_ptr<PushableBox> m_BoxB;

    std::shared_ptr<GameText> m_TimerText;

    std::vector<PlayerBinding> m_Players;

    glm::vec2 m_KeyInitialPos = {-400.0f, -220.0f};
    glm::vec2 m_DoorPos       = { 420.0f, kRoomFloorY + kDoorHalf.y };

    int   m_KeyCarrierIdx = -1;
    bool  m_DoorOpened    = false;
    float m_ElapsedSec    = 0.0f;


    // enter door
    void UpdateDoorEntryAndClear();
    std::vector<bool> m_PlayerEntered;
    int  m_EnteredCount = 0;
    bool m_ClearDone    = false;
};

#endif // LEVEL_ONE_SCENE_HPP
