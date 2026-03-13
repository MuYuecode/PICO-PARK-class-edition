//
// Created by cody2 on 2026/3/13.
//

#include "GameText.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"

// ==========================================
// 實作：偵測滑鼠是否懸停
// ==========================================
bool GameText::IsMouseHovering() const {
    // 取得滑鼠當前座標
    glm::vec2 mousePos = Util::Input::GetCursorPosition();

    // 取得文字物件自身的中心座標
    glm::vec2 myPos = this->GetPosition();
    glm::vec2 mySize = this->GetSize();
    float halfWidth = mySize.x/2 ;
    float halfHeight = mySize.y/2 ;
    // 進行 AABB (矩形範圍) 碰撞計算
    return (mousePos.x >= myPos.x - halfWidth &&
            mousePos.x <= myPos.x + halfWidth &&
            mousePos.y >= myPos.y - halfHeight &&
            mousePos.y <= myPos.y + halfHeight);
}

// ==========================================
// 實作：偵測是否被左鍵點擊
// ==========================================
bool GameText::IsLeftClicked() const {
    // 先確認滑鼠左鍵有被按下
    if (Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
        // 直接重複利用上面寫好的懸停判斷，只要在範圍內就是被點擊了！
        return IsMouseHovering();
    }
    return false;
}