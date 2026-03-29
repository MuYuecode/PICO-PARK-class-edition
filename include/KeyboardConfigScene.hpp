//
// Created by cody2 on 2026/3/16.
//

#ifndef PICOPART_KEYBOARDCONFIGSCENE_HPP
#define PICOPART_KEYBOARDCONFIGSCENE_HPP

#include <array>
#include <vector>
#include "Scene.hpp"
#include "Character.hpp"
#include "GameText.hpp"
#include "UITriangleButton.hpp"
#include "PlayerKeyConfig.hpp"
#include "Util/Keycode.hpp"

class OptionMenuScene;

class KeyboardConfigScene : public Scene {
public:
    KeyboardConfigScene(GameContext& ctx,
                        OptionMenuScene* optionScene,
                        std::shared_ptr<Character> exitGameButton);
    ~KeyboardConfigScene() override = default;

    void   OnEnter() override;
    void   OnExit()  override;
    Scene* Update()  override;

    void SetOptionScene(OptionMenuScene* s) { m_OptionScene = s; }

    static const PlayerKeyConfig k_Default1P;
    static const PlayerKeyConfig k_Default2P;

    static constexpr int MAX_PLAYERS = 8;

    [[nodiscard]] int GetConfiguredPlayerCount() const;

    [[nodiscard]] PlayerKeyConfig GetAppliedConfig(int playerIdx) const {
        if (playerIdx < 0 || playerIdx >= MAX_PLAYERS) {
            return PlayerKeyConfig{};
        }
        return m_Applied[playerIdx];
    }

private:
    std::shared_ptr<Character> m_ExitGameButton;
    std::shared_ptr<Character> m_Frame;
    std::shared_ptr<Character> m_ChoiceFrame;
    std::shared_ptr<Character> m_HLine1;
    std::shared_ptr<Character> m_HLine2;
    std::shared_ptr<Character> m_HLine3;

    std::shared_ptr<GameText> m_TitleText;

    std::shared_ptr<GameText>           m_PlayerLabel;
    std::shared_ptr<UITriangleButton> m_PlayerLeftBtn;
    std::shared_ptr<GameText>           m_PlayerValue;   // "1P" ~ "8P"
    std::shared_ptr<UITriangleButton> m_PlayerRightBtn;

    static constexpr int BIND_COUNT = 9;
    std::array<std::shared_ptr<GameText>, BIND_COUNT> m_BindLabels;
    std::array<std::shared_ptr<GameText>, BIND_COUNT> m_BindValues;

    std::shared_ptr<GameText> m_OkText;
    std::shared_ptr<GameText> m_CancelText;
    std::shared_ptr<GameText> m_DefaultText;

    OptionMenuScene* m_OptionScene = nullptr;

    std::array<PlayerKeyConfig, MAX_PLAYERS> m_Applied;
    PlayerKeyConfig m_Pending;

    int  m_CurrentPlayer = 0;

    int  m_SelectedRow  = 0;
    static constexpr int ROW_PLAYER   = 0;
    static constexpr int ROW_BIND_0   = 1;
    static constexpr int ROW_MENU_KEY = 8;
    static constexpr int ROW_SUBMENU  = 9;
    static constexpr int ROW_OK       = 10;
    static constexpr int ROW_CANCEL   = 11;
    static constexpr int ROW_DEFAULT  = 12;
    static constexpr int ROW_COUNT    = 13;

    bool m_WaitingForKey = false;
    int  m_WaitingRow    = -1;

    static constexpr float COL_LABEL_X     = -295.0f;
    static constexpr float COL_LEFT_BTN_X  =  130.0f;
    static constexpr float COL_VALUE_X     =  245.0f;
    static constexpr float COL_RIGHT_BTN_X =  340.0f;
    static constexpr float COL_OK_X        = -170.0f;
    static constexpr float COL_CANCEL_X    =   10.0f;
    static constexpr float COL_DEFAULT_X   =  175.0f;

    static constexpr float ROW_Y_TITLE    =  242.0f;
    static constexpr float ROW_Y_HLINE1   =  208.0f;
    static constexpr float ROW_Y_PLAYER   =  170.0f;
    static constexpr float ROW_Y_HLINE2   =  135.0f;
    static constexpr float ROW_Y_BIND_TOP =   96.0f;
    static constexpr float ROW_Y_STEP     =  -37.5f;
    static constexpr float ROW_Y_HLINE3   = -240.5f;
    static constexpr float ROW_Y_BTN      = -274.5f;

    static const Util::Color k_Black;
    static const Util::Color k_Red;
    static const Util::Color k_Grey;

    void LoadPlayer(int playerIdx);
    void CommitPending();
    void ApplyDefault();

    void DecrementRow();
    void IncrementRow();
    [[nodiscard]] bool IsRowSelectable(int row) const;

    void          AssignKey(int bindIdx, Util::Keycode key);
    [[nodiscard]] Util::Keycode GetPendingKey(int bindIdx) const;

    void UpdateValueTexts()  const ;
    void UpdateChoiceFrame() const ;
    void SyncAppliedToContext() const;

    [[nodiscard]] std::vector<Util::Keycode> GetConflicts() const;
    [[nodiscard]] bool HasConflicts() const { return !GetConflicts().empty(); }

    static float BindRowY(int bindIdx) {
        return ROW_Y_BIND_TOP + static_cast<float>(bindIdx) * ROW_Y_STEP;
    }
    static float RowY(int row) ;
};

#endif // PICOPART_KEYBOARDCONFIGSCENE_HPP