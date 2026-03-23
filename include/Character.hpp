#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include <string>
#include "Util/GameObject.hpp"



class Character : public Util::GameObject
{
public:
    explicit Character(const std::string& ImagePath);

    Character(const Character&) = delete;
    Character(Character&&) = delete;
    Character& operator=(const Character&) = delete;
    Character& operator=(Character&&) = delete;

    [[nodiscard]] const std::string& GetImagePath() const { return m_ImagePath; }
    [[nodiscard]] const glm::vec2& GetPosition() const { return m_Transform.translation; }
    [[nodiscard]] bool GetVisibility() const { return m_Visible; }
    [[nodiscard]] glm::vec2 GetSize() const { return GetScaledSize(); }

    void SetImage(const std::string& ImagePath);
    void SetPosition(const glm::vec2& Position) { m_Transform.translation = Position; }
    void SetScale(const glm::vec2& Scale) { m_Transform.scale = Scale; }

    // 偵測滑鼠是否在物件範圍內
    [[nodiscard]] bool IsMouseHovering() const;

    // 偵測物件是否被滑鼠左鍵點擊
    [[nodiscard]] bool IsLeftClicked() const;

private:
    void ResetPosition() { m_Transform.translation = {0, 0}; }

    std::string m_ImagePath;
};


#endif //CHARACTER_HPP
