//
// Created by cody2 on 2026/3/16.
//

#include "KeyboardConfigScene.hpp"
#include "OptionMenuScene.hpp"
#include "AppUtil.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

// 靜態常數
const Util::Color KeyboardConfigScene::k_Black = Util::Color::FromRGB(0,   0,   0,   255);
const Util::Color KeyboardConfigScene::k_Red   = Util::Color::FromRGB(220, 50,  50,  255);
const Util::Color KeyboardConfigScene::k_Grey  = Util::Color::FromRGB(150, 150, 150, 255);

// 1P 預設
const PlayerKeyConfig KeyboardConfigScene::k_Default1P = {
    Util::Keycode::W,
    Util::Keycode::S,
    Util::Keycode::A,
    Util::Keycode::D,
    Util::Keycode::W,
    Util::Keycode::ESCAPE,
    Util::Keycode::SPACE,
    Util::Keycode::RETURN,
    Util::Keycode::TAB
};

// 2P 預設
const PlayerKeyConfig KeyboardConfigScene::k_Default2P = {
    Util::Keycode::UP,
    Util::Keycode::DOWN,
    Util::Keycode::LEFT,
    Util::Keycode::RIGHT,
    Util::Keycode::UP,
    Util::Keycode::AC_BACK,
    Util::Keycode::RCTRL,
    Util::Keycode::UNKNOWN,   // MENU: -
    Util::Keycode::UNKNOWN    // SUB MENU: -
};

static constexpr std::array<Util::Keycode PlayerKeyConfig::*, 9> kBindFields = {
    &PlayerKeyConfig::up,
    &PlayerKeyConfig::down,
    &PlayerKeyConfig::left,
    &PlayerKeyConfig::right,
    &PlayerKeyConfig::jump,
    &PlayerKeyConfig::cancel,
    &PlayerKeyConfig::shot,
    &PlayerKeyConfig::menu,
    &PlayerKeyConfig::subMenu,
};

// PlayerKeyConfig::AllKeys
std::vector<Util::Keycode> PlayerKeyConfig::AllKeys() const {
    std::vector<Util::Keycode> result;
    for (auto k : {up, down, left, right, jump, cancel, shot, menu, subMenu}) {
        if (k != Util::Keycode::UNKNOWN) result.push_back(k);
    }
    return result;
}

