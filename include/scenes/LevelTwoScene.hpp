#ifndef LEVEL_TWO_SCENE_HPP
#define LEVEL_TWO_SCENE_HPP

#include <memory>
#include <vector>

#include "game/Character.hpp"
#include "game/GameText.hpp"
#include "game/HackMenu.hpp"
#include "systems/PhysicsWorld.hpp"
#include "game/PlayerCat.hpp"
#include "game/PlayerKeyConfig.hpp"
#include "app/Scene.hpp"

class LevelTwoScene : public Scene {
public:
    explicit LevelTwoScene(SceneServices services);
    ~LevelTwoScene() override = default;

    void OnEnter() override;
    void OnExit() override;
    void Update() override;
    void PauseGameplay() override;
    void ResumeGameplay() override;

private:
    struct PlayerBinding {
        std::shared_ptr<PlayerCat> cat;
        PlayerKeyConfig            key;
    };

    class MovingPlankBody;

    void SetupStaticBoundaries();
    void SpawnPlayers(int count);
    void ApplyInitialFormation() const;

    void HandlePlayerInput() const;
    void UpdateTimerText() const;

    void SetupPlanks(int playerCount);
    void SyncPlankSprites() const;
    void TryActivateButton();
    void UpdateFallenPlayers();

    void TryPickKey();
    void UpdateKeyFollow() const;
    void TryOpenDoorAndClear();
    void UpdateDoorEntryAndClear();
    void SetupHackMenu();
    void TeleportPlayersTo(const glm::vec2& pos) const;
    void ApplyHackFlightToPlayers(bool enabled) const;
    void HackTeleportToKey() const;
    void HackTeleportToDoor() const;
    void HackGrantKey();
    void HackOpenDoor();
    void HackTeleportToButton() const;
    void HackPressButton();

    static constexpr int   kLevelIndex = 1;
    static constexpr float kRoomLeftX  = -624.0f;
    static constexpr float kRoomRightX = 624.0f;
    static constexpr float kRoomTopY   = 337.0f;
    static constexpr float kRoomFloorY = -337.0f;

    static constexpr float kRightSectionShiftX = 72.0f;

    static constexpr float kRightFloorBaseMinX  = 300.0f;
    static constexpr float kRightFloorBaseMaxX  = 550.0f;
    static constexpr float kRightFloorMinX      = kRightFloorBaseMinX + kRightSectionShiftX;
    static constexpr float kRightFloorMaxX      = kRightFloorBaseMaxX + kRightSectionShiftX;
    static constexpr float kRightFloorCenterX   = (kRightFloorMinX + kRightFloorMaxX) * 0.5f;

    static constexpr float kCeilingLeftX      = -178.0f;
    static constexpr float kCeilingRightX     = 672.0f;

    static constexpr float kButtonTriggerHalfW = 28.0f;
    static constexpr float kButtonTriggerHalfH = 18.0f;

    static constexpr float kPlankMoveSpeed = 160.0f; // px/s
    static constexpr float kPlankSeamInsetPx = 2.0f; // each side contributes 2px seam padding
    static constexpr int   kPlankCount = 10;

    static constexpr float kFallTeleportThresholdY = -390.0f;
    static constexpr glm::vec2 kTeleportPos = {-540.0f, 300.0f};

    static constexpr glm::vec2 kKeyHalf  = {20.0f, 26.0f};
    static constexpr glm::vec2 kDoorHalf = {31.0f, 34.0f};

    PhysicsWorld m_World;

    std::shared_ptr<Character> m_LeftFloorSprite;
    std::shared_ptr<Character> m_RightFloorSprite;
    std::shared_ptr<Character> m_CeilingSprite;
    std::shared_ptr<Character> m_LeftWallSprite;
    std::shared_ptr<Character> m_RightWallSprite;
    std::shared_ptr<Character> m_KeySprite;
    std::shared_ptr<Character> m_ButtonSprite;

    std::vector<std::shared_ptr<Character>>       m_PlankSprites;
    std::vector<std::shared_ptr<MovingPlankBody>> m_PlankBodies;
    std::vector<float>                            m_PlankTargetX;

    std::shared_ptr<GameText> m_TimerText;
    HackMenu m_HackMenu;

    std::vector<PlayerBinding> m_Players;

    bool  m_ButtonPressed = false;
    int   m_KeyCarrierIdx = -1;
    bool  m_DoorOpened    = false;
    bool  m_FlightEnabled = false;
    float m_ElapsedSec    = 0.0f;

    std::vector<bool> m_PlayerEntered;
    int  m_EnteredCount = 0;
    bool m_ClearDone    = false;
};

#endif // LEVEL_TWO_SCENE_HPP


