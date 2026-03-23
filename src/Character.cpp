#include "Character.hpp"
#include "Util/Image.hpp"
#include "AppUtil.hpp"


Character::Character(const std::string& ImagePath) {
    SetImage(ImagePath);
    ResetPosition();
}

void Character::SetImage(const std::string& ImagePath) {
    m_ImagePath = ImagePath;
    m_Drawable = std::make_shared<Util::Image>(m_ImagePath);
}

bool Character::IsMouseHovering() const {
    return AppUtil::IsMouseHovering(*this);
}

bool Character::IsLeftClicked() const {
    return AppUtil::IsLeftClicked(*this);
}
