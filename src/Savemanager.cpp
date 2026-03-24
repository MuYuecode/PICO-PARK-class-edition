//
// Created by cody2 on 2026/3/24.
//

#include "SaveManager.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <nlohmann/json.hpp>
#include "Util/Logger.hpp"

using json = nlohmann::json;

//  路徑

std::string SaveManager::GetSettingsPath() {
    return std::string(GA_RESOURCE_DIR) + "/Save/settings.json";
}

std::string SaveManager::GetSaveDataPath() {
    return std::string(GA_RESOURCE_DIR) + "/Save/save_data.json";
}

//  工具

std::string SaveManager::FormatTime(float seconds) {
    if (seconds < 0.0f) return "--:--.--";

    // 轉換為百分秒(centiseconds)
    auto totalCs = static_cast<int>(seconds * 100.0f + 0.5f);
    int cs  = totalCs % 100;
    int sec = (totalCs / 100) % 60;
    int min = totalCs / 6000;

    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(2) << min << ":"
        << std::setw(2) << sec << "."
        << std::setw(2) << cs;
    return oss.str();
}

// 確保 Save 目錄存在
static bool EnsureSaveDir() {
    std::filesystem::path dir(std::string(GA_RESOURCE_DIR) + "/Save");
    try {
        if (!std::filesystem::exists(dir)) {
            std::filesystem::create_directories(dir);
        }
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("SaveManager: cannot create Save dir: {}", e.what());
        return false;
    }
}

// 讀取已有的 settings.json(若不存在或解析失敗，回傳空物件)
static json ReadSettingsJson() {
    std::ifstream f(SaveManager::GetSettingsPath());
    if (!f) return json::object();
    try {
        json j;
        f >> j;
        return j;
    } catch (...) {
        return json::object();
    }
}

//  settings.json — Option 設定

bool SaveManager::SaveOptionSettings(const OptionSettingsData& opts) {
    if (!EnsureSaveDir()) return false;

    json j = ReadSettingsJson();  // read-modify-write
    j["bgColorIndex"] = opts.bgColorIndex;
    j["bgmVolume"]    = opts.bgmVolume;
    j["seVolume"]     = opts.seVolume;
    j["dispNumber"]   = opts.dispNumber;

    try {
        std::ofstream f(GetSettingsPath());
        if (!f) { LOG_ERROR("SaveManager: cannot open settings.json for write"); return false; }
        f << j.dump(2);
        LOG_INFO("SaveManager: saved option settings");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("SaveManager: SaveOptionSettings failed: {}", e.what());
        return false;
    }
}

bool SaveManager::LoadOptionSettings(OptionSettingsData& outOpts) {
    json j = ReadSettingsJson();
    if (j.empty()) return false;

    bool any = false;
    auto tryLoad = [&](const char* key, auto& dst) {
        if (j.contains(key)) { dst = j[key].get<std::decay_t<decltype(dst)>>(); any = true; }
    };
    tryLoad("bgColorIndex", outOpts.bgColorIndex);
    tryLoad("bgmVolume",    outOpts.bgmVolume);
    tryLoad("seVolume",     outOpts.seVolume);
    tryLoad("dispNumber",   outOpts.dispNumber);
    return any;
}

//  settings.json — 按鍵綁定

bool SaveManager::SaveKeyConfigs(const std::array<KeyConfigData, MAX_PLAYERS>& keys) {
    if (!EnsureSaveDir()) return false;

    json j = ReadSettingsJson();  // read-modify-write

    json arr = json::array();
    for (const auto& k : keys) {
        json kj;
        kj["up"]      = k.up;
        kj["down"]    = k.down;
        kj["left"]    = k.left;
        kj["right"]   = k.right;
        kj["jump"]    = k.jump;
        kj["cancel"]  = k.cancel;
        kj["shot"]    = k.shot;
        kj["menu"]    = k.menu;
        kj["subMenu"] = k.subMenu;
        arr.push_back(kj);
    }
    j["keyConfigs"] = arr;

    try {
        std::ofstream f(GetSettingsPath());
        if (!f) { LOG_ERROR("SaveManager: cannot open settings.json for write"); return false; }
        f << j.dump(2);
        LOG_INFO("SaveManager: saved key configs");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("SaveManager: SaveKeyConfigs failed: {}", e.what());
        return false;
    }
}