KeyboardConfigScene::KeyboardConfigScene(GameContext& ctx,
                                         OptionMenuScene* optionScene,
                                         std::shared_ptr<Character> exitGameButton)
    : Scene(ctx)
    , m_ExitGameButton(std::move(exitGameButton))
    , m_OptionScene(optionScene)
{
    // 初始化預設設定
    m_Applied[0] = k_Default1P;
    m_Applied[1] = k_Default2P;
    // 3P-8P：全 UNKNOWN

    // 框架
    m_Frame = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/Option_Menu_Frame.png");
    m_Frame->SetZIndex(25);
    m_Frame->SetPosition({0.0f, -5.0f});

    // 選擇框
    m_ChoiceFrame = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/Option_Choice_Frame.png");
    m_ChoiceFrame->SetZIndex(30);
    m_ChoiceFrame->SetScale({0.46f, 1.0f}) ;

    // 橫線
    auto makeHLine = [&](float y) {
        auto line = std::make_shared<Character>(
            GA_RESOURCE_DIR "/Image/Background/Option_HLine.png");
        line->SetZIndex(35);
        line->SetPosition({0.0f, y});
        return line;
    };
    m_HLine1 = makeHLine(ROW_Y_HLINE1);
    m_HLine2 = makeHLine(ROW_Y_HLINE2);
    m_HLine3 = makeHLine(ROW_Y_HLINE3);

    m_TitleText = std::make_shared<GameText>("KEYBOARD CONFIG", 55, k_Black);
    m_TitleText->SetZIndex(35);
    m_TitleText->SetPosition({0.0f, ROW_Y_TITLE});

    m_PlayerLabel = std::make_shared<GameText>("PLAYER", 45, k_Black);
    m_PlayerLabel->SetZIndex(35);
    m_PlayerLabel->SetPosition({AppUtil::AlignLeft(*m_PlayerLabel, COL_LABEL_X), ROW_Y_PLAYER});

    m_PlayerLeftBtn = std::make_shared<UI_Triangle_Button>(
        GA_RESOURCE_DIR "/Image/Button/Left_Tri_Button.png",
        GA_RESOURCE_DIR "/Image/Button/Left_Tri_Button_Full.png");
    m_PlayerLeftBtn->SetZIndex(35);
    m_PlayerLeftBtn->SetPosition({COL_LEFT_BTN_X, ROW_Y_PLAYER});

    m_PlayerValue = std::make_shared<GameText>("1P", 45, k_Black);
    m_PlayerValue->SetZIndex(35);
    m_PlayerValue->SetPosition({COL_VALUE_X, ROW_Y_PLAYER});

    m_PlayerRightBtn = std::make_shared<UI_Triangle_Button>(
        GA_RESOURCE_DIR "/Image/Button/Right_Tri_Button.png",
        GA_RESOURCE_DIR "/Image/Button/Right_Tri_Button_Full.png");
    m_PlayerRightBtn->SetZIndex(35);
    m_PlayerRightBtn->SetPosition({COL_RIGHT_BTN_X, ROW_Y_PLAYER});

    static constexpr std::array<const char*, BIND_COUNT> labels = {
        "UP", "DOWN", "LEFT", "RIGHT", "JUMP",
        "CANCEL", "SHOT/SELECT", "MENU", "SUB MENU"
    };
    for (int i = 0; i < BIND_COUNT; ++i) {
        float y = BindRowY(i);
        m_BindLabels[i] = std::make_shared<GameText>(labels[i], 40, k_Black);
        m_BindLabels[i]->SetZIndex(35);
        m_BindLabels[i]->SetPosition({
            AppUtil::AlignLeft(*m_BindLabels[i], COL_LABEL_X), y});

        m_BindValues[i] = std::make_shared<GameText>("-", 40, k_Black);
        m_BindValues[i]->SetZIndex(35);
        m_BindValues[i]->SetPosition({COL_VALUE_X, y});
    }

    m_OkText      = std::make_shared<GameText>("OK",      45, k_Black);
    m_CancelText  = std::make_shared<GameText>("CANCEL",  45, k_Black);
    m_DefaultText = std::make_shared<GameText>("DEFAULT", 45, k_Black);
    m_OkText->SetZIndex(35);
    m_CancelText->SetZIndex(35);
    m_DefaultText->SetZIndex(35);
    m_OkText->SetPosition({COL_OK_X,      ROW_Y_BTN});
    m_CancelText->SetPosition({COL_CANCEL_X,  ROW_Y_BTN});
    m_DefaultText->SetPosition({COL_DEFAULT_X, ROW_Y_BTN});
}

void KeyboardConfigScene::OnEnter() {
    LOG_INFO("KeyboardConfigScene::OnEnter");

    m_Ctx.Root.AddChild(m_ExitGameButton);
    m_Ctx.Root.AddChild(m_Frame);
    m_Ctx.Root.AddChild(m_ChoiceFrame);
    m_Ctx.Root.AddChild(m_HLine1);
    m_Ctx.Root.AddChild(m_HLine2);
    m_Ctx.Root.AddChild(m_HLine3);
    m_Ctx.Root.AddChild(m_TitleText);
    m_Ctx.Root.AddChild(m_PlayerLabel);
    m_Ctx.Root.AddChild(m_PlayerLeftBtn);
    m_Ctx.Root.AddChild(m_PlayerValue);
    m_Ctx.Root.AddChild(m_PlayerRightBtn);
    for (int i = 0; i < BIND_COUNT; ++i) {
        m_Ctx.Root.AddChild(m_BindLabels[i]);
        m_Ctx.Root.AddChild(m_BindValues[i]);
    }
    m_Ctx.Root.AddChild(m_OkText);
    m_Ctx.Root.AddChild(m_CancelText);
    m_Ctx.Root.AddChild(m_DefaultText);

    m_ExitGameButton->SetPosition({399.2f, 285.1f});

    m_PlayerLeftBtn->ResetState();
    m_PlayerRightBtn->ResetState();

    // 進場時顯示 1P 設定
    m_CurrentPlayer = 0;
    m_SelectedRow   = ROW_PLAYER;
    m_WaitingForKey = false;

    LoadPlayer(0);
    UpdateValueTexts();
    UpdateChoiceFrame();
}

