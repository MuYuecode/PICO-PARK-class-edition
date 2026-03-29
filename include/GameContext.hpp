#ifndef PICOPART_GAMECONTEXT_HPP
#define PICOPART_GAMECONTEXT_HPP

#include "BGMPlayer.hpp"
#include "Util/Renderer.hpp"
#include "Character.hpp"
#include "PlayerCat.hpp"
#include "PushableBox.hpp"
#include "PlayerKeyConfig.hpp"
#include <array>
#include <vector>



struct GameContext {
    Util::Renderer& Root;

    std::shared_ptr<Character> Background;
    std::shared_ptr<BGMPlayer> BGMPlayer;

    std::shared_ptr<Character> Header;
    std::shared_ptr<Character> Door;

    std::shared_ptr<Character> Floor;

    std::shared_ptr<PushableBox> TestBox;

    std::vector<std::shared_ptr<PlayerCat>> StartupCats;

    int  SelectedPlayerCount  = 2;
    int  CooperativePushPower = 1;

    std::array<PlayerKeyConfig, 8> AppliedKeyConfigs{};

    static constexpr std::array<const char*, 8> kCatColorOrder = {
        "blue", "red", "yellow", "green", "purple", "pink", "orange", "gray"
    };

    explicit GameContext(Util::Renderer& root) : Root(root) {}
    bool ShouldQuit = false;

    GameContext(const GameContext&)            = delete;
    GameContext& operator=(const GameContext&) = delete;
    GameContext(GameContext&&)                 = delete;
    GameContext& operator=(GameContext&&)      = delete;
};

#endif // PICOPART_GAMECONTEXT_HPP