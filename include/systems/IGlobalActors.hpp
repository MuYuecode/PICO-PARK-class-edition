#ifndef I_GLOBAL_ACTORS_HPP
#define I_GLOBAL_ACTORS_HPP

#include <array>
#include <memory>
#include <vector>

#include "game/Character.hpp"
#include "game/PlayerCat.hpp"
#include "Util/Renderer.hpp"

class IGlobalActors {
public:
    virtual ~IGlobalActors() = default;

    virtual Util::Renderer& Root() = 0;

    virtual std::shared_ptr<Character> Background() const = 0;
    virtual std::shared_ptr<Character> Header() const = 0; // TODO: don't need put in Actors
    virtual std::shared_ptr<Character> Door() const = 0;
    virtual std::shared_ptr<Character> Floor() const = 0; // TODO: don't need put in Actors
    virtual std::vector<std::shared_ptr<PlayerCat>>& StartupCats() = 0;

    virtual const std::array<const char*, 8>& CatColorOrder() const = 0;
};

#endif // I_GLOBAL_ACTORS_HPP


