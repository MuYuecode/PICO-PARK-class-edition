#ifndef LEVEL_SCENE_HELPERS_HPP
#define LEVEL_SCENE_HELPERS_HPP

#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "game/CatAssets.hpp"
#include "game/Character.hpp"
#include "game/PlayerCat.hpp"
#include "scenes/KeyboardConfigScene.hpp"
#include "systems/IAudioService.hpp"
#include "systems/IGlobalActors.hpp"
#include "systems/IPhysicsBody.hpp"
#include "systems/ISessionState.hpp"
#include "systems/PhysicsWorld.hpp"
#include "systems/SaveManager.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"

namespace LevelSceneHelpers {

inline bool AabbOverlap(const glm::vec2& aPos, const glm::vec2& aHalf,
                        const glm::vec2& bPos, const glm::vec2& bHalf) {
    return std::abs(aPos.x - bPos.x) < (aHalf.x + bHalf.x) &&
           std::abs(aPos.y - bPos.y) < (aHalf.y + bHalf.y);
}

template <typename PlayerBindings>
void SpawnPlayerBindings(PlayerBindings& players, int count,
                         IGlobalActors& actors, ISessionState& session,
                         PhysicsWorld& world, float baseZ = 20.0f) {
    players.clear();
    players.reserve(static_cast<size_t>(count));

    for (int i = 0; i < count; ++i) {
        const std::string color = actors.CatColorOrder()[static_cast<size_t>(i)];
        auto cat = std::make_shared<PlayerCat>(
            CatAssets::BuildFullAnimPaths(color),
            Util::Keycode::UNKNOWN,
            Util::Keycode::UNKNOWN,
            Util::Keycode::UNKNOWN);

        players.emplace_back();
        auto& pb = players.back();
        pb.cat = cat;
        pb.key = session.GetAppliedKeyConfigs()[static_cast<size_t>(i)];

        if (i == 0 && pb.key.left == Util::Keycode::UNKNOWN &&
            pb.key.right == Util::Keycode::UNKNOWN) {
            pb.key = KeyboardConfigScene::k_Default1P;
        } else if (i == 1 && pb.key.left == Util::Keycode::UNKNOWN &&
                   pb.key.right == Util::Keycode::UNKNOWN) {
            pb.key = KeyboardConfigScene::k_Default2P;
        }

        cat->SetZIndex(baseZ + static_cast<float>(i) * 0.01f);
        actors.Root().AddChild(cat);
        world.Register(cat);
    }
}

template <typename PlayerBindings>
void HandlePlayerInput(const PlayerBindings& players, IAudioService& audio,
                       bool flightEnabled = false) {
    for (const auto& pb : players) {
        if (pb.cat == nullptr || !pb.cat->IsActive() || !pb.cat->GetInputEnabled()) continue;

        int moveDir = 0;
        const bool goLeft = (pb.key.left != Util::Keycode::UNKNOWN) &&
                            Util::Input::IsKeyPressed(pb.key.left);
        const bool goRight = (pb.key.right != Util::Keycode::UNKNOWN) &&
                             Util::Input::IsKeyPressed(pb.key.right);

        if (goLeft && !goRight) moveDir = -1;
        else if (goRight && !goLeft) moveDir = 1;

        pb.cat->SetMoveDir(moveDir);

        int verticalDir = 0;
        if (flightEnabled) {
            const bool goUp = (pb.key.up != Util::Keycode::UNKNOWN) &&
                              Util::Input::IsKeyPressed(pb.key.up);
            const bool goDown = (pb.key.down != Util::Keycode::UNKNOWN) &&
                                Util::Input::IsKeyPressed(pb.key.down);
            if (goUp && !goDown) verticalDir = 1;
            else if (goDown && !goUp) verticalDir = -1;
        }
        pb.cat->SetHackFlightVerticalDir(verticalDir);

        const bool wantJump = (pb.key.jump != Util::Keycode::UNKNOWN) &&
                              Util::Input::IsKeyDown(pb.key.jump);
        if (!flightEnabled && pb.cat->IsGrounded() && wantJump) {
            pb.cat->Jump();
            audio.PlaySe(SoundEffect::Jump);
        }
    }
}

template <typename PlayerBindings>
void TeleportActivePlayersTo(const PlayerBindings& players, const glm::vec2& pos,
                             float spacing = 42.0f) {
    int activeIndex = 0;
    for (const auto& pb : players) {
        if (pb.cat == nullptr || !pb.cat->IsActive()) continue;
        pb.cat->SetPosition(pos + glm::vec2{spacing * static_cast<float>(activeIndex), 0.0f});
        ++activeIndex;
    }
}

template <typename PlayerBindings>
void ApplyHackFlightToPlayers(const PlayerBindings& players, bool enabled) {
    for (const auto& pb : players) {
        if (pb.cat == nullptr) continue;
        pb.cat->SetHackFlightEnabled(enabled);
    }
}

template <typename PlayerBindings>
void TryOpenDoorWithKeyCarrier(const PlayerBindings& players, int keyCarrierIdx,
                               const std::shared_ptr<Character>& door,
                               const std::shared_ptr<Character>& keySprite,
                               bool& doorOpened, IAudioService& audio) {
    if (keyCarrierIdx < 0 || keyCarrierIdx >= static_cast<int>(players.size())) return;
    if (door == nullptr || doorOpened) return;

    const auto& carrier = players[static_cast<size_t>(keyCarrierIdx)].cat;
    if (carrier == nullptr || !carrier->IsActive()) return;

    const glm::vec2 doorPos = door->GetPosition();
    const glm::vec2 doorHalf = glm::abs(door->GetScaledSize()) * 0.5f;
    const bool carrierTouchDoor = AabbOverlap(
        carrier->GetPosition(), carrier->GetHalfSize(),
        doorPos, doorHalf);

    if (!carrierTouchDoor) return;

    doorOpened = true;
    door->SetImage(GA_RESOURCE_DIR "/Image/Background/door_open.png");
    if (keySprite != nullptr) {
        keySprite->SetVisible(false);
    }
    audio.PlaySe(SoundEffect::Door);
}

template <typename PlayerBindings>
void UpdateDoorEntryAndClear(const PlayerBindings& players,
                             std::vector<bool>& playerEntered,
                             int& enteredCount,
                             bool& clearDone,
                             const std::shared_ptr<Character>& door,
                             int selectedPlayerCount,
                             int levelIndex,
                             float elapsedSec,
                             IAudioService& audio) {
    if (door == nullptr) return;

    const glm::vec2 doorPos = door->GetPosition();
    const glm::vec2 doorHalf = glm::abs(door->GetScaledSize()) * 0.5f;
    const int totalPlayers = static_cast<int>(players.size());

    for (int i = 0; i < totalPlayers; ++i) {
        if (i >= static_cast<int>(playerEntered.size())) break;
        if (playerEntered[static_cast<size_t>(i)]) continue;

        const auto& pb = players[static_cast<size_t>(i)];
        auto& cat = pb.cat;
        if (cat == nullptr || !cat->IsActive()) continue;

        const bool touchingDoor = AabbOverlap(
            cat->GetPosition(), cat->GetHalfSize(),
            doorPos, doorHalf);
        const bool pressedUp = (pb.key.up != Util::Keycode::UNKNOWN) &&
                               Util::Input::IsKeyDown(pb.key.up);

        if (!(touchingDoor && pressedUp)) continue;

        playerEntered[static_cast<size_t>(i)] = true;
        ++enteredCount;

        cat->SetVisible(false);
        cat->SetInputEnabled(false);
        cat->SetActive(false);
        cat->SetPosition({640.0f, -360.0f});
    }

    if (!clearDone && enteredCount == totalPlayers) {
        SaveManager::UpdateBestTime(levelIndex, selectedPlayerCount, elapsedSec);
        clearDone = true;
        audio.PlaySe(SoundEffect::Win);
    }
}

} // namespace LevelSceneHelpers

#endif // LEVEL_SCENE_HELPERS_HPP
