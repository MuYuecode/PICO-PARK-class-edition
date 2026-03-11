#include "App.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

void App::Update() {

    // header image
    // if (m_Header->GetVisibility() && Util::Input::IsKeyPressed(Util::Keycode::RETURN)) {
    //     m_Header->SetVisible(false);
    // }

    // title
    if (m_TitleSub->GetVisibility() && Util::Input::IsKeyPressed(Util::Keycode::RETURN)) {
        m_TitleSub->SetVisible(false);
    }
    if (!m_TitleSub->GetVisibility() && Util::Input::IsKeyPressed(Util::Keycode::ESCAPE)) {
        m_TitleSub->SetVisible(false);
    }


    if (Util::Input::IsKeyPressed(Util::Keycode::ESCAPE) || Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }

    if (m_EnterDown) {
        if (!Util::Input::IsKeyPressed(Util::Keycode::RETURN)) {
            ValidTask();
        }
    }
    m_EnterDown = Util::Input::IsKeyPressed(Util::Keycode::RETURN);

    // ==========================================
    // charmove
    glm::vec2 catPos = m_BlueCat->GetPosition();

    float moveSpeed = 5.0f;
    if (Util::Input::IsKeyPressed(Util::Keycode::D)) {
        LOG_DEBUG("press D");
        catPos.x += moveSpeed;
    }
    else if (Util::Input::IsKeyPressed(Util::Keycode::A)) {
        LOG_DEBUG("press A");
        catPos.x -= moveSpeed;
    }


    // 簡易重力
    float gravity = 5.0f;
    catPos.y -= gravity;

    // 站立判定 (確保你在 include/Character.hpp 已經加入了 IsStandingOn 方法)
    float standOffset = 58.5f;
    if (m_BlueCat->IsStandingOn(m_Floor, standOffset)) {
        catPos.y = m_Floor->GetPosition().y + standOffset;
    }

    m_BlueCat->SetPosition(catPos);
    // ==========================================

    m_Root.Update();
}