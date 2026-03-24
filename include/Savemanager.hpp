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

// Option 設定資料
// 對應 OptionMenuScene::Settings，但獨立定義以避免循環依賴
struct OptionSettingsData {
    int  bgColorIndex = 0;
    int  bgmVolume    = 10;
    int  seVolume     = 10;
    bool dispNumber   = false;
};

// 按鍵綁定資料(以 int 儲存，對應 Util::Keycode 底層值)
// Util::Keycode::UNKNOWN = 0(SDL_SCANCODE_UNKNOWN)
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

// 單一關卡存檔
struct LevelSaveData {
    bool completed = false;
    // index 2~8 分別對應 2P~8P 的最佳時間(秒)；-1.0f 表示尚未破關
    std::array<float, 9> bestTimes;

    LevelSaveData() { bestTimes.fill(-1.0f); }
};

// SaveManager
class SaveManager {
public:
    static constexpr int MAX_PLAYERS = 8;
    static constexpr int LEVEL_COUNT = 10;

    // settings.json

    /**
     * 儲存 Option 設定(背景色、音量等)
     * 採用 read-modify-write：不會覆蓋 keyConfigs 欄位
     */
    static bool SaveOptionSettings(const OptionSettingsData& opts);

    /**
     * 讀取 Option 設定
     * 若檔案不存在或解析失敗，保留 outOpts 原本的預設值並回傳 false
     */
    static bool LoadOptionSettings(OptionSettingsData& outOpts);

    /**
     * 儲存全部 8 位玩家的按鍵綁定
     * 採用 read-modify-write：不會覆蓋 option 設定欄位
     */
    static bool SaveKeyConfigs(
        const std::array<KeyConfigData, MAX_PLAYERS>& keys);

    /**
     * 讀取按鍵綁定
     * 若檔案不存在或解析失敗，保留 outKeys 原本的預設值並回傳 false
     */
    static bool LoadKeyConfigs(
        std::array<KeyConfigData, MAX_PLAYERS>& outKeys);

    // save_data.json

    /**
     * 儲存全部關卡的通關狀態與最佳時間
     */
    static bool SaveLevelData(
        const std::array<LevelSaveData, LEVEL_COUNT>& levels);

    /**
     * 讀取關卡存檔
     * 若檔案不存在或解析失敗，保留 outLevels 原本的預設值並回傳 false
     */
    static bool LoadLevelData(
        std::array<LevelSaveData, LEVEL_COUNT>& outLevels);

    /**
     * 便利函式：更新單一關卡的最佳時間
     * 只有當 elapsed < 目前記錄(或尚無記錄)時才更新並儲存
     * @param levelIdx    0-indexed (0 = Level 1)
     * @param playerCount 2 ~ 8
     * @param elapsed     這次破關耗時(秒)
     * @return true 表示確實寫入了新紀錄
     */
    static bool UpdateBestTime(int levelIdx, int playerCount, float elapsed);

    // 工具

    /** 格式化時間為 "mm:ss.cs"(mm=分, ss=秒, cs=百分秒)；-1.0f → "--:--.--" */
    static std::string FormatTime(float seconds);

    static std::string GetSettingsPath();
    static std::string GetSaveDataPath();
};

#endif // SAVE_MANAGER_HPP