void KeyboardConfigScene::OnExit() {
    LOG_INFO("KeyboardConfigScene::OnExit");

    m_Ctx.Root.RemoveChild(m_Frame);
    m_Ctx.Root.RemoveChild(m_ChoiceFrame);
    m_Ctx.Root.RemoveChild(m_HLine1);
    m_Ctx.Root.RemoveChild(m_HLine2);
    m_Ctx.Root.RemoveChild(m_HLine3);
    m_Ctx.Root.RemoveChild(m_TitleText);
    m_Ctx.Root.RemoveChild(m_PlayerLabel);
    m_Ctx.Root.RemoveChild(m_PlayerLeftBtn);
    m_Ctx.Root.RemoveChild(m_PlayerValue);
    m_Ctx.Root.RemoveChild(m_PlayerRightBtn);
    for (int i = 0; i < BIND_COUNT; ++i) {
        m_Ctx.Root.RemoveChild(m_BindLabels[i]);
        m_Ctx.Root.RemoveChild(m_BindValues[i]);
    }
    m_Ctx.Root.RemoveChild(m_OkText);
    m_Ctx.Root.RemoveChild(m_CancelText);
    m_Ctx.Root.RemoveChild(m_DefaultText);

    m_ExitGameButton->SetPosition({331.0f, -14.0f});
    m_Ctx.Root.RemoveChild(m_ExitGameButton);
}

Scene* KeyboardConfigScene::Update() {
    m_PlayerLeftBtn->UpdateButton();
    m_PlayerRightBtn->UpdateButton();

    // 按鍵捕捉模式
    if (m_WaitingForKey) {
        Util::Keycode pressed = AppUtil::GetAnyKeyDown();
        if (pressed != Util::Keycode::UNKNOWN) {
            if (pressed != Util::Keycode::ESCAPE) {
                AssignKey(m_WaitingRow - ROW_BIND_0, pressed);
            }
            m_WaitingForKey = false;
            UpdateValueTexts();
        }
        return nullptr;
    }

    // ESC / X 按鈕 / CANCEL 不儲存，返回 OptionMenuScene
    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE) ||
        m_ExitGameButton->IsLeftClicked()              ||
        m_CancelText->IsLeftClicked())
    {
        LOG_INFO("KeyboardConfigScene: cancelled => OptionMenuScene");
        return m_OptionScene;
    }

    // 游標移動 W/S
    if (Util::Input::IsKeyDown(Util::Keycode::W) ||
        Util::Input::IsKeyDown(Util::Keycode::UP))
    {
        DecrementRow();
    }
    else if (Util::Input::IsKeyDown(Util::Keycode::S) ||
             Util::Input::IsKeyDown(Util::Keycode::DOWN))
    {
        IncrementRow();
    }

    // 滑鼠 hover 自動切換游標
    auto hoverTo = [&](const std::shared_ptr<GameText>& t, int row) {
        if (IsRowSelectable(row) && t->IsMouseHovering() && m_SelectedRow != row) {
            m_SelectedRow = row;
            UpdateChoiceFrame();
        }
    };
    hoverTo(m_PlayerValue, ROW_PLAYER);
    for (int i = 0; i < BIND_COUNT; ++i) {
        hoverTo(m_BindValues[i], ROW_BIND_0 + i);
    }
    hoverTo(m_OkText,      ROW_OK);
    hoverTo(m_CancelText,  ROW_CANCEL);
    hoverTo(m_DefaultText, ROW_DEFAULT);

    // A/D 或點擊按鈕
    bool pressedLeft  = Util::Input::IsKeyDown(Util::Keycode::A) ||
                        Util::Input::IsKeyDown(Util::Keycode::LEFT) ;
    bool pressedRight = Util::Input::IsKeyDown(Util::Keycode::D) ||
                        Util::Input::IsKeyDown(Util::Keycode::RIGHT) ||
                        m_PlayerRightBtn->IsLeftClicked();

    if ((m_SelectedRow == ROW_PLAYER && pressedLeft) || m_PlayerLeftBtn->IsLeftClicked()) {
        m_SelectedRow = ROW_PLAYER ;
        // 切到前一位玩家（循環）
        int next = (m_CurrentPlayer + MAX_PLAYERS - 1) % MAX_PLAYERS;
        m_CurrentPlayer = next;
        LoadPlayer(next);
        m_PlayerLeftBtn->Press(75.0f);
        UpdateValueTexts();
    }
    else if ((m_SelectedRow == ROW_PLAYER && pressedRight) || m_PlayerLeftBtn->IsLeftClicked()) {
        m_SelectedRow = ROW_PLAYER ;
        int next = (m_CurrentPlayer + 1) % MAX_PLAYERS;
        m_CurrentPlayer = next;
        LoadPlayer(next);
        m_PlayerRightBtn->Press(75.0f);
        UpdateValueTexts();
    }
    else if (m_SelectedRow == ROW_OK) {
        // A/D 在底部按鈕列移動游標
        if (pressedLeft)  { m_SelectedRow = ROW_DEFAULT; UpdateChoiceFrame(); }
        if (pressedRight) { m_SelectedRow = ROW_CANCEL; UpdateChoiceFrame(); }
    }
    else if (m_SelectedRow == ROW_CANCEL) {
        if (pressedLeft)  { m_SelectedRow = ROW_OK; UpdateChoiceFrame(); }
        if (pressedRight) { m_SelectedRow = ROW_DEFAULT; UpdateChoiceFrame(); }
    }
    else if (m_SelectedRow == ROW_DEFAULT) {
        if (pressedLeft)  { m_SelectedRow = ROW_CANCEL; UpdateChoiceFrame(); }
        if (pressedRight) { m_SelectedRow = ROW_OK; UpdateChoiceFrame(); }
    }

    // ENTER 確認
    if (Util::Input::IsKeyDown(Util::Keycode::RETURN) || Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
        if (m_SelectedRow == ROW_OK) {
            // 有衝突時禁止 OK（3P-8P）
            if (m_CurrentPlayer >= 1 && HasConflicts()) {
                LOG_INFO("KeyboardConfigScene: OK blocked due to key conflict");
            }
            else {
                CommitPending();
                LOG_INFO("KeyboardConfigScene: OK => OptionMenuScene");
                return m_OptionScene;
            }
        }
        else if (m_SelectedRow == ROW_CANCEL) {
            LOG_INFO("KeyboardConfigScene: CANCEL => OptionMenuScene");
            return m_OptionScene;
        }
        else if (m_SelectedRow == ROW_DEFAULT) {
            ApplyDefault();
            UpdateValueTexts();
        }
        else if (m_SelectedRow >= ROW_BIND_0 && m_SelectedRow <= ROW_SUBMENU) {
            // 綁定行：進入按鍵捕捉模式
            if (IsRowSelectable(m_SelectedRow)) {
                m_WaitingRow    = m_SelectedRow;
                m_WaitingForKey = true;
                // 顯示 "..." 提示使用者等待輸入
                int idx = m_SelectedRow - ROW_BIND_0;
                m_BindValues[idx]->SetText("...");
            }
        }
    }

    // 滑鼠點擊底部按鈕
    if (m_OkText->IsLeftClicked()) {
        if (!(m_CurrentPlayer >= 1 && HasConflicts())) {
            CommitPending();
            return m_OptionScene;
        }
    }
    if (m_DefaultText->IsLeftClicked()) {
        ApplyDefault();
        UpdateValueTexts();
    }

    return nullptr;
}

