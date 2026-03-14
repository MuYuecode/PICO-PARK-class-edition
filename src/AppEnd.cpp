// 必須 include 所有 Scene 的完整 header。
// 這樣 unique_ptr<TitleScene> 等在 App destructor 展開時
// 能看到完整型別，才能正確呼叫 delete。
#include "App.hpp"
#include "TitleScene.hpp"
#include "MenuScene.hpp"
#include "ExitConfirmScene.hpp"
#include "Util/Logger.hpp"
 
void App::End() {
    LOG_TRACE("End");
}