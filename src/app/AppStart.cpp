#include "app/App.hpp"
#include "services/audio/BGMPlayer.hpp"
#include "gameplay/assets/CatAssets.hpp"

#include "scenes/menu/TitleScene.hpp"
#include "scenes/menu/MenuScene.hpp"
#include "scenes/overlay/ExitConfirmScene.hpp"
#include "scenes/menu/OptionMenuScene.hpp"
#include "scenes/menu/KeyboardConfigScene.hpp"
#include "scenes/play/LocalPlayScene.hpp"
#include "scenes/play/LocalPlayGameScene.hpp"
#include "scenes/level/LevelSelectScene.hpp"
#include "scenes/level/LevelExitScene.hpp"
#include "scenes/level/LevelOneScene.hpp"
#include "scenes/level/LevelTwoScene.hpp"
#include "scenes/level/LevelThreeScene.hpp"
#include "core/scene/SceneId.hpp"

#include "Util/Logger.hpp"
#include <array>
#include <cmath>

using namespace std;

void App::Start() {
    LOG_TRACE("Start");

    m_GlobalActors = make_unique<GlobalActors>(m_Root);
    m_SessionState = make_unique<SessionState>();
    m_SceneManager = make_unique<SceneManager>();

    auto background = make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/white_background.png");
    background->SetZIndex(-10);
    background->SetScale({100.0f, 100.0f});
    m_GlobalActors->SetBackground(background);
    m_Root.AddChild(m_GlobalActors->Background());

    auto floor = make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/background_floor.png");
    floor->SetZIndex(0);
    floor->SetPosition({0.0f, -340.0f});
    m_GlobalActors->SetFloor(floor);
    m_Root.AddChild(m_GlobalActors->Floor());

    auto header = make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/header.png");
    header->SetZIndex(0);
    header->SetPosition({0.0f, 135.0f});
    m_GlobalActors->SetHeader(header);
    m_Root.AddChild(m_GlobalActors->Header());

    auto door = make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/door_close.png");
    door->SetScale({0.148f, 0.161f});
    door->SetZIndex(5.0f);
    m_GlobalActors->SetDoor(door);
    {
        const float floorY     = m_GlobalActors->Floor()->GetPosition().y;
        const float floorHalfH = m_GlobalActors->Floor()->GetScaledSize().y / 2.0f;
        const float doorHalfH  = m_GlobalActors->Door()->GetScaledSize().y / 2.0f;
        m_GlobalActors->Door()->SetPosition({0.0f, floorY + floorHalfH + doorHalfH});
    }
    m_Root.AddChild(m_GlobalActors->Door());


    m_BgmPlayer = make_shared<BGMPlayer>();
    m_AudioService = make_unique<AudioService>(m_BgmPlayer);
    m_AudioService->PlayBgm();

    m_ThemeService = make_unique<VisualThemeService>(m_GlobalActors->Background());

    auto& startupCats = m_GlobalActors->StartupCats();
    startupCats.clear();
    startupCats.reserve(m_GlobalActors->CatColorOrder().size());

    for (int i = 0; i < static_cast<int>(m_GlobalActors->CatColorOrder().size()); ++i) {
        Util::Keycode left  = Util::Keycode::UNKNOWN;
        Util::Keycode right = Util::Keycode::UNKNOWN;
        Util::Keycode jump  = Util::Keycode::UNKNOWN;

        if (i == 0) {
            left  = Util::Keycode::A;
            right = Util::Keycode::D;
            jump  = Util::Keycode::W;
        } else if (i == 1) {
            left  = Util::Keycode::LEFT;
            right = Util::Keycode::RIGHT;
            jump  = Util::Keycode::UP;
        }

        auto cat = make_shared<PlayerCat>(
            CatAssets::BuildFullAnimPaths(m_GlobalActors->CatColorOrder()[static_cast<size_t>(i)]),
            left, right, jump);
        cat->SetZIndex(20.0f + static_cast<float>(i) * 0.01f);
        cat->SetInputEnabled(i < 2);
        m_Root.AddChild(cat);
        startupCats.push_back(cat);
    }

    const float floorY      = m_GlobalActors->Floor()->GetPosition().y;
    const float floorHalfH  = m_GlobalActors->Floor()->GetScaledSize().y / 2.0f;

    constexpr float spacing     = 75.0f;
    constexpr float centerBlank = spacing * 2.0f;

    for (int i = 0; i < static_cast<int>(startupCats.size()); ++i) {
        auto& cat = startupCats[i];

        const float charHalfH   = cat->GetScaledSize().y / 2.0f;
        const float standOffset = floorHalfH + charHalfH;
        const float spawnY      = floorY + standOffset + 100.0f;

        const bool  isLeftSide = (i % 2 == 0);
        const int   sideLayer  = i / 2;
        const float x = isLeftSide
                            ? (-centerBlank * 0.5f - static_cast<float>(sideLayer) * spacing)
                            : ( centerBlank * 0.5f + static_cast<float>(sideLayer) * spacing);

        cat->SetPosition({x, spawnY});
        cat->SetZIndex(15.0f + static_cast<float>(i) * 0.01f);

        cat->SetFacingByDirection(isLeftSide ? 1 : -1);
    }

    SceneServices services{*m_AudioService, *m_ThemeService, *m_SessionState, *m_GlobalActors};

    auto titleScene = make_unique<TitleScene>(services);

    auto menuScene = make_unique<MenuScene>(services);

    auto exitConfirmScene = make_unique<ExitConfirmScene>(services);

    auto optionMenuScene = make_unique<OptionMenuScene>(services);

    auto kbConfigScene = make_unique<KeyboardConfigScene>(services);

    auto localPlayScene = make_unique<LocalPlayScene>(services);

    auto localPlayGameScene = make_unique<LocalPlayGameScene>(services);

    auto levelSelectScene = make_unique<LevelSelectScene>(services);
    levelSelectScene->SetLevelSceneId(0, SceneId::Level01);
    levelSelectScene->SetLevelSceneId(1, SceneId::Level02);
    levelSelectScene->SetLevelSceneId(2, SceneId::Level03);

    auto levelExitScene = make_unique<LevelExitScene>(services);

    auto levelOneScene = make_unique<LevelOneScene>(services);
    auto levelTwoScene = make_unique<LevelTwoScene>(services);
    auto levelThreeScene = make_unique<LevelThreeScene>(services);

    m_SceneManager->Register(SceneId::Title, std::move(titleScene));
    m_SceneManager->Register(SceneId::Menu, std::move(menuScene));
    m_SceneManager->Register(SceneId::ExitConfirm, std::move(exitConfirmScene));
    m_SceneManager->Register(SceneId::OptionMenu, std::move(optionMenuScene));
    m_SceneManager->Register(SceneId::KeyboardConfig, std::move(kbConfigScene));
    m_SceneManager->Register(SceneId::LocalPlay, std::move(localPlayScene));
    m_SceneManager->Register(SceneId::LocalPlayGame, std::move(localPlayGameScene));
    m_SceneManager->Register(SceneId::LevelSelect, std::move(levelSelectScene));
    m_SceneManager->Register(SceneId::LevelExit, std::move(levelExitScene));
    m_SceneManager->Register(SceneId::Level01, std::move(levelOneScene));
    m_SceneManager->Register(SceneId::Level02, std::move(levelTwoScene));
    m_SceneManager->Register(SceneId::Level03, std::move(levelThreeScene));

    m_SceneManager->GoTo(SceneId::Title);
    m_CurrentState = State::UPDATE;
}