// 私有：載入玩家設定到 m_Pending
void KeyboardConfigScene::LoadPlayer(int playerIdx) {
    m_Pending = m_Applied[playerIdx];
    // 更新 PLAYER 值文字
    m_PlayerValue->SetText(std::to_string(playerIdx + 1) + "P");
}

// m_Pending → m_Applied[m_CurrentPlayer]
void KeyboardConfigScene::CommitPending() {
    m_Applied[m_CurrentPlayer] = m_Pending;
}

// 私有：套用預設值到 m_Pending
void KeyboardConfigScene::ApplyDefault() {
    if (m_CurrentPlayer == 0)      m_Pending = k_Default1P;
    else if (m_CurrentPlayer == 1) m_Pending = k_Default2P;
    else                           m_Pending = PlayerKeyConfig{};  // 3P-8P：清空
}

// 判斷列是否可選(2P+ 下 MENU/SUB MENU 不可選)
bool KeyboardConfigScene::IsRowSelectable(int row) const {
    if (m_CurrentPlayer > 0 &&
        (row == ROW_MENU_KEY || row == ROW_SUBMENU)) {
        return false;
    }
    return true;
}

// 游標移動(自動跳過不可選列)
void KeyboardConfigScene::DecrementRow() {
    int r = m_SelectedRow;
    do {
        r = (r + ROW_COUNT - 1) % ROW_COUNT;
    } while (!IsRowSelectable(r));
    m_SelectedRow = r;
    UpdateChoiceFrame();
}

void KeyboardConfigScene::IncrementRow() {
    int r = m_SelectedRow;
    do {
        r = (r + 1) % ROW_COUNT;
    } while (!IsRowSelectable(r));
    m_SelectedRow = r;
    UpdateChoiceFrame();
}

// 設定綁定值
void KeyboardConfigScene::AssignKey(int bindIdx, Util::Keycode key) {
    if (bindIdx >= 0 && bindIdx < static_cast<int>(kBindFields.size())) {
        m_Pending.*kBindFields[static_cast<size_t>(bindIdx)] = key;
    }
}

Util::Keycode KeyboardConfigScene::GetPendingKey(int bindIdx) const {
    if (bindIdx >= 0 && bindIdx < static_cast<int>(kBindFields.size())) {
        return m_Pending.*kBindFields[static_cast<size_t>(bindIdx)];
    }
    return Util::Keycode::UNKNOWN;
}

// 衝突偵測（只對 3P-8P）
std::vector<Util::Keycode> KeyboardConfigScene::GetConflicts() const {
    if (m_CurrentPlayer < 1) return {};

    // 收集其他所有玩家的已儲存按鍵
    std::vector<Util::Keycode> usedByOthers;
    for (int p = 0; p < MAX_PLAYERS; ++p) {
        if (p == m_CurrentPlayer) continue;
        auto keys = m_Applied[p].AllKeys();
        usedByOthers.insert(usedByOthers.end(), keys.begin(), keys.end());
    }

    // 找出 m_Pending 中與 usedByOthers 有交集的按鍵
    std::vector<Util::Keycode> conflicts;
    for (auto k : m_Pending.AllKeys()) {
        if (std::find(usedByOthers.begin(), usedByOthers.end(), k) != usedByOthers.end()) {
            conflicts.push_back(k);
        }
    }
    return conflicts;
}

// 根據 m_Pending 更新所有值文字與顏色
void KeyboardConfigScene::UpdateValueTexts() const {
    auto conflicts = GetConflicts();

    for (int i = 0; i < BIND_COUNT; ++i) {
        Util::Keycode key = GetPendingKey(i);
        std::string   str = AppUtil::KeycodeToString(key);
        m_BindValues[i]->SetText(str);

        bool disabled   = !IsRowSelectable(ROW_BIND_0 + i);
        bool conflicted = std::find(conflicts.begin(), conflicts.end(), key) != conflicts.end()
                          && key != Util::Keycode::UNKNOWN;

        if (disabled)        m_BindValues[i]->SetColor(k_Grey);
        else if (conflicted) m_BindValues[i]->SetColor(k_Red);
        else                 m_BindValues[i]->SetColor(k_Black);

        // Label 也用灰色標示不可選
        m_BindLabels[i]->SetColor(disabled ? k_Grey : k_Black);
    }

    // OK 按鈕：有衝突時變灰
    bool okDisabled = (m_CurrentPlayer >= 1 && HasConflicts());
    m_OkText->SetColor(okDisabled ? k_Grey : k_Black);
}

// RowY(用於 ChoiceFrame)
float KeyboardConfigScene::RowY(int row) {
    if (row == ROW_PLAYER)  return ROW_Y_PLAYER;
    if (row >= ROW_BIND_0 && row <= ROW_SUBMENU)
        return BindRowY(row - ROW_BIND_0);
    if (row == ROW_OK || row == ROW_CANCEL || row == ROW_DEFAULT)
        return ROW_Y_BTN;
    return 0.0f;
}

// 更新 ChoiceFrame 位置
void KeyboardConfigScene::UpdateChoiceFrame() const {
    float x = COL_VALUE_X - 10.0f;  // 預設：PLAYER 列與所有 BIND 列共用此 X
    if      (m_SelectedRow == ROW_OK)      x = COL_OK_X      - 10.0f;
    else if (m_SelectedRow == ROW_CANCEL)  x = COL_CANCEL_X  - 10.0f;
    else if (m_SelectedRow == ROW_DEFAULT) x = COL_DEFAULT_X - 10.0f;

    m_ChoiceFrame->SetPosition({x, RowY(m_SelectedRow)});
}

// 回傳已設定足夠按鍵的玩家數
// 判定標準：m_Applied[p].AllKeys().size() >= 4
// （至少需要上下左右四個方向鍵才算「已設定」）
// 初始狀態：1P 和 2P 有預設設定 → 回傳 2
//           3P-8P 全 UNKNOWN   → 不計入
int KeyboardConfigScene::GetConfiguredPlayerCount() const {
    int count = 0;
    for (int p = 0; p < MAX_PLAYERS; ++p) {
        if (static_cast<int>(m_Applied[p].AllKeys().size()) >= 4) {
            ++count;
        }
    }
    return count;
}