#include "game/HackMenu.hpp"

#include "Util/Input.hpp"
#include "Util/Keycode.hpp"

#include <algorithm>
#include <utility>

namespace {
constexpr float kHeaderW = 260.0f;
constexpr float kHeaderH = 40.0f;
constexpr float kRowW = 260.0f;
constexpr float kRowH = 40.0f;
constexpr float kZBase = 80.0f;

const Util::Color kTextColor = Util::Color::FromRGB(15, 15, 15, 255);

glm::vec2 RowPosition(const glm::vec2& headerPos, int index) {
    return {headerPos.x, headerPos.y - kHeaderH - kRowH * static_cast<float>(index)};
}

bool HitRect(const glm::vec2& mousePos, const glm::vec2& center, const glm::vec2& size) {
    const glm::vec2 half = size * 0.5f;
    return mousePos.x >= center.x - half.x &&
           mousePos.x <= center.x + half.x &&
           mousePos.y >= center.y - half.y &&
           mousePos.y <= center.y + half.y;
}
}

HackMenu::HackMenu() {
    m_Header = std::make_shared<Character>(GA_RESOURCE_DIR "/Hack/header.png");
    m_Header->SetZIndex(kZBase);

    m_TitleText = std::make_shared<GameText>("HACK MENU", 24, kTextColor);
    m_TitleText->SetZIndex(kZBase + 1.0f);

    UpdateLayout();
}

void HackMenu::SetItems(std::vector<Item> items) {
    m_Items = std::move(items);
    EnsureVisualCount();
    UpdateLayout();
    UpdateRowImages();
}

void HackMenu::AddToRoot(Util::Renderer& root) {
    if (m_AddedToRoot) return;

    root.AddChild(m_Header);
    root.AddChild(m_TitleText);
    for (const auto& row : m_Rows) {
        root.AddChild(row.background);
        root.AddChild(row.label);
    }
    m_AddedToRoot = true;
    UpdateLayout();
}

void HackMenu::RemoveFromRoot(Util::Renderer& root) {
    if (!m_AddedToRoot) return;

    root.RemoveChild(m_Header);
    root.RemoveChild(m_TitleText);
    for (const auto& row : m_Rows) {
        root.RemoveChild(row.background);
        root.RemoveChild(row.label);
    }
    m_AddedToRoot = false;
}

void HackMenu::Update() {
    const glm::vec2 mousePos = Util::Input::GetCursorPosition();
    const bool mouseDown = Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB);
    const bool mouseHeld = Util::Input::IsKeyPressed(Util::Keycode::MOUSE_LB);
    const bool mouseUp = Util::Input::IsKeyUp(Util::Keycode::MOUSE_LB);

    m_HoveredRow = m_Expanded ? HitRow(mousePos) : -1;

    if (mouseDown && HitHeader(mousePos)) {
        m_Dragging = true;
        m_DragMoved = false;
        m_DragOffset = m_Position - mousePos;
    }

    if (m_Dragging && mouseHeld) {
        const glm::vec2 nextPos = mousePos + m_DragOffset;
        const glm::vec2 delta = nextPos - m_Position;
        if ((delta.x * delta.x + delta.y * delta.y) > 4.0f) {
            m_DragMoved = true;
        }
        m_Position = nextPos;
        m_Position.x = std::clamp(m_Position.x, -510.0f, 510.0f);
        m_Position.y = std::clamp(m_Position.y, -300.0f, 330.0f);
        UpdateLayout();
    }

    if (m_Dragging && mouseUp) {
        m_Dragging = false;
        if (!m_DragMoved && HitHeader(mousePos)) {
            m_Expanded = !m_Expanded;
            UpdateLayout();
        }
    } else if (!m_Dragging && mouseDown && m_HoveredRow >= 0 &&
               m_HoveredRow < static_cast<int>(m_Items.size())) {
        auto& item = m_Items[static_cast<size_t>(m_HoveredRow)];
        if (item.isToggle) {
            item.enabled = !item.enabled;
            if (item.onToggle) item.onToggle(item.enabled);
        } else if (item.onClick) {
            item.onClick();
        }
        UpdateRowImages();
    }

    UpdateRowImages();
}

void HackMenu::EnsureVisualCount() {
    while (m_Rows.size() < m_Items.size()) {
        RowVisual row;
        row.background = std::make_shared<Character>(GA_RESOURCE_DIR "/Hack/row_off.png");
        row.imagePath = GA_RESOURCE_DIR "/Hack/row_off.png";
        row.background->SetZIndex(kZBase);
        row.label = std::make_shared<GameText>("-", 20, kTextColor);
        row.label->SetZIndex(kZBase + 1.0f);
        m_Rows.push_back(row);
    }

    for (size_t i = 0; i < m_Rows.size(); ++i) {
        const bool visible = m_Expanded && i < m_Items.size();
        m_Rows[i].background->SetVisible(visible);
        m_Rows[i].label->SetVisible(visible);
    }
}

void HackMenu::SetRowImage(RowVisual& row, const std::string& imagePath) const {
    if (row.imagePath == imagePath) return;
    row.imagePath = imagePath;
    row.background->SetImage(imagePath);
}

void HackMenu::UpdateLayout() {
    if (m_Header == nullptr || m_TitleText == nullptr) return;

    m_Header->SetPosition(m_Position);
    m_TitleText->SetPosition({m_Position.x - 42.0f, m_Position.y - 1.0f});

    for (int i = 0; i < static_cast<int>(m_Rows.size()); ++i) {
        const bool visible = m_Expanded && i < static_cast<int>(m_Items.size());
        const glm::vec2 pos = RowPosition(m_Position, i);
        m_Rows[static_cast<size_t>(i)].background->SetPosition(pos);
        m_Rows[static_cast<size_t>(i)].label->SetPosition({pos.x - 42.0f, pos.y - 1.0f});
        m_Rows[static_cast<size_t>(i)].background->SetVisible(visible);
        m_Rows[static_cast<size_t>(i)].label->SetVisible(visible);
        if (i < static_cast<int>(m_Items.size())) {
            m_Rows[static_cast<size_t>(i)].label->SetText(m_Items[static_cast<size_t>(i)].label);
        }
    }
}

void HackMenu::UpdateRowImages() {
    for (int i = 0; i < static_cast<int>(m_Rows.size()); ++i) {
        if (i >= static_cast<int>(m_Items.size())) continue;

        const auto& item = m_Items[static_cast<size_t>(i)];
        const bool active = item.isToggle && item.enabled;
        const bool hovered = i == m_HoveredRow;

        std::string imagePath = GA_RESOURCE_DIR "/Hack/row_off.png";
        if (active) {
            imagePath = GA_RESOURCE_DIR "/Hack/row_on.png";
        } else if (hovered) {
            imagePath = GA_RESOURCE_DIR "/Hack/row_hover.png";
        }
        SetRowImage(m_Rows[static_cast<size_t>(i)], imagePath);
    }
}

int HackMenu::HitRow(const glm::vec2& mousePos) const {
    for (int i = 0; i < static_cast<int>(m_Items.size()); ++i) {
        if (HitRect(mousePos, RowPosition(m_Position, i), {kRowW, kRowH})) {
            return i;
        }
    }
    return -1;
}

bool HackMenu::HitHeader(const glm::vec2& mousePos) const {
    return HitRect(mousePos, m_Position, {kHeaderW, kHeaderH});
}
