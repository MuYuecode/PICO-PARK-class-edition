//
// Created by cody2 on 2026/3/24.
//

#include "LevelSelectScene.hpp"
#include "LocalPlayGameScene.hpp"
#include "AppUtil.hpp"
#include "Util/Input.hpp"
#include "Util/Logger.hpp"

const Util::Color LevelSelectScene::kBlack  = Util::Color::FromRGB(  0,   0,   0, 255);
const Util::Color LevelSelectScene::kOrange = Util::Color::FromRGB(255, 140,   0, 255);
const Util::Color LevelSelectScene::kGray   = Util::Color::FromRGB(150, 150, 150, 255);

LevelSelectScene::LevelSelectScene(GameContext& ctx,
                                   LocalPlayGameScene* localPlayGameScene)
    : Scene(ctx)
    , m_LocalPlayGameScene(localPlayGameScene)
{
    m_SelectorFrame = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/level_select_frame.png");
    m_SelectorFrame->SetZIndex(32);

    m_TitleText = std::make_shared<GameText>("LEVEL 1", 78, kOrange);
    m_TitleText->SetZIndex(35);
    m_TitleText->SetPosition({0.0f, 270.0f});

    for (int i = 0; i < LEVEL_COUNT; ++i) {
        glm::vec2 cp = CellPos(i);

        m_LevelCover[i] = std::make_shared<Character>(GA_RESOURCE_DIR + imagePaths[i]);
        m_LevelCover[i]->SetZIndex(35);
        m_LevelCover[i]->SetPosition(cp);

        m_Crown[i] = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/Crown.png");
        m_Crown[i]->SetZIndex(36);
        m_Crown[i]->SetPosition({cp.x - 36.0f, cp.y + 32.0f});
        m_Crown[i]->SetVisible(false);
    }

    m_BestTimeText = std::make_shared<GameText>(
        "2 PLAYERS BEST TIME: --:--.--", 48, kOrange);
    m_BestTimeText->SetZIndex(35);
    m_BestTimeText->SetPosition({0.0f, -278.0f});
}

glm::vec2 LevelSelectScene::CellPos(int idx) {
    int col = (idx < 5) ? idx-2 : idx-7 ;
    int row = idx / COLS;

    return {
        GRID_MID_X + static_cast<float>(col) * CELL_W,
        GRID_TOP_Y  - static_cast<float>(row) * CELL_H
    };
}

void LevelSelectScene::OnEnter() {
    LOG_INFO("LevelSelectScene::OnEnter  players={}  selectedIdx={}",
             m_Ctx.SelectedPlayerCount, m_SelectedIdx);

    m_LevelData = {};
    SaveManager::LoadLevelData(m_LevelData);

    m_SelectedIdx = 0;

    m_Ctx.Root.AddChild(m_SelectorFrame);
    m_Ctx.Root.AddChild(m_TitleText);
    m_Ctx.Root.AddChild(m_BestTimeText);
    for (int i = 0; i < LEVEL_COUNT; ++i) {
        m_Ctx.Root.AddChild(m_LevelCover[i]);
        m_Ctx.Root.AddChild(m_Crown[i]);
    }

    m_Ctx.Header->SetVisible(false);
    m_Ctx.Header->SetVisible(false);
    if (m_Ctx.Floor != nullptr) m_Ctx.Floor->SetVisible(false);
    if (m_Ctx.Door != nullptr)  m_Ctx.Door->SetVisible(false);
    for (auto& cat : m_Ctx.StartupCats) {
        if (cat != nullptr) cat->SetVisible(false);
    }

    UpdateSelectorPos();
    UpdateTitleText();
    UpdateBestTimeText();
    UpdateCrowns();
}

