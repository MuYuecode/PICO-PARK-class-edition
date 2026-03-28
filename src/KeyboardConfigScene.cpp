//
// Created by cody2 on 2026/3/16.
//

#include "SaveManager.hpp"
#include "AppUtil.hpp"

#include "KeyboardConfigScene.hpp"
#include "OptionMenuScene.hpp"

#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

using namespace Util ;
using namespace AppUtil ;
const Color KeyboardConfigScene::k_Black = Color::FromRGB(0,   0,   0,   255);
const Color KeyboardConfigScene::k_Red   = Color::FromRGB(220, 50,  50,  255);
const Color KeyboardConfigScene::k_Grey  = Color::FromRGB(150, 150, 150, 255);

const PlayerKeyConfig KeyboardConfigScene::k_Default1P = {
    Keycode::W,
    Keycode::S,
    Keycode::A,
    Keycode::D,
    Keycode::W,
    Keycode::ESCAPE,
    Keycode::SPACE,
    Keycode::RETURN,
    Keycode::TAB
};

const PlayerKeyConfig KeyboardConfigScene::k_Default2P = {
    Keycode::UP,
    Keycode::DOWN,
    Keycode::LEFT,
    Keycode::RIGHT,
    Keycode::UP,
    Keycode::AC_BACK,
    Keycode::RCTRL,
    Keycode::UNKNOWN,
    Keycode::UNKNOWN
};

