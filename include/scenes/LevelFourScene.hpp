#ifndef LEVEL_FOUR_SCENE_HPP
#define LEVEL_FOUR_SCENE_HPP

#include <memory>
#include <vector>

#include "core/Scene.hpp"
#include "gameplay/Character.hpp"
#include "gameplay/PlayerCat.hpp"
#include "gameplay/PlayerKeyConfig.hpp"
#include "physics/BulletBody.hpp"
#include "physics/PhysicsWorld.hpp"
#include "ui/GameText.hpp"

class StaticBody;

class LevelFourScene : public Scene {
public:
    explicit LevelFourScene(SceneServices services);
    ~LevelFourScene() override = default;

    void OnEnter() override;
    void OnExit() override;
    void Update() override;
    void PauseGameplay() override;
    void ResumeGameplay() override;

private:
    struct PlayerBinding {
        std::shared_ptr<PlayerCat> cat;
        PlayerKeyConfig key;
    };

    enum class JarState {
        Jar0,
        Jar1,
        Jar2,
        Gone,
    };

    static bool AabbOverlap(const glm::vec2& aPos, const glm::vec2& aHalf,
                            const glm::vec2& bPos, const glm::vec2& bHalf);

    void SetupSceneVisuals();
    void SetupStaticBoundaries();
    void SpawnPlayers(int count);
    void ApplyInitialFormation() const;
    void HandlePlayerInput() const;
    void UpdateTimerText() const;

    float FloorTopY() const;
    float CurrentBulletSpeed() const;
    void SpawnBullet();
    void DeactivateBullet();
    void SyncBulletSprite() const;
    void UpdateBulletSpawner(float dtSec);
    void HandleBulletHits();
    void AdvanceJarState();

    void TryPickKey();
    void UpdateKeyFollow() const;
    void TryOpenDoorAndClear();
    void UpdateDoorEntryAndClear();

    static constexpr int kLevelIndex = 3;
    static constexpr float kViewLeftX = -640.0f;
    static constexpr float kViewRightX = 640.0f;
    static constexpr float kViewTopY = 360.0f;
    static constexpr float kViewBottomY = -360.0f;

    static constexpr glm::vec2 kKeyHalf = {20.0f, 26.0f};
    static constexpr glm::vec2 kBulletHalf = {9.0f, 9.0f};
    static constexpr float kBulletBaseSpeed = 240.0f;
    static constexpr float kBulletRespawnDelaySec = 0.8f;
    static constexpr float kInitialBulletDelaySec = 1.0f;

    PhysicsWorld m_World;

    std::shared_ptr<Character> m_CeilingSprite;
    std::shared_ptr<Character> m_FloorSprite;
    std::shared_ptr<Character> m_LHighWallSprite;
    std::shared_ptr<Character> m_LMidWallSprite;
    std::shared_ptr<Character> m_RHighWallSprite;
    std::shared_ptr<Character> m_RMidWallSprite;
    std::shared_ptr<Character> m_JarSprite;
    std::shared_ptr<Character> m_ShooterSprite;
    std::shared_ptr<Character> m_KeySprite;

    std::shared_ptr<Character> m_BulletSprite;
    std::shared_ptr<GameText> m_TimerText;
    std::shared_ptr<BulletBody> m_BulletBody;
    StaticBody* m_JarBody = nullptr;
    StaticBody* m_ShooterBody = nullptr;

    std::vector<PlayerBinding> m_Players;
    std::vector<bool> m_PlayerEntered;

    JarState m_JarState = JarState::Jar0;
    int m_KeyCarrierIdx = -1;
    int m_EnteredCount = 0;

    bool m_DoorOpened = false;
    bool m_ClearDone = false;
    bool m_BulletActive = false;

    float m_BulletCooldownSec = kInitialBulletDelaySec;
    float m_ElapsedSec = 0.0f;
};

#endif
