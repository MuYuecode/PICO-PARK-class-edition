#include "gameplay/Character.hpp"
#include "Util/Image.hpp"
#include "app/AppUtil.hpp"


Character::Character(const std::string& ImagePath) {
    SetImage(ImagePath);
    ResetPosition();
}

void Character::SetImage(const std::string& ImagePath) {
    m_ImagePath = ImagePath;
    m_Drawable = std::make_shared<Util::Image>(m_ImagePath);
}

bool Character::IsMouseHovering() const {
    return AppUtil::IsMouseHoveringByRect(GetPosition(), GetSize());
}

bool Character::IsLeftClicked() const {
    return AppUtil::IsLeftClickedByRect(GetPosition(), GetSize());
}