bool SaveManager::LoadKeyConfigs(std::array<KeyConfigData, MAX_PLAYERS>& outKeys) {
    json j = ReadSettingsJson();
    if (!j.contains("keyConfigs") || !j["keyConfigs"].is_array()) return false;

    auto& arr = j["keyConfigs"];
    bool  any = false;
    for (int i = 0; i < MAX_PLAYERS && i < static_cast<int>(arr.size()); ++i) {
        auto& kj = arr[i];
        auto& k  = outKeys[i];
        auto tryLoad = [&](const char* key, int& dst) {
            if (kj.contains(key)) { dst = kj[key].get<int>(); any = true; }
        };
        tryLoad("up",      k.up);
        tryLoad("down",    k.down);
        tryLoad("left",    k.left);
        tryLoad("right",   k.right);
        tryLoad("jump",    k.jump);
        tryLoad("cancel",  k.cancel);
        tryLoad("shot",    k.shot);
        tryLoad("menu",    k.menu);
        tryLoad("subMenu", k.subMenu);
    }
    return any;
}

//  save_data.json

bool SaveManager::SaveLevelData(const std::array<LevelSaveData, LEVEL_COUNT>& levels) {
    if (!EnsureSaveDir()) return false;

    json arr = json::array();
    for (int i = 0; i < LEVEL_COUNT; ++i) {
        json lj;
        lj["levelId"]   = i + 1;
        lj["completed"] = levels[i].completed;
        json times = json::object();
        for (int p = 2; p <= 8; ++p) {
            times[std::to_string(p)] = levels[i].bestTimes[p];
        }
        lj["bestTimes"] = times;
        arr.push_back(lj);
    }

    json j;
    j["levels"] = arr;

    try {
        std::ofstream f(GetSaveDataPath());
        if (!f) { LOG_ERROR("SaveManager: cannot open save_data.json for write"); return false; }
        f << j.dump(2);
        LOG_INFO("SaveManager: saved level data");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("SaveManager: SaveLevelData failed: {}", e.what());
        return false;
    }
}

bool SaveManager::LoadLevelData(std::array<LevelSaveData, LEVEL_COUNT>& outLevels) {
    std::ifstream f(GetSaveDataPath());
    if (!f) return false;

    try {
        json j;
        f >> j;
        if (!j.contains("levels") || !j["levels"].is_array()) return false;

        for (auto& lj : j["levels"]) {
            int id = lj.value("levelId", 0);
            if (id < 1 || id > LEVEL_COUNT) continue;
            int idx = id - 1;
            outLevels[idx].completed = lj.value("completed", false);
            if (lj.contains("bestTimes") && lj["bestTimes"].is_object()) {
                for (int p = 2; p <= 8; ++p) {
                    std::string key = std::to_string(p);
                    if (lj["bestTimes"].contains(key)) {
                        outLevels[idx].bestTimes[p] = lj["bestTimes"][key].get<float>();
                    }
                }
            }
        }
        LOG_INFO("SaveManager: loaded level data");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("SaveManager: LoadLevelData failed: {}", e.what());
        return false;
    }
}

bool SaveManager::UpdateBestTime(int levelIdx, int playerCount, float elapsed) {
    if (levelIdx < 0 || levelIdx >= LEVEL_COUNT) return false;
    if (playerCount < 2 || playerCount > 8)       return false;
    if (elapsed < 0.0f)                            return false;

    // 讀取現有資料(不存在則使用預設)
    std::array<LevelSaveData, LEVEL_COUNT> levels;
    LoadLevelData(levels);

    float& best = levels[levelIdx].bestTimes[playerCount];
    if (best < 0.0f || elapsed < best) {
        best = elapsed;
        levels[levelIdx].completed = true;
        LOG_INFO("SaveManager: new best time Level {} ({}P): {}s",
                 levelIdx + 1, playerCount, elapsed);
        return SaveLevelData(levels);
    }
    return false;  // 非最佳，不更新
}