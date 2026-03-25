#ifndef PICOPART_GAMECONTEXT_HPP
#define PICOPART_GAMECONTEXT_HPP

#include "BGMPlayer.hpp"
#include "Util/Renderer.hpp"
#include "Character.hpp"
#include "PlayerCat.hpp"
#include <vector>

struct GameContext {
    Util::Renderer& Root;

    // always exist
    std::shared_ptr<Character>  Background;
    std::shared_ptr<BGMPlayer>  BGMPlayer;

    // In many Scene
    std::shared_ptr<Character> Header;
    std::shared_ptr<Character> Door;
    std::shared_ptr<Character>  Floor;

    std::vector<std::shared_ptr<PlayerCat>> StartupCats;
    
    int SelectedPlayerCount = 2;
    int CooperativePushPower = 1;

    bool ShouldQuit = false;

    // character color
    static constexpr std::array<const char*, 8> kCatColorOrder = {
        "blue", "red", "yellow", "green", "purple", "pink", "orange", "gray"
    };

    explicit GameContext(Util::Renderer& root)
        : Root(root) {}

    GameContext(const GameContext&)            = delete;
    GameContext& operator=(const GameContext&) = delete;
    GameContext(GameContext&&)                 = delete;
    GameContext& operator=(GameContext&&)      = delete;
};

#endif //PICOPART_GAMECONTEXT_HPP