// //
// // Created by cody2 on 2026/3/14.
// //
//
// #ifndef OPTION_MENU_HPP
// #define OPTION_MENU_HPP
//
// #include <vector>
// #include <string>
// #include <memory>
// #include "Util/GameObject.hpp"
// #include "Util/Text.hpp"
// #include "Util/BGM.hpp"
// #include "Util/SFX.hpp"
//
// // 定義選項的列舉，方便管理狀態
// enum class OptionItem {
//     KEYBOARD_CONFIG,
//     BG_COLOR,
//     BGM_VOLUME,
//     SE_VOLUME,
//     DISP_NUMBER,
//     OK,
//     CANCEL
// };
//
// class OptionMenu : public Util::GameObject {
// public:
//     OptionMenu(Util::Color color);
//     ~OptionMenu() = default;
//
//     // 處理輸入與邏輯更新
//     void Update();
//
//     // 渲染畫面
//     void Draw();
//
//     // 判斷是否需要退出設定選單
//     bool ShouldExit() const { return m_ShouldExit; }
//     void ResetExitState() { m_ShouldExit = false; }
//
// private:
//     void HandleInput();
//     void UpdateUI();
//     std::string GetVolumeBar(int volume) const;
//
//     int m_CurrentIndex = 0;
//     bool m_ShouldExit = false;
//
//     // 遊戲音量內部狀態 (假設 0~20   )
//     int m_BgmVolume = 5;
//     int m_SeVolume = 5;
//
//     // UI 渲染元件
//     std::vector<OptionItem> m_Items;
//     std::vector<std::shared_ptr<Util::Text>> m_TextObjects;
//
//     // 常數設定
//     const int MAX_VOLUME = 10;
// };
//
// #endif // OPTION_MENU_HPP