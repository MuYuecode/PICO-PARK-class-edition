#include "scenes/TitleScene.hpp"

#include "gameplay/BoundaryFactory.hpp"

#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"

TitleScene::TitleScene(SceneServices services)
    : Scene(services)
{
    const Util::Color orange(254, 133, 78, 255);

    m_TitleSub = std::make_shared<GameText>("- CLASSIC EDITED -", 46, orange);
    m_TitleSub->SetZIndex(0);
    m_TitleSub->SetPosition({0.f, -22.5f});

    m_PressEnterText = std::make_shared<GameText>("PRESS ENTER KEY", 66, orange);
    m_PressEnterText->SetZIndex(0);
    m_PressEnterText->SetPosition({0.f, -155.f});
}

void TitleScene::SetupStaticBoundaries() {
    float floorSurfaceY = -360.f;  // fallback if Floor is null
    if (m_Actors.Floor() != nullptr) {
        floorSurfaceY = m_Actors.Floor()->GetPosition().y
                      + m_Actors.Floor()->GetScaledSize().y * 0.5f;
    }

    BoundaryFactory::AddStaticRoomBoundaries(
        m_World,
        floorSurfaceY,
        std::nullopt,
        LevelGeometryPreset::kSharedMenuLikeNoCeiling);
}


void TitleScene::OnEnter() {
    LOG_INFO("TitleScene::OnEnter");

    if (m_Actors.Header() != nullptr) {
        m_Actors.Header()->SetVisible(true);
    }
    if (m_Actors.Floor() != nullptr) {
        m_Actors.Floor()->SetVisible(true);
    }
    if (m_Actors.Door() != nullptr) {
        m_Actors.Door()->SetVisible(true);
        m_Actors.Door()->SetImage(GA_RESOURCE_DIR "/Image/Background/door_close.png");

        if (m_Actors.Floor() != nullptr) {
            const float floorY = m_Actors.Floor()->GetPosition().y;
            const float floorHalfH = m_Actors.Floor()->GetScaledSize().y * 0.5f;
            const float doorHalfH = m_Actors.Door()->GetScaledSize().y * 0.5f;
            m_Actors.Door()->SetPosition({0.0f, floorY + floorHalfH + doorHalfH});
        }
    }
    if (m_Actors.TestBox() != nullptr) {
        m_Actors.TestBox()->SetVisible(true);
        if (m_Actors.TestBox()->GetTextObject() != nullptr) {
            m_Actors.TestBox()->GetTextObject()->SetVisible(true);
        }
    }
    for (auto& cat : m_Actors.StartupCats()) {
        if (cat != nullptr) {
            cat->SetVisible(true);
        }
    }

    m_Actors.Root().AddChild(m_TitleSub);
    m_Actors.Root().AddChild(m_PressEnterText);

    m_FlashTimer = 0.f;
    m_PressEnterText->SetVisible(true);

    // Clear any leftover state from a previous visit.
    m_World.Clear();

    // Register the decorative startup cats.
    for (int i = 0; i < static_cast<int>(m_Actors.StartupCats().size()); ++i) {
        auto& cat = m_Actors.StartupCats()[i];
        if (cat == nullptr) continue;

        cat->SetInputEnabled(i < 2);
        cat->SetCatAnimState(CatAnimState::STAND);

        m_World.Register(cat);
    }

    // Register the test pushable box if present.
    if (m_Actors.TestBox() != nullptr) {
        m_Actors.TestBox()->SetPushQuery(&m_World);
        m_World.Register(m_Actors.TestBox());
    }

    // Register immovable boundary geometry derived from the visual Floor.
    SetupStaticBoundaries();
}

void TitleScene::OnExit() {
    LOG_INFO("TitleScene::OnExit");

    m_Actors.Root().RemoveChild(m_TitleSub);
    m_Actors.Root().RemoveChild(m_PressEnterText);

    for (auto& cat : m_Actors.StartupCats()) {
        if (cat != nullptr) cat->SetInputEnabled(false);
    }

    m_World.Clear();
}

void TitleScene::Update() {
    m_FlashTimer += Util::Time::GetDeltaTimeMs();
    if (m_FlashTimer >= 1000.f) {
        m_PressEnterText->SetVisible(!m_PressEnterText->GetVisibility());
        m_FlashTimer = 0.f;
    }

    for (auto& cat : m_Actors.StartupCats()) {
        if (cat == nullptr) continue;

        cat->SetMoveDir(0);  // default: no horizontal input

        if (!cat->GetInputEnabled()) continue;

        const Util::Keycode lk = cat->GetLeftKey();
        const Util::Keycode rk = cat->GetRightKey();
        const Util::Keycode jk = cat->GetJumpKey();

        const bool goLeft  = (lk != Util::Keycode::UNKNOWN) &&
                              Util::Input::IsKeyPressed(lk);
        const bool goRight = (rk != Util::Keycode::UNKNOWN) &&
                              Util::Input::IsKeyPressed(rk);

        if      (goLeft  && !goRight) cat->SetMoveDir(-1);
        else if (goRight && !goLeft)  cat->SetMoveDir( 1);

        const bool wantJump = (jk != Util::Keycode::UNKNOWN) &&
                               Util::Input::IsKeyDown(jk);

        if (cat->IsGrounded() && wantJump) {
            cat->Jump();
        }
    }

    m_World.Update();

    if (Util::Input::IsKeyDown(Util::Keycode::RETURN)) {
        LOG_INFO("TitleScene: ENTER pressed -> MenuScene");
        RequestSceneOp({SceneOpType::ClearToAndGoTo, SceneId::Menu});
        return;
    }
}
