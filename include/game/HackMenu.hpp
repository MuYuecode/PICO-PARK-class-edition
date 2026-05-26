#ifndef PICOPART_HACK_MENU_HPP
#define PICOPART_HACK_MENU_HPP

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "game/Character.hpp"
#include "game/GameText.hpp"
#include "Util/Renderer.hpp"

class HackMenu {
public:
    struct Item {
        std::string label;
        bool isToggle = false;
        bool enabled = false;
        std::function<void(bool)> onToggle;
        std::function<void()> onClick;
    };

    HackMenu();

    void SetItems(std::vector<Item> items);
    void AddToRoot(Util::Renderer& root);
    void RemoveFromRoot(Util::Renderer& root);
    void Update();

private:
    struct RowVisual {
        std::shared_ptr<Character> background;
        std::shared_ptr<GameText> label;
        std::string imagePath;
    };

    void EnsureVisualCount();
    void SetRowImage(RowVisual& row, const std::string& imagePath) const;
    void UpdateLayout();
    void UpdateRowImages();
    [[nodiscard]] int HitRow(const glm::vec2& mousePos) const;
    [[nodiscard]] bool HitHeader(const glm::vec2& mousePos) const;

    std::vector<Item> m_Items;
    std::shared_ptr<Character> m_Header;
    std::shared_ptr<GameText> m_TitleText;
    std::vector<RowVisual> m_Rows;

    glm::vec2 m_Position = {-500.0f, 310.0f};
    glm::vec2 m_DragOffset = {0.0f, 0.0f};
    bool m_Expanded = false;
    bool m_Dragging = false;
    bool m_DragMoved = false;
    bool m_AddedToRoot = false;
    int m_HoveredRow = -1;
};

#endif // PICOPART_HACK_MENU_HPP
