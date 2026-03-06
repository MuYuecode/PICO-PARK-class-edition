#include "App.hpp"

#include "Util/Input.hpp"
#include "Util/Keycode.hpp"

void App::Update() {

    // TODO: Add your own logics to finish the tasks in README.md

    // get position
    auto pos = m_Giraffe->GetPosition();

    // detect wasd or arrow keys to move
    if (Util::Input::IsKeyPressed(Util::Keycode::W) || Util::Input::IsKeyPressed(Util::Keycode::UP)) pos.y += 5.0f;
    if (Util::Input::IsKeyPressed(Util::Keycode::S) || Util::Input::IsKeyPressed(Util::Keycode::DOWN)) pos.y -= 5.0f;
    if (Util::Input::IsKeyPressed(Util::Keycode::A) || Util::Input::IsKeyPressed(Util::Keycode::LEFT)) pos.x -= 5.0f;
    if (Util::Input::IsKeyPressed(Util::Keycode::D) || Util::Input::IsKeyPressed(Util::Keycode::RIGHT)) pos.x += 5.0f;

    // set new position
    m_Giraffe->SetPosition(pos);

    // hide chest (set invisible)
    if (m_Phase == Phase::COLLIDE_DETECTION && m_Giraffe->IfCollides(m_Chest)) {
        m_Chest->SetVisible(false);
    }

    // bee animation
    if (m_Phase == Phase::BEE_ANIMATION) {
        m_Bee->SetLooping(true); // set looping to true
        m_Bee->Play();           // start to play
    }

    // open door
    if (m_Phase == Phase::OPEN_THE_DOORS) {
        for (auto& door : m_Doors) {
            if (m_Giraffe->IfCollides(door)) {
                // change to open image when touching
                door->SetImage(GA_RESOURCE_DIR"/Image/Character/door_open.png");
            }
        }
    }

    // counting down ball
    if (m_Phase == Phase::COUNTDOWN) {
        if (!m_Ball->IsPlaying() && !m_Ball->IfAnimationEnds()) {
            m_Ball->SetVisible(true);
            m_Ball->SetLooping(false); // 倒數完顯示 OK 就不再輪迴
            m_Ball->Play();
        }
    }

    /*
     *  Do not touch the code below as they serve the purpose for validating the tasks,
     *  rendering the frame, and exiting the game.
    */

    if (Util::Input::IsKeyPressed(Util::Keycode::ESCAPE) || Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }

    if (m_EnterDown) {
        if (!Util::Input::IsKeyPressed(Util::Keycode::RETURN)) {
            ValidTask();
        }
    }
    m_EnterDown = Util::Input::IsKeyPressed(Util::Keycode::RETURN);

    m_Root.Update();
}
