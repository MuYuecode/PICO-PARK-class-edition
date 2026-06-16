# PICO PARK class edition

## 專案簡介

本專案為物件導向程式設計實習的遊戲專案，以《PICO PARK: Classic Edition》為參考，製作本機多人合作平台遊戲。專案使用
[PTSD](https://github.com/ntut-open-source-club/practical-tools-for-simple-design)
框架進行開發。

## 開發環境

- IDE：CLion 2026.1.3
- 程式語言：C++17
- 建置系統：CMake 3.26 或以上版本
- 遊戲框架：PTSD v0.2
- 使用函式庫：SDL2、SDL2_image、SDL2_ttf、SDL2_mixer、spdlog、glm、googletest、nlohmann/json

## 環境配置方式

1. 安裝 CLion 2026.1.3。
2. 將專案 clone 到本機。

```bash
git clone https://github.com/MuYuecode/PICO-PARK-class-edition.git
```

3. 使用 CLion 開啟專案根目錄。
4. 等待 CLion 執行 Load CMake Project。
5. Build 並執行 `picopart` target。

## CLion TLS 錯誤處理

部分電腦在使用學校有線網路與 CLion 2026.1.3 時，可能會在 Load CMake Project 階段遇到 TLS verification error，導致 FetchContent 無法下載 PTSD 或其他相依套件。

若發生此問題，可依照以下方式處理：

1. 開啟 File > Settings。
2. 進入 Build, Execution, Deployment > CMake。
3. 找到 Environment 欄位。
4. 加入以下環境變數：

```text
CMAKE_TLS_VERIFY=0
```

5. 執行 Tools > CMake > Reset Cache and Reload Project。
6. 重新 Build 並執行 `picopart` target。

## 注意事項

- 第一次載入 CMake 可能需要較長時間，因為專案會透過 FetchContent 準備相依套件。
- 使用 Debug build 執行時，請保留 `Resources` 資料夾於專案根目錄。
- 若多人遊玩時出現鍵盤無法同時讀取多個按鍵的情況，建議先測試鍵盤 rollover 能力，或改用外接機械鍵盤。
