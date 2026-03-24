#ifndef PICOPART_GAMECONTEXT_HPP
#define PICOPART_GAMECONTEXT_HPP

#include "BGMPlayer.hpp"
#include "Util/Renderer.hpp"
#include "Character.hpp"
#include "PlayerCat.hpp"
#include <vector>

struct GameContext {
    Util::Renderer& Root;

    // 永久留在渲染樹的物件(所有場景都看得到)
    std::shared_ptr<Character>  Background;
    std::shared_ptr<Character>  Floor;
    std::shared_ptr<BGMPlayer>  BGMPlayer;

    // 在多個 Scene 都要顯示
    // 所以移到 GameContext，由 AppStart 加入渲染樹後就不再移除
    std::shared_ptr<Character> Header;
    std::shared_ptr<Character> Door;

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