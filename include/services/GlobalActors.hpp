#ifndef GLOBAL_ACTORS_HPP
#define GLOBAL_ACTORS_HPP

#include <array>
#include <memory>
#include <vector>

#include "services/IGlobalActors.hpp"

class GlobalActors final : public IGlobalActors {
public:
    explicit GlobalActors(Util::Renderer& root)
        : m_Root(root) {}

    Util::Renderer& Root() override { return m_Root; }

    std::shared_ptr<Character> Background() const override { return m_Background; }
    std::shared_ptr<Character> Header() const override { return m_Header; }
    std::shared_ptr<Character> Door() const override { return m_Door; }
    std::shared_ptr<Character> Floor() const override { return m_Floor; }
    std::shared_ptr<PushableBox> TestBox() const override { return m_TestBox; }
    std::vector<std::shared_ptr<PlayerCat>>& StartupCats() override { return m_StartupCats; }

    const std::array<const char*, 8>& CatColorOrder() const override { return kCatColorOrder; }

    void SetBackground(std::shared_ptr<Character> obj) { m_Background = std::move(obj); }
    void SetHeader(std::shared_ptr<Character> obj) { m_Header = std::move(obj); }
    void SetDoor(std::shared_ptr<Character> obj) { m_Door = std::move(obj); }
    void SetFloor(std::shared_ptr<Character> obj) { m_Floor = std::move(obj); }
    void SetTestBox(std::shared_ptr<PushableBox> obj) { m_TestBox = std::move(obj); }

private:
    Util::Renderer& m_Root;

    std::shared_ptr<Character> m_Background;
    std::shared_ptr<Character> m_Header;
    std::shared_ptr<Character> m_Door;
    std::shared_ptr<Character> m_Floor;
    std::shared_ptr<PushableBox> m_TestBox;

    std::vector<std::shared_ptr<PlayerCat>> m_StartupCats;

    static constexpr std::array<const char*, 8> kCatColorOrder = {
        "blue", "red", "yellow", "green", "purple", "pink", "orange", "gray"
    };
};

#endif // GLOBAL_ACTORS_HPP


