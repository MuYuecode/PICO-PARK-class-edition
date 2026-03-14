// //
// // Created by cody2 on 2026/3/14.
// //
//
// #include "OptionMenu.hpp"
// #include "Util/Input.hpp"
// #include "Util/Keycode.hpp"
// #include "Util/Color.hpp"
// #include "Util/Logger.hpp"
//
// OptionMenu::OptionMenu(Util::Color color) {
//     // 初始化選單項目 (排除 PAD CONFIG 與 WINDOW MODE)
//     m_Items = { OptionItem::KEYBOARD_CONFIG, OptionItem::BGM_VOLUME, OptionItem::SE_VOLUME, OptionItem::DISP_NUMBER, OptionItem::OK, OptionItem::CANCEL };
//
//     // 這裡需要替換成你們專案實際的字體路徑
//     std::string fontPath = "Resources/Font/TerminusTTFWindows-Bold-4.49.3.ttf";
//
//     // 初始化 UI 文字物件
//     for (size_t i = 0; i < m_Items.size(); ++i) {
//         auto text = std::make_shared<Util::Text>(fontPath, 40, " ", color);
//         m_TextObjects.push_back(text);
//     }
//
//     UpdateUI();
// }
//
// void OptionMenu::Update() {
//     HandleInput();
//     UpdateUI();
// }
//
// void OptionMenu::Draw() {
//     // 繪製背景 (可依需求加入暗化半透明背景或背景圖)
//     // 繪製文字
//     for (size_t i = 0; i < m_TextObjects.size(); ++i) {
//         // 設定選單的垂直排列位置
//         m_TextObjects[i]->SetTransform(Util::Transform{ {0.0f, 150.0f - (i * 100.0f)}, 0.0f, {1.0f, 1.0f} });
//         m_TextObjects[i]->Draw();
//     }
// }
//
// void OptionMenu::HandleInput() {
//     // 處理上下移動 (選單切換)
//     if (Util::Input::IsKeyDown(Util::Keycode::UP)) {
//         m_CurrentIndex--;
//         if (m_CurrentIndex < 0) m_CurrentIndex = m_Items.size() - 1;
//     }
//     if (Util::Input::IsKeyDown(Util::Keycode::DOWN)) {
//         m_CurrentIndex++;
//         if (m_CurrentIndex >= m_Items.size()) m_CurrentIndex = 0;
//     }
//
//     // 處理左右移動 (數值調整) 或 確認鍵
//     OptionItem currentItem = m_Items[m_CurrentIndex];
//
//     if (currentItem == OptionItem::BGM_VOLUME) {
//         if (Util::Input::IsKeyDown(Util::Keycode::LEFT) && m_BgmVolume > 0) {
//             m_BgmVolume--;
//             Util::BGM::SetVolume(m_BgmVolume * 5); // 假設引擎吃 0-100
//         }
//         if (Util::Input::IsKeyDown(Util::Keycode::RIGHT) && m_BgmVolume < MAX_VOLUME) {
//             m_BgmVolume++;
//             Util::BGM::SetVolume(m_BgmVolume * 5);
//         }
//     }
//     else if (currentItem == OptionItem::SE_VOLUME) {
//         if (Util::Input::IsKeyDown(Util::Keycode::LEFT) && m_SeVolume > 0) {
//             m_SeVolume--;
//             Util::SFX::SetVolume(m_SeVolume * 5);
//         }
//         if (Util::Input::IsKeyDown(Util::Keycode::RIGHT) && m_SeVolume < MAX_VOLUME) {
//             m_SeVolume++;
//             Util::SFX::SetVolume(m_SeVolume * 5);
//             // 可以加上播放一聲 SE 讓玩家確認音量的邏輯
//         }
//     }
//     else if (currentItem == OptionItem::BACK) {
//         if (Util::Input::IsKeyDown(Util::Keycode::RETURN) || Util::Input::IsKeyDown(Util::Keycode::Z)) {
//             m_ShouldExit = true;
//         }
//     }
// }
//
// void OptionMenu::UpdateUI() {
//     for (size_t i = 0; i < m_Items.size(); ++i) {
//         std::string displayText = "";
//
//         // 加上游標指示器
//         if (i == m_CurrentIndex) {
//             displayText += "> ";
//         } else {
//             displayText += "  ";
//         }
//
//         switch (m_Items[i]) {
//             case OptionItem::BGM_VOLUME:
//                 displayText += "BGM VOLUME   " + GetVolumeBar(m_BgmVolume);
//                 break;
//             case OptionItem::SE_VOLUME:
//                 displayText += "SE VOLUME    " + GetVolumeBar(m_SeVolume);
//                 break;
//             case OptionItem::BACK:
//                 displayText += "BACK";
//                 break;
//         }
//
//         m_TextObjects[i]->SetText(displayText);
//
//         // 被選中的項目給予不同顏色提示
//         if (i == m_CurrentIndex) {
//             m_TextObjects[i]->SetColor(Util::Color::FromName(Util::Colors::YELLOW));
//         } else {
//             m_TextObjects[i]->SetColor(Util::Color::FromName(Util::Colors::WHITE));
//         }
//     }
// }
//
// std::string OptionMenu::GetVolumeBar(int volume) const {
//     std::string bar = "[";
//     for (int i = 0; i < MAX_VOLUME; ++i) {
//         if (i < volume) bar += "|";
//         else bar += " ";
//     }
//     bar += "]";
//     return bar;
// }