#include "MenuScene.hpp"
#include "ExitConfirmScene.hpp"
#include "OptionMenuScene.hpp"
#include "LocalPlayScene.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

using ip = Util::Input;
using k  = Util::Keycode;

MenuScene::MenuScene(GameContext&      ctx,
                     Scene*            titleScene,
                     ExitConfirmScene* exitConfirmScene,
                     OptionMenuScene*  optionScene,
                     LocalPlayScene*   localPlayScene)
    : Scene(ctx)
    , m_TitleScene(titleScene)
    , m_ExitConfirmScene(exitConfirmScene)
    , m_OptionScene(optionScene)
    , m_LocalPlayScene(localPlayScene)
{
    const Util::Color black = Util::Color::FromRGB(0, 0, 0, 255);

    m_MenuFrame = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/Menu_Frame.png");
    m_MenuFrame->SetZIndex(10);
    m_MenuFrame->SetPosition({0.f, -105.f});

    m_ExitGameButton = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Button/ExitGameButton.png");
    m_ExitGameButton->SetZIndex(30);
    m_ExitGameButton->SetPosition({331.f, -14.f});

    m_blue_cat_run_img = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Character/blue_cat/blue_cat_run_img.png");
    m_blue_cat_run_img->SetZIndex(10);
    m_blue_cat_run_img->SetScale({1.2f, 1.2f});
    m_blue_cat_run_img->SetPosition({0.f, -74.f});

    m_LeftTriButton = std::make_shared<UITriangleButton>(
        GA_RESOURCE_DIR "/Image/Button/Left_Tri_Button.png",
        GA_RESOURCE_DIR "/Image/Button/Left_Tri_Button_Full.png");
    m_LeftTriButton->SetZIndex(15);
    m_LeftTriButton->SetPosition({-305.f, -153.f});

    m_RightTriButton = std::make_shared<UITriangleButton>(
        GA_RESOURCE_DIR "/Image/Button/Right_Tri_Button.png",
        GA_RESOURCE_DIR "/Image/Button/Right_Tri_Button_Full.png");
    m_RightTriButton->SetZIndex(15);
    m_RightTriButton->SetPosition({305.f, -153.f});

    m_ExitGameText = std::make_shared<GameText>("EXIT GAME", 65, black);
    m_ExitGameText->SetPosition({0.f, -153.f});

    m_OptionText = std::make_shared<GameText>("OPTION", 65, black);
    m_OptionText->SetPosition({0.f, -153.f});

    m_LocalPlayText = std::make_shared<GameText>(" LOCAL PLAY MODE", 65, black);
    m_LocalPlayText->SetPosition({0.f, -153.f});
}

void MenuScene::SetupStaticBoundaries() {
    constexpr float kWallHalfW  = 50.f;
    constexpr float kWallHalfH  = 400.f;
    constexpr float kFloorHalfH = 40.f;

    float floorSurfaceY = -360.f;
    if (m_Ctx.Floor != nullptr) {
        floorSurfaceY = m_Ctx.Floor->GetPosition().y
                      + m_Ctx.Floor->GetScaledSize().y * 0.5f;
    }

    // Floor collider: top edge sits at floorSurfaceY.
    m_World.AddStaticBoundary(
        {0.f, floorSurfaceY - kFloorHalfH},
        {640.f + kWallHalfW, kFloorHalfH});

    // Left wall.
    m_World.AddStaticBoundary(
        {-(640.f + kWallHalfW), 0.f},
        {kWallHalfW, kWallHalfH});

    // Right wall.
    m_World.AddStaticBoundary(
        { (640.f + kWallHalfW), 0.f},
        {kWallHalfW, kWallHalfH});
}

void MenuScene::OnEnter() {
    LOG_INFO("MenuScene::OnEnter  index={}", m_SelectedIndex);

    m_Ctx.Root.AddChild(m_MenuFrame);
    m_Ctx.Root.AddChild(m_ExitGameButton);
    m_Ctx.Root.AddChild(m_blue_cat_run_img);
    m_Ctx.Root.AddChild(m_LeftTriButton);
    m_Ctx.Root.AddChild(m_RightTriButton);
    m_Ctx.Root.AddChild(m_ExitGameText);
    m_Ctx.Root.AddChild(m_OptionText);
    m_Ctx.Root.AddChild(m_LocalPlayText);

    m_LeftTriButton->ResetState();
    m_RightTriButton->ResetState();

    m_LeftTriButton->SetVisible(true);
    m_RightTriButton->SetVisible(true);
    m_ExitGameButton->SetVisible(true);
    m_MenuFrame->SetVisible(true);

    m_World.Clear();

    for (auto& cat : m_Ctx.StartupCats) {
        if (cat == nullptr) continue;
        cat->SetInputEnabled(false);
        cat->SetMoveDir(0);
        m_World.Register(cat);
    }

    SetupStaticBoundaries();

    ShowCurrentOption();
}

void MenuScene::OnExit() {
    LOG_INFO("MenuScene::OnExit");

    m_Ctx.Root.RemoveChild(m_MenuFrame);
    m_Ctx.Root.RemoveChild(m_ExitGameButton);
    m_Ctx.Root.RemoveChild(m_blue_cat_run_img);
    m_Ctx.Root.RemoveChild(m_LeftTriButton);
    m_Ctx.Root.RemoveChild(m_RightTriButton);
    m_Ctx.Root.RemoveChild(m_ExitGameText);
    m_Ctx.Root.RemoveChild(m_OptionText);
    m_Ctx.Root.RemoveChild(m_LocalPlayText);

    m_World.Clear();
}

Scene* MenuScene::Update() {
    m_World.Update();

    m_LeftTriButton->UpdateButton();
    m_RightTriButton->UpdateButton();

    if (ip::IsKeyDown(k::ESCAPE) || m_ExitGameButton->IsLeftClicked()) {
        return m_TitleScene;
    }

    if (ip::IsKeyDown(k::A)) {
        HideAllOptions();
        m_SelectedIndex = (m_SelectedIndex + 2) % 3;
        m_LeftTriButton->Press(75.f);
        ShowCurrentOption();
        return nullptr;
    }

    if (ip::IsKeyDown(k::D)) {
        HideAllOptions();
        m_SelectedIndex = (m_SelectedIndex + 1) % 3;
        m_RightTriButton->Press(75.f);
        ShowCurrentOption();
        return nullptr;
    }

    if (ip::IsKeyDown(k::RETURN)) {
        switch (m_SelectedIndex) {
        case 0: if (m_ExitConfirmScene != nullptr) return m_ExitConfirmScene; break;
        case 1: if (m_OptionScene      != nullptr) return m_OptionScene;      break;
        case 2: if (m_LocalPlayScene   != nullptr) return m_LocalPlayScene;   break;
        default: break;
        }
    }

    return nullptr;
}

void MenuScene::ShowCurrentOption() const {
    m_ExitGameText->SetVisible(m_SelectedIndex == 0);
    m_OptionText->SetVisible(m_SelectedIndex == 1);
    m_LocalPlayText->SetVisible(m_SelectedIndex == 2);
}

void MenuScene::HideAllOptions() const {
    m_ExitGameText->SetVisible(false);
    m_OptionText->SetVisible(false);
    m_LocalPlayText->SetVisible(false);
}