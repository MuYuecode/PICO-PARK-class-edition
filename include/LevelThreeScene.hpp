#ifndef LEVEL_THREE_SCENE_HPP
#define LEVEL_THREE_SCENE_HPP

#include <memory>
#include <vector>

#include "Character.hpp"
#include "GameText.hpp"
#include "PhysicsWorld.hpp"
#include "PlayerCat.hpp"
#include "PlayerKeyConfig.hpp"
#include "Scene.hpp"

class LevelThreeScene : public Scene {
public:
    explicit LevelThreeScene(SceneServices services);
    ~LevelThreeScene() override = default;

    void OnEnter() override;
    void OnExit() override;
    void Update() override;
    void PauseGameplay() override;
    void ResumeGameplay() override;

private:
    class MovingLiftBody;
    class PatrolMobBody;
    class PipeMobBody;

    static bool AabbOverlap(const glm::vec2& aPos, const glm::vec2& aHalf,
                            const glm::vec2& bPos, const glm::vec2& bHalf);

    void SetupSceneVisuals();
    void SetupStaticBoundaries();
    void SetupDynamicBodies();
    void LogLayoutSnapshot() const;

    void BuildConsensusBindings(int playerCount);
    void HandleConsensusInput();
    [[nodiscard]] bool IsAllPlayersHolding(Util::Keycode PlayerKeyConfig::* keyMember) const;

    void SyncDynamicSprites() const;
    void UpdateTimerText() const;

    void UpdateCheckpoint();
    void TryPickKey();
    void UpdateKeyFollow() const;
    void TryOpenDoor();
    void TryClearLevel();
    [[nodiscard]] bool IsPlayerInOpenDoorZone() const;

    void HandleHazardsAndRespawn();
    [[nodiscard]] bool IsStompCollision(const IPhysicsBody& mobBody) const;
    void TriggerStompBounce(const IPhysicsBody& mobBody) const;
    void StartDeath();
    void RespawnPlayer();

    void UpdateShake();
    void ResetShake() const;

    static constexpr int   kLevelIndex = 2;
    static constexpr float kRoomLeftX  = -624.0f;
    static constexpr float kRoomRightX = 624.0f;
    static constexpr float kRoomTopY   = 337.0f;
    static constexpr float kRoomFloorY = -337.0f;

    static constexpr float kBottomTopY = -300.0f;
    static constexpr float kMidTopY    = -40.0f;
    static constexpr float kHighTopY   = 210.0f;

    static constexpr float kDeathRespawnDelaySec = 2.0f;
    static constexpr float kShakeDurationSec = 0.35f;
    static constexpr float kShakeAmplitudePx = 18.0f;

    PhysicsWorld m_World;

    std::shared_ptr<Character> m_CeilingSprite;
    std::shared_ptr<Character> m_LeftWallSprite;
    std::shared_ptr<Character> m_RightWallSprite;

    std::shared_ptr<Character> m_GamePadSprite;
    std::shared_ptr<Character> m_LFloorSprite;
    std::shared_ptr<Character> m_RFloorSprite;
    std::shared_ptr<Character> m_LMidFloorSprite;
    std::shared_ptr<Character> m_RMidFloorSprite;
    std::shared_ptr<Character> m_LHighFloorSprite;
    std::shared_ptr<Character> m_RHighFloorSprite;
    std::shared_ptr<Character> m_LLiftSprite;
    std::shared_ptr<Character> m_RLiftSprite;
    std::shared_ptr<Character> m_MidBlockLeftSprite;
    std::shared_ptr<Character> m_MidBlockRightSprite;
    std::shared_ptr<Character> m_PipeSprite;
    std::shared_ptr<Character> m_Mob1Sprite;
    std::shared_ptr<Character> m_Mob2Sprite;
    std::shared_ptr<Character> m_FlagSprite;
    std::shared_ptr<Character> m_KeySprite;
    std::shared_ptr<Character> m_DeadCatSprite;

    std::shared_ptr<GameText> m_CheckText;
    std::shared_ptr<GameText> m_TimerText;

    std::shared_ptr<PlayerCat> m_Player;

    std::shared_ptr<MovingLiftBody> m_LeftLiftBody;
    std::shared_ptr<MovingLiftBody> m_RightLiftBody;
    std::shared_ptr<PatrolMobBody>  m_Mob2Body;
    std::shared_ptr<PipeMobBody>    m_Mob1Body;

    std::vector<PlayerKeyConfig> m_ConsensusBindings;

    glm::vec2 m_SpawnPoint = {-520.0f, kBottomTopY + PlayerCat::kHalfHeight};
    glm::vec2 m_RespawnPoint = m_SpawnPoint;
    glm::vec2 m_PlayerPrevPos = m_SpawnPoint;

    bool  m_CheckpointReached = false;
    bool  m_HasKey = false;
    bool  m_DoorOpened = false;
    bool  m_PlayerDead = false;
    bool  m_ClearDone = false;

    float m_ElapsedSec = 0.0f;
    float m_DeathWaitSec = 0.0f;
    float m_ShakeTimerSec = 0.0f;
    float m_ShakeSeed = 0.0f;
    bool  m_JumpConsensusLatched = false;
};

#endif // LEVEL_THREE_SCENE_HPP

