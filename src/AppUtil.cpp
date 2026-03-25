#include "AppUtil.hpp"
#include "Util/Input.hpp"
#include "Util/Logger.hpp"

namespace AppUtil {

    std::string KeycodeToString(Util::Keycode key) {
        using K = Util::Keycode;
        switch (key) {
        case K::A: return "A"; case K::B: return "B"; case K::C: return "C";
        case K::D: return "D"; case K::E: return "E"; case K::F: return "F";
        case K::G: return "G"; case K::H: return "H"; case K::I: return "I";
        case K::J: return "J"; case K::K: return "K"; case K::L: return "L";
        case K::M: return "M"; case K::N: return "N"; case K::O: return "O";
        case K::P: return "P"; case K::Q: return "Q"; case K::R: return "R";
        case K::S: return "S"; case K::T: return "T"; case K::U: return "U";
        case K::V: return "V"; case K::W: return "W"; case K::X: return "X";
        case K::Y: return "Y"; case K::Z: return "Z";
        case K::NUM_1: return "1"; case K::NUM_2: return "2";
        case K::NUM_3: return "3"; case K::NUM_4: return "4";
        case K::NUM_5: return "5"; case K::NUM_6: return "6";
        case K::NUM_7: return "7"; case K::NUM_8: return "8";
        case K::NUM_9: return "9"; case K::NUM_0: return "0";
        case K::RETURN:    return "ENTER";
        case K::ESCAPE:    return "ESCAPE";
        case K::BACKSPACE: return "BACKSPACE";
        case K::TAB:       return "TAB";
        case K::SPACE:     return "SPACE";
        case K::MINUS:        return "-";
        case K::EQUALS:       return "=";
        case K::LEFTBRACKET:  return "[";
        case K::RIGHTBRACKET: return "]";
        case K::BACKSLASH:    return "\\";
        case K::SEMICOLON:    return ";";
        case K::APOSTROPHE:   return "'";
        case K::GRAVE:        return "`";
        case K::COMMA:        return ",";
        case K::PERIOD:       return ".";
        case K::SLASH:        return "/";
        case K::UP:    return "UP";
        case K::DOWN:  return "DOWN";
        case K::LEFT:  return "LEFT";
        case K::RIGHT: return "RIGHT";
        case K::F1:  return "F1";  case K::F2:  return "F2";
        case K::F3:  return "F3";  case K::F4:  return "F4";
        case K::F5:  return "F5";  case K::F6:  return "F6";
        case K::F7:  return "F7";  case K::F8:  return "F8";
        case K::F9:  return "F9";  case K::F10: return "F10";
        case K::F11: return "F11"; case K::F12: return "F12";
        case K::INSERT:   return "INSERT";
        case K::HOME:     return "HOME";
        case K::PAGEUP:   return "PAGE UP";
        case K::DELETE:   return "DELETE";
        case K::END:      return "END";
        case K::PAGEDOWN: return "PAGE DOWN";
        case K::LCTRL:  return "L CTRL";
        case K::RCTRL:  return "R CONTROL";
        case K::LSHIFT: return "L SHIFT";
        case K::RSHIFT: return "R SHIFT";
        case K::LALT:   return "L ALT";
        case K::RALT:   return "R ALT";
        case K::KP_0: return "KP 0"; case K::KP_1: return "KP 1";
        case K::KP_2: return "KP 2"; case K::KP_3: return "KP 3";
        case K::KP_4: return "KP 4"; case K::KP_5: return "KP 5";
        case K::KP_6: return "KP 6"; case K::KP_7: return "KP 7";
        case K::KP_8: return "KP 8"; case K::KP_9: return "KP 9";
        case K::KP_ENTER:    return "KP ENTER";
        case K::KP_PLUS:     return "KP +";
        case K::KP_MINUS:    return "KP -";
        case K::KP_MULTIPLY: return "KP *";
        case K::KP_DIVIDE:   return "KP /";
        case K::KP_PERIOD:   return "KP .";
        case K::CAPSLOCK:    return "CAPS LOCK";
        case K::PRINTSCREEN: return "PRINT SCREEN";
        case K::SCROLLLOCK:  return "SCROLL LOCK";
        case K::PAUSE:       return "PAUSE";
        case K::AC_BACK:     return "BACK";
        case K::MENU:        return "MENU";
        case K::UNKNOWN: return "-";
        default:         return "?";
        }
    }

    Util::Keycode GetAnyKeyDown() {
        using K = Util::Keycode;

        static const std::vector k_Bindable = {
            K::A, K::B, K::C, K::D, K::E, K::F, K::G, K::H, K::I, K::J,
            K::K, K::L, K::M, K::N, K::O, K::P, K::Q, K::R, K::S, K::T,
            K::U, K::V, K::W, K::X, K::Y, K::Z,
            K::NUM_1, K::NUM_2, K::NUM_3, K::NUM_4, K::NUM_5,
            K::NUM_6, K::NUM_7, K::NUM_8, K::NUM_9, K::NUM_0,
            K::SPACE, K::RETURN, K::TAB, K::ESCAPE, K::BACKSPACE,
            K::UP, K::DOWN, K::LEFT, K::RIGHT,
            K::F1, K::F2,  K::F3,  K::F4,  K::F5,  K::F6,
            K::F7, K::F8,  K::F9,  K::F10, K::F11, K::F12,
            K::LCTRL, K::RCTRL, K::LSHIFT, K::RSHIFT, K::LALT, K::RALT,
            K::INSERT, K::HOME, K::PAGEUP, K::DELETE, K::END, K::PAGEDOWN,
            K::MINUS, K::EQUALS, K::LEFTBRACKET, K::RIGHTBRACKET,
            K::BACKSLASH, K::SEMICOLON, K::APOSTROPHE, K::GRAVE,
            K::COMMA, K::PERIOD, K::SLASH,
            K::CAPSLOCK, K::PRINTSCREEN, K::SCROLLLOCK, K::PAUSE,
            K::AC_BACK, K::MENU,
            K::KP_0, K::KP_1, K::KP_2, K::KP_3, K::KP_4,
            K::KP_5, K::KP_6, K::KP_7, K::KP_8, K::KP_9,
            K::KP_ENTER, K::KP_PLUS, K::KP_MINUS,
            K::KP_MULTIPLY, K::KP_DIVIDE, K::KP_PERIOD,
        };

        for (auto key : k_Bindable) {
            if (Util::Input::IsKeyDown(key)) return key;
        }
        return Util::Keycode::UNKNOWN;
    }

    bool IsMouseHovering(const Util::GameObject& obj) {
    const glm::vec2 mousePos   = Util::Input::GetCursorPosition();
    const glm::vec2 myPos      = obj.m_Transform.translation;
    const glm::vec2 mySize     = obj.GetScaledSize();
    const float     halfWidth  = mySize.x / 2.0f;
    const float     halfHeight = mySize.y / 2.0f;

    return (mousePos.x >= myPos.x - halfWidth  &&
            mousePos.x <= myPos.x + halfWidth  &&
            mousePos.y >= myPos.y - halfHeight &&
            mousePos.y <= myPos.y + halfHeight);
    }

    bool IsLeftClicked(const Util::GameObject& obj) {
        return Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB) && IsMouseHovering(obj);
    }
}