void LevelSelectScene::OnExit() {
    LOG_INFO("LevelSelectScene::OnExit");

    m_Ctx.Root.RemoveChild(m_SelectorFrame);
    m_Ctx.Root.RemoveChild(m_TitleText);
    m_Ctx.Root.RemoveChild(m_BestTimeText);
    for (int i = 0; i < LEVEL_COUNT; ++i) {
        m_Ctx.Root.RemoveChild(m_LevelCover[i]);
        m_Ctx.Root.RemoveChild(m_Crown[i]);
    }

    m_Ctx.Header->SetVisible(true);
    if (m_Ctx.Floor != nullptr) m_Ctx.Floor->SetVisible(true);
    if (m_Ctx.Door != nullptr)  m_Ctx.Door->SetVisible(true);
    for (auto& cat : m_Ctx.StartupCats) {
        if (cat != nullptr) cat->SetVisible(true);
    }
}

Scene* LevelSelectScene::Update() {
    using ip = Util::Input ;
    using k  = Util::Keycode;
    if (ip::IsKeyDown(k::ESCAPE)) {
        LOG_INFO("LevelSelectScene: ESC → LocalPlayGameScene");
        return m_LocalPlayGameScene;
    }

    int  newIdx     = m_SelectedIdx;
    bool idxChanged = false;

    if (ip::IsKeyDown(k::LEFT) ||
        ip::IsKeyDown(k::A)) {
        if (newIdx > 0) { --newIdx; idxChanged = true; }
    }
    else if (ip::IsKeyDown(k::RIGHT) ||
             ip::IsKeyDown(k::D)) {
        if (newIdx < LEVEL_COUNT - 1) { ++newIdx; idxChanged = true; }
    }
    else if (ip::IsKeyDown(k::UP) ||
             ip::IsKeyDown(k::W)) {
        if (newIdx >= COLS) { newIdx -= COLS; idxChanged = true; }
    }
    else if (ip::IsKeyDown(k::DOWN) ||
             ip::IsKeyDown(k::S)) {
        if (newIdx + COLS < LEVEL_COUNT) { newIdx += COLS; idxChanged = true; }
    }

    for (int i = 0; i < LEVEL_COUNT; ++i) {
        if (AppUtil::IsMouseHovering(*m_LevelCover[i]) && newIdx != i) {
            newIdx     = i;
            idxChanged = true;
            break;
        }
    }

    if (idxChanged) {
        m_SelectedIdx = newIdx;
        UpdateSelectorPos();
        UpdateTitleText();
        UpdateBestTimeText();
    }

    bool confirmed = ip::IsKeyDown(k::RETURN);

    if (!confirmed) {
        for (int i = 0; i < LEVEL_COUNT; ++i) {
            if (AppUtil::IsLeftClicked(*m_LevelCover[i])) {
                m_SelectedIdx = i;
                UpdateSelectorPos();
                UpdateTitleText();
                UpdateBestTimeText();
                confirmed = true;
                break;
            }
        }
    }

    if (confirmed) {
    //     Scene* target = m_LevelScenes[m_SelectedIdx];
    //     if (target != nullptr) {
    //         LOG_INFO("LevelSelectScene: entering Level {}", m_SelectedIdx + 1);
    //         return target;
    //     }
    //     // 關卡尚未實作：留在此場景(可在此加提示文字)
    //     LOG_INFO("LevelSelectScene: Level {} not implemented yet", m_SelectedIdx + 1);
    }

    return nullptr;
}

void LevelSelectScene::UpdateSelectorPos() const {
    m_SelectorFrame->SetPosition(CellPos(m_SelectedIdx));
}

void LevelSelectScene::UpdateTitleText() const {
    m_TitleText->SetText("LEVEL " + std::to_string(m_SelectedIdx + 1));
}

void LevelSelectScene::UpdateBestTimeText() const {
    int   p    = m_Ctx.SelectedPlayerCount;
    float best = -1.0f;
    if (p >= 2 && p <= 8) {
        best = m_LevelData[m_SelectedIdx].bestTimes[p];
    }
    m_BestTimeText->SetText(
        std::to_string(p) + " PLAYERS BEST TIME: " + SaveManager::FormatTime(best));
}

void LevelSelectScene::UpdateCrowns() const {
    for (int i = 0; i < LEVEL_COUNT; ++i) {
        m_Crown[i]->SetVisible(m_LevelData[i].completed);
    }
}