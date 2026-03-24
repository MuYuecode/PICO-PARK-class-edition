//
// Created by cody2 on 2026/3/24.
//

#include "LevelSelectScene.hpp"
#include "LocalPlayGameScene.hpp"
#include "AppUtil.hpp"
#include "Util/Input.hpp"
#include "Util/Logger.hpp"

//  靜態顏色常數
const Util::Color LevelSelectScene::kBlack  = Util::Color::FromRGB(  0,   0,   0, 255);
const Util::Color LevelSelectScene::kOrange = Util::Color::FromRGB(255, 140,   0, 255);
const Util::Color LevelSelectScene::kGray   = Util::Color::FromRGB(150, 150, 150, 255);

//  建構子
LevelSelectScene::LevelSelectScene(GameContext& ctx,
                                   LocalPlayGameScene* localPlayGameScene)
    : Scene(ctx)
    , m_LocalPlayGameScene(localPlayGameScene)
{
    // m_LevelScenes.fill(nullptr);

    // 橘色選擇框：複用 Option_Choice_Frame，用 scale 調整成方形輪廓
    // Choice_Frame 原始尺寸約 367 × 50；目標格子約 100 × 100，
    // scale ≈ {100/367, 100/50} = {0.27, 2.0}(可依實際圖片微調)
    m_SelectorFrame = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/level_select_frame.png");
    m_SelectorFrame->SetZIndex(32);

    // 標題文字 "LEVEL N"
    m_TitleText = std::make_shared<GameText>("LEVEL 1", 78, kOrange);
    m_TitleText->SetZIndex(35);
    m_TitleText->SetPosition({0.0f, 270.0f});

    // 10 個關卡格
    for (int i = 0; i < LEVEL_COUNT; ++i) {
        glm::vec2 cp = CellPos(i);

        m_LevelCover[i] = std::make_shared<Character>(GA_RESOURCE_DIR + imagePaths[i]);
        m_LevelCover[i]->SetZIndex(35);
        m_LevelCover[i]->SetPosition(cp);

        // 皇冠標記(使用 * 號；TerminusTTF Bold 字型含此字元)
        // 位置偏至格子左上方
        m_Crown[i] = std::make_shared<Character>(GA_RESOURCE_DIR "/Image/Level_Cover/Crown.png");
        m_Crown[i]->SetZIndex(36);
        m_Crown[i]->SetPosition({cp.x - 36.0f, cp.y + 32.0f});
        m_Crown[i]->SetVisible(false);
    }

    // 底部最佳時間文字
    m_BestTimeText = std::make_shared<GameText>(
        "2 PLAYERS BEST TIME: --:--.--", 48, kOrange);
    m_BestTimeText->SetZIndex(35);
    m_BestTimeText->SetPosition({0.0f, -278.0f});
}

//  格子中心座標計算
glm::vec2 LevelSelectScene::CellPos(int idx) {
    int col = (idx < 5) ? idx-2 : idx-7 ;
    int row = idx / COLS; // Explicit integer division

    return {
        GRID_MID_X + static_cast<float>(col) * CELL_W,
        GRID_TOP_Y  - static_cast<float>(row) * CELL_H
    };
}

//  生命週期
void LevelSelectScene::OnEnter() {
    LOG_INFO("LevelSelectScene::OnEnter  players={}  selectedIdx={}",
             m_Ctx.SelectedPlayerCount, m_SelectedIdx);

    // 每次進入都重新讀取關卡存檔(確保反映上一局的新紀錄)
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

    // Header 在此場景中隱藏(關卡選擇有自己的佈局)
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

    // 還原全域物件可見性
    m_Ctx.Header->SetVisible(true);
    if (m_Ctx.Floor != nullptr) m_Ctx.Floor->SetVisible(true);
    if (m_Ctx.Door != nullptr)  m_Ctx.Door->SetVisible(true);
    for (auto& cat : m_Ctx.StartupCats) {
        if (cat != nullptr) cat->SetVisible(true);
    }
}

Scene* LevelSelectScene::Update() {
    //  ESC → 回到 LocalPlayGameScene
    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
        LOG_INFO("LevelSelectScene: ESC → LocalPlayGameScene");
        return m_LocalPlayGameScene;
    }

    //  鍵盤移動選擇框
    int  newIdx     = m_SelectedIdx;
    bool idxChanged = false;

    if (Util::Input::IsKeyDown(Util::Keycode::LEFT) ||
        Util::Input::IsKeyDown(Util::Keycode::A)) {
        if (newIdx > 0) { --newIdx; idxChanged = true; }
    }
    else if (Util::Input::IsKeyDown(Util::Keycode::RIGHT) ||
             Util::Input::IsKeyDown(Util::Keycode::D)) {
        if (newIdx < LEVEL_COUNT - 1) { ++newIdx; idxChanged = true; }
    }
    else if (Util::Input::IsKeyDown(Util::Keycode::UP) ||
             Util::Input::IsKeyDown(Util::Keycode::W)) {
        if (newIdx >= COLS) { newIdx -= COLS; idxChanged = true; }
    }
    else if (Util::Input::IsKeyDown(Util::Keycode::DOWN) ||
             Util::Input::IsKeyDown(Util::Keycode::S)) {
        if (newIdx + COLS < LEVEL_COUNT) { newIdx += COLS; idxChanged = true; }
    }

    //  滑鼠懸停自動切換
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

    //  ENTER / 左鍵點擊格子 → 進入關卡
    bool confirmed = Util::Input::IsKeyDown(Util::Keycode::RETURN);

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

//  私有輔助
void LevelSelectScene::UpdateSelectorPos() const {
    m_SelectorFrame->SetPosition(CellPos(m_SelectedIdx));
}

void LevelSelectScene::UpdateTitleText() const {
    m_TitleText->SetText("LEVEL " + std::to_string(m_SelectedIdx + 1));
}

void LevelSelectScene::UpdateBestTimeText() const {
    int   p    = m_Ctx.SelectedPlayerCount;
    // SelectedPlayerCount 範圍 2~8；safeguard
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