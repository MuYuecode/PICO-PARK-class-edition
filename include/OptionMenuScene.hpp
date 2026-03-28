//
// Created by cody2 on 2026/3/15.
//

#ifndef PICOPART_OPTIONMENUSCENE_HPP
#define PICOPART_OPTIONMENUSCENE_HPP

#include <vector>
#include <string>

#include "Scene.hpp"
#include "Character.hpp"
#include "GameText.hpp"
#include "UITriangleButton.hpp"

class MenuScene;
class KeyboardConfigScene;

class OptionMenuScene : public Scene {
public:
    OptionMenuScene(GameContext& ctx,
                    MenuScene* menuScene,
                    std::shared_ptr<Character> exitGameButton);
    ~OptionMenuScene() override = default;

    void   OnEnter() override;
    void   OnExit()  override;
    Scene* Update()  override;

    void SetMenuScene(MenuScene* s) { m_MenuScene = s; }
    void SetKeyboardConfigScene(KeyboardConfigScene* s) { m_KeyboardConfigScene = s; }

    struct Settings {
        int  bgColorIndex = 0;
        int  bgmVolume    = 10;
        int  seVolume     = 10;
        bool dispNumber   = false;
    };

    Settings m_Applied;
    Settings m_Pending;

private:
    std::shared_ptr<Character> m_ExitGameButton;

    std::shared_ptr<Character> m_OptionMenuFrame;
    std::shared_ptr<Character> m_ChoiceFrame;
    std::shared_ptr<GameText> m_TitleText;           // "OPTION"

    std::shared_ptr<GameText> m_KbConfigLabel;       // "KEYBOARD CONFIG"
    std::shared_ptr<GameText> m_KbConfigOpen;        // "OPEN"

    std::shared_ptr<GameText>           m_BgColorLabel;
    std::shared_ptr<UITriangleButton> m_BgColorLeftBtn;
    std::shared_ptr<GameText>           m_BgColorValue;
    std::shared_ptr<UITriangleButton> m_BgColorRightBtn;

    std::shared_ptr<GameText>           m_BgmVolumeLabel;
    std::shared_ptr<UITriangleButton> m_BgmVolumeLeftBtn;
    std::shared_ptr<GameText>           m_BgmVolumeValue;
    std::shared_ptr<UITriangleButton> m_BgmVolumeRightBtn;

    std::shared_ptr<GameText>           m_SeVolumeLabel;
    std::shared_ptr<UITriangleButton> m_SeVolumeLeftBtn;
    std::shared_ptr<GameText>           m_SeVolumeValue;
    std::shared_ptr<UITriangleButton> m_SeVolumeRightBtn;

    std::shared_ptr<GameText>           m_DispNumberLabel;
    std::shared_ptr<UITriangleButton> m_DispNumberLeftBtn;
    std::shared_ptr<GameText>           m_DispNumberValue;
    std::shared_ptr<UITriangleButton> m_DispNumberRightBtn;

    std::shared_ptr<GameText> m_OkText;
    std::shared_ptr<GameText> m_CancelText;

    MenuScene* m_MenuScene = nullptr;
    KeyboardConfigScene* m_KeyboardConfigScene = nullptr;

    static const std::vector<std::string> s_BgColorOptions;
    static const std::vector<std::string> s_BgColorPaths;

    int m_SelectedRow   = 0;
    static constexpr int ROW_COUNT = 7;
    static constexpr float ROW_Y_KB    =  145.0f;
    static constexpr float ROW_Y_BG    =  60.0f;
    static constexpr float ROW_Y_BGM   = -25.0f;
    static constexpr float ROW_Y_SE    = -110.0f;
    static constexpr float ROW_Y_DISP  = -195.0f;
    static constexpr float ROW_Y_BTN   = -260.0f;

    static constexpr float COL_LABEL_X      = -330.0f;
    static constexpr float COL_LEFT_BTN_X   =  100.0f;
    static constexpr float COL_VALUE_X      =  230.0f;
    static constexpr float COL_RIGHT_BTN_X  =  330.0f;
    static constexpr float COL_OK_X         = -130.0f;
    static constexpr float COL_CANCEL_X     =  130.0f;

    void UpdateChoiceFrame() const;
    void UpdateValueTexts() const;

    void DecrementRow();
    void IncrementRow();
    void AdjustLeft(int row);
    void AdjustRight(int row);

    void SwapOkCancel();
};

#endif //PICOPART_OPTIONMENUSCENE_HPP