static constexpr std::array<Keycode PlayerKeyConfig::*, 9> kBindFields = {
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

std::vector<Keycode> PlayerKeyConfig::AllKeys() const {
    std::vector<Keycode> result;
    for (auto k : {up, down, left, right, jump, cancel, shot, menu, subMenu}) {
        if (k != Keycode::UNKNOWN) result.push_back(k);
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
    m_Applied[0] = k_Default1P;
    m_Applied[1] = k_Default2P;

    m_Frame = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/Option_Menu_Frame.png");
    m_Frame->SetZIndex(25);
    m_Frame->SetPosition({0.0f, -5.0f});

    m_ChoiceFrame = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/Option_Choice_Frame.png");
    m_ChoiceFrame->SetZIndex(30);
    m_ChoiceFrame->SetScale({0.46f, 1.0f}) ;

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
    m_PlayerLabel->SetPosition({AlignLeft(*m_PlayerLabel, COL_LABEL_X), ROW_Y_PLAYER});

    m_PlayerLeftBtn = std::make_shared<UITriangleButton>(
        GA_RESOURCE_DIR "/Image/Button/Left_Tri_Button.png",
        GA_RESOURCE_DIR "/Image/Button/Left_Tri_Button_Full.png");
    m_PlayerLeftBtn->SetZIndex(35);
    m_PlayerLeftBtn->SetPosition({COL_LEFT_BTN_X, ROW_Y_PLAYER});

    m_PlayerValue = std::make_shared<GameText>("1P", 45, k_Black);
    m_PlayerValue->SetZIndex(35);
    m_PlayerValue->SetPosition({COL_VALUE_X, ROW_Y_PLAYER});

    m_PlayerRightBtn = std::make_shared<UITriangleButton>(
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
            AlignLeft(*m_BindLabels[i], COL_LABEL_X), y});

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

    {
        std::array<KeyConfigData, MAX_PLAYERS> loaded;
        if (SaveManager::LoadKeyConfigs(loaded)) {
            auto toKeyConfig = [](const KeyConfigData& d) -> PlayerKeyConfig {
                PlayerKeyConfig p;
                p.up      = static_cast<Keycode>(d.up);
                p.down    = static_cast<Keycode>(d.down);
                p.left    = static_cast<Keycode>(d.left);
                p.right   = static_cast<Keycode>(d.right);
                p.jump    = static_cast<Keycode>(d.jump);
                p.cancel  = static_cast<Keycode>(d.cancel);
                p.shot    = static_cast<Keycode>(d.shot);
                p.menu    = static_cast<Keycode>(d.menu);
                p.subMenu = static_cast<Keycode>(d.subMenu);
                return p;
            };
            for (int i = 0; i < MAX_PLAYERS; ++i) {
                if (loaded[i].up != 0 || loaded[i].down != 0 ||
                    loaded[i].left != 0 || loaded[i].right != 0) {
                    m_Applied[i] = toKeyConfig(loaded[i]);
                    }
            }
        }
    }
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
    m_ExitGameButton->SetVisible(true);

    m_PlayerLeftBtn->ResetState();
    m_PlayerRightBtn->ResetState();

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

    if (m_WaitingForKey) {
        Keycode pressed = GetAnyKeyDown();
        if (pressed != Keycode::UNKNOWN) {
            if (pressed != Keycode::ESCAPE) {
                AssignKey(m_WaitingRow - ROW_BIND_0, pressed);
            }
            m_WaitingForKey = false;
            UpdateValueTexts();
        }
        return nullptr;
    }

    if (Input::IsKeyDown(Keycode::ESCAPE) ||
        m_ExitGameButton->IsLeftClicked()              ||
        m_CancelText->IsLeftClicked())
    {
        return m_OptionScene;
    }

    if (Input::IsKeyDown(Keycode::W) ||
        Input::IsKeyDown(Keycode::UP))
    {
        DecrementRow();
    }
    else if (Input::IsKeyDown(Keycode::S) ||
             Input::IsKeyDown(Keycode::DOWN))
    {
        IncrementRow();
    }

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

    bool pressedLeft  = Input::IsKeyDown(Keycode::A) ||
                        Input::IsKeyDown(Keycode::LEFT) ;
    bool pressedRight = Input::IsKeyDown(Keycode::D) ||
                        Input::IsKeyDown(Keycode::RIGHT) ||
                        m_PlayerRightBtn->IsLeftClicked();

    if ((m_SelectedRow == ROW_PLAYER && pressedLeft) || m_PlayerLeftBtn->IsLeftClicked()) {
        m_SelectedRow = ROW_PLAYER ;
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

    if (Input::IsKeyDown(Keycode::RETURN) || Input::IsKeyDown(Keycode::MOUSE_LB)) {
        if (m_SelectedRow == ROW_OK) {
            if (m_CurrentPlayer >= 1 && HasConflicts()) {
            }
            else {
                CommitPending();
                return m_OptionScene;
            }
        }
        else if (m_SelectedRow == ROW_CANCEL) {
            return m_OptionScene;
        }
        else if (m_SelectedRow == ROW_DEFAULT) {
            ApplyDefault();
            UpdateValueTexts();
        }
        else if (m_SelectedRow >= ROW_BIND_0 && m_SelectedRow <= ROW_SUBMENU) {
            if (IsRowSelectable(m_SelectedRow)) {
                m_WaitingRow    = m_SelectedRow;
                m_WaitingForKey = true;
                int idx = m_SelectedRow - ROW_BIND_0;
                m_BindValues[idx]->SetText("...");
            }
        }
    }

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

void KeyboardConfigScene::LoadPlayer(int playerIdx) {
    m_Pending = m_Applied[playerIdx];
    m_PlayerValue->SetText(std::to_string(playerIdx + 1) + "P");
}

void KeyboardConfigScene::CommitPending() {
    m_Applied[m_CurrentPlayer] = m_Pending;

    {
        auto toData = [](const PlayerKeyConfig& p) -> KeyConfigData {
            return { static_cast<int>(p.up),   static_cast<int>(p.down),
                     static_cast<int>(p.left),  static_cast<int>(p.right),
                     static_cast<int>(p.jump),  static_cast<int>(p.cancel),
                     static_cast<int>(p.shot),  static_cast<int>(p.menu),
                     static_cast<int>(p.subMenu) };
        };
        std::array<KeyConfigData, MAX_PLAYERS> saveData;
        for (int i = 0; i < MAX_PLAYERS; ++i)
            saveData[i] = toData(m_Applied[i]);
        SaveManager::SaveKeyConfigs(saveData);
    }
}

void KeyboardConfigScene::ApplyDefault() {
    if (m_CurrentPlayer == 0)      m_Pending = k_Default1P;
    else if (m_CurrentPlayer == 1) m_Pending = k_Default2P;
    else                           m_Pending = PlayerKeyConfig{};
}

bool KeyboardConfigScene::IsRowSelectable(int row) const {
    if (m_CurrentPlayer > 0 &&
        (row == ROW_MENU_KEY || row == ROW_SUBMENU)) {
        return false;
    }
    return true;
}

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

void KeyboardConfigScene::AssignKey(int bindIdx, Keycode key) {
    if (bindIdx >= 0 && bindIdx < static_cast<int>(kBindFields.size())) {
        m_Pending.*kBindFields[static_cast<size_t>(bindIdx)] = key;
    }
}

Keycode KeyboardConfigScene::GetPendingKey(int bindIdx) const {
    if (bindIdx >= 0 && bindIdx < static_cast<int>(kBindFields.size())) {
        return m_Pending.*kBindFields[static_cast<size_t>(bindIdx)];
    }
    return Keycode::UNKNOWN;
}

std::vector<Keycode> KeyboardConfigScene::GetConflicts() const {
    if (m_CurrentPlayer < 1) return {};

    std::vector<Keycode> usedByOthers;
    for (int p = 0; p < MAX_PLAYERS; ++p) {
        if (p == m_CurrentPlayer) continue;
        auto keys = m_Applied[p].AllKeys();
        usedByOthers.insert(usedByOthers.end(), keys.begin(), keys.end());
    }

    std::vector<Keycode> conflicts;
    for (auto k : m_Pending.AllKeys()) {
        if (std::find(usedByOthers.begin(), usedByOthers.end(), k) != usedByOthers.end()) {
            conflicts.push_back(k);
        }
    }
    return conflicts;
}

void KeyboardConfigScene::UpdateValueTexts() const {
    auto conflicts = GetConflicts();

    for (int i = 0; i < BIND_COUNT; ++i) {
        Keycode key = GetPendingKey(i);
        std::string   str = KeycodeToString(key);
        m_BindValues[i]->SetText(str);

        bool disabled   = !IsRowSelectable(ROW_BIND_0 + i);
        bool conflicted = std::find(conflicts.begin(), conflicts.end(), key) != conflicts.end()
                          && key != Keycode::UNKNOWN;

        if (disabled)        m_BindValues[i]->SetColor(k_Grey);
        else if (conflicted) m_BindValues[i]->SetColor(k_Red);
        else                 m_BindValues[i]->SetColor(k_Black);

        m_BindLabels[i]->SetColor(disabled ? k_Grey : k_Black);
    }

    bool okDisabled = (m_CurrentPlayer >= 1 && HasConflicts());
    m_OkText->SetColor(okDisabled ? k_Grey : k_Black);
}

float KeyboardConfigScene::RowY(int row) {
    if (row == ROW_PLAYER)  return ROW_Y_PLAYER;
    if (row >= ROW_BIND_0 && row <= ROW_SUBMENU)
        return BindRowY(row - ROW_BIND_0);
    if (row == ROW_OK || row == ROW_CANCEL || row == ROW_DEFAULT)
        return ROW_Y_BTN;
    return 0.0f;
}

void KeyboardConfigScene::UpdateChoiceFrame() const {
    float x = COL_VALUE_X - 10.0f;
    if      (m_SelectedRow == ROW_OK)      x = COL_OK_X      - 10.0f;
    else if (m_SelectedRow == ROW_CANCEL)  x = COL_CANCEL_X  - 10.0f;
    else if (m_SelectedRow == ROW_DEFAULT) x = COL_DEFAULT_X - 10.0f;

    m_ChoiceFrame->SetPosition({x, RowY(m_SelectedRow)});
}

int KeyboardConfigScene::GetConfiguredPlayerCount() const {
    int count = 0;
    for (int p = 0; p < MAX_PLAYERS; ++p) {
        if (static_cast<int>(m_Applied[p].AllKeys().size()) >= 4) {
            ++count;
        }
    }
    return count;
}