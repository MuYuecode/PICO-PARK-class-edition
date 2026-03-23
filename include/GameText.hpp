//
// Created by cody2 on 2026/3/12.
//

#ifndef GAMETEXT_HPP
#define GAMETEXT_HPP

#include <string>
#include "Util/GameObject.hpp"
#include "Util/Text.hpp"
#include "Util/Color.hpp"

class GameText : public Util::GameObject {
public:
    GameText(const std::string& text, int size, const Util::Color& color)
        : GameObject(std::make_unique<Util::Text>(GA_RESOURCE_DIR"/Font/TerminusTTFWindows-Bold-4.49.3.ttf", size, text, color), 100) {
        m_Visible = true;
    }

    [[nodiscard]] bool GetVisibility() const { return m_Visible; }
    [[nodiscard]] glm::vec2 GetPosition() const { return m_Transform.translation; }
    [[nodiscard]] glm::vec2 GetSize() const { return GetScaledSize(); }

    void SetPosition(const glm::vec2& Position) { m_Transform.translation = Position; }
    // 重新設定顏色 用於表達不同意義
    void SetColor(const Util::Color& color) const ;
    // 重新設定文字 用於選單值變更時更新顯示
    void SetText(const std::string& text) const ;

    // 新增：偵測滑鼠是否懸停在文字範圍內
    [[nodiscard]] bool IsMouseHovering() const;

    // 新增：偵測文字是否被滑鼠左鍵點擊
    [[nodiscard]] bool IsLeftClicked() const;
};

#endif //GAMETEXT_HPP