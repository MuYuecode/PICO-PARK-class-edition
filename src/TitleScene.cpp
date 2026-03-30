#include "TitleScene.hpp"

#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"

TitleScene::TitleScene(GameContext& ctx)
    : Scene(ctx)
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
    constexpr float kWallHalfW  = 50.f;   // thickness of left/right walls
    constexpr float kWallHalfH  = 400.f;  // height of left/right wall colliders
    constexpr float kFloorHalfH = 40.f;   // thickness of the floor collider

    float floorSurfaceY = -360.f;  // fallback if Floor is null
    if (m_Ctx.Floor != nullptr) {
        floorSurfaceY = m_Ctx.Floor->GetPosition().y
                      + m_Ctx.Floor->GetScaledSize().y * 0.5f;
    }

    // Floor: center is below the surface, top edge sits at floorSurfaceY.
    m_World.AddStaticBoundary(
        {0.f, floorSurfaceY - kFloorHalfH},
        {640.f + kWallHalfW, kFloorHalfH});

    // Left boundary wall.
    m_World.AddStaticBoundary(
        {-(640.f + kWallHalfW), 0.f},
        {kWallHalfW, kWallHalfH});

    // Right boundary wall.
    m_World.AddStaticBoundary(
        { (640.f + kWallHalfW), 0.f},
        {kWallHalfW, kWallHalfH});
}


void TitleScene::OnEnter() {
    LOG_INFO("TitleScene::OnEnter");

    m_Ctx.Header->SetVisible(true);
    m_Ctx.Root.AddChild(m_TitleSub);
    m_Ctx.Root.AddChild(m_PressEnterText);

    m_FlashTimer = 0.f;
    m_PressEnterText->SetVisible(true);

    // Clear any leftover state from a previous visit.
    m_World.Clear();

    // Register the decorative startup cats.
    for (int i = 0; i < static_cast<int>(m_Ctx.StartupCats.size()); ++i) {
        auto& cat = m_Ctx.StartupCats[i];
        if (cat == nullptr) continue;

        cat->SetInputEnabled(i < 2);
        cat->SetCatAnimState(CatAnimState::STAND);

        m_World.Register(cat);
    }

    // Register the test pushable box if present.
    if (m_Ctx.TestBox != nullptr) {
        m_Ctx.TestBox->SetWorld(&m_World);
        m_World.Register(m_Ctx.TestBox);
    }

    // Register immovable boundary geometry derived from the visual Floor.
    SetupStaticBoundaries();
}

void TitleScene::OnExit() {
    LOG_INFO("TitleScene::OnExit");

    m_Ctx.Root.RemoveChild(m_TitleSub);
    m_Ctx.Root.RemoveChild(m_PressEnterText);

    for (auto& cat : m_Ctx.StartupCats) {
        if (cat != nullptr) cat->SetInputEnabled(false);
    }

    m_World.Clear();
}

SceneId TitleScene::Update() {
    m_FlashTimer += Util::Time::GetDeltaTimeMs();
    if (m_FlashTimer >= 1000.f) {
        m_PressEnterText->SetVisible(!m_PressEnterText->GetVisibility());
        m_FlashTimer = 0.f;
    }

    for (auto& cat : m_Ctx.StartupCats) {
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
        return SceneId::Menu;
    }
    return SceneId::None;
}