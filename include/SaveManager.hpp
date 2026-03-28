//
// Created by cody2 on 2026/3/24.
//
// SaveManager.hpp
// 負責讀寫 GA_RESOURCE_DIR/Save/ 下的持久化資料：
//   settings.json  — OptionMenuScene 設定 + KeyboardConfigScene 按鍵綁定
//   save_data.json — 各關卡通關狀態與最佳時間紀錄
//
// 使用方式(各場景只需 #include 此標頭)：
//
//   // 儲存 Option 設定(在 OptionMenuScene OK 時呼叫)
//   SaveManager::SaveOptionSettings(opts);
//
//   // 儲存按鍵綁定(在 KeyboardConfigScene CommitPending 時呼叫)
//   SaveManager::SaveKeyConfigs(keysArray);
//
//   // 更新單一關卡最佳時間(在關卡完成時呼叫)
//   SaveManager::UpdateBestTime(levelIdx, playerCount, elapsedSeconds);
//

#ifndef SAVE_MANAGER_HPP
#define SAVE_MANAGER_HPP

#include <string>
#include <array>

struct OptionSettingsData {
    int  bgColorIndex = 0;
    int  bgmVolume    = 10;
    int  seVolume     = 10;
    bool dispNumber   = false;
};

struct KeyConfigData {
    int up      = 0;
    int down    = 0;
    int left    = 0;
    int right   = 0;
    int jump    = 0;
    int cancel  = 0;
    int shot    = 0;
    int menu    = 0;
    int subMenu = 0;
};

struct LevelSaveData {
    bool completed = false;
    std::array<float, 9> bestTimes;

    LevelSaveData() { bestTimes.fill(-1.0f); }
};

class SaveManager {
public:
    static constexpr int MAX_PLAYERS = 8;
    static constexpr int LEVEL_COUNT = 10;

    static bool SaveOptionSettings(const OptionSettingsData& opts);

    static bool LoadOptionSettings(OptionSettingsData& outOpts);

    static bool SaveKeyConfigs(
        const std::array<KeyConfigData, MAX_PLAYERS>& keys);

    static bool LoadKeyConfigs(
        std::array<KeyConfigData, MAX_PLAYERS>& outKeys);

    static bool SaveLevelData(
        const std::array<LevelSaveData, LEVEL_COUNT>& levels);

    static bool LoadLevelData(
        std::array<LevelSaveData, LEVEL_COUNT>& outLevels);

    static bool UpdateBestTime(int levelIdx, int playerCount, float elapsed);

    static std::string FormatTime(float seconds);

    static std::string GetSettingsPath();
    static std::string GetSaveDataPath();
};

#endif // SAVE_MANAGER_HPP