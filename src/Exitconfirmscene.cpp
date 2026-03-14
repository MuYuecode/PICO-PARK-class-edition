#include "ExitConfirmScene.hpp"
#include "MenuScene.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

// =============================================================================
// Constructor
// =============================================================================
ExitConfirmScene::ExitConfirmScene(GameContext& ctx,
                                   MenuScene* menuScene,
                                   std::shared_ptr<Character> menuFrame,
                                   std::shared_ptr<Character> exitGameButton)
    : Scene(ctx)
    , m_MenuFrame(std::move(menuFrame))
    , m_ExitGameButton(std::move(exitGameButton))
    , m_MenuScene(menuScene)
{
    const Util::Color black = Util::Color::FromRGB(0, 0, 0, 255);

    m_ExitGame1Text = std::make_shared<GameText>("EXIT GAME ?", 65, black);
    m_ExitGame1Text->SetPosition({10.0f, -53.0f});

    m_YesText = std::make_shared<GameText>("YES", 65, black);
    m_YesText->SetZIndex(10);
    m_YesText->SetPosition({-110.0f, -183.0f});

    m_NoText = std::make_shared<GameText>("NO", 65, black);
    m_NoText->SetZIndex(10);
    m_NoText->SetPosition({130.0f, -183.0f});

    m_ChoiceFrame = std::make_shared<Character>(
        GA_RESOURCE_DIR "/Image/Background/Choice_Frame.png");
    m_ChoiceFrame->SetZIndex(20);
}

// =============================================================================
// OnEnter
// =============================================================================
void ExitConfirmScene::OnEnter() {
    LOG_INFO("ExitConfirmScene::OnEnter");

    // 私有 UI 元件加入渲染樹
    m_Ctx.Root.AddChild(m_ExitGame1Text);
    m_Ctx.Root.AddChild(m_YesText);
    m_Ctx.Root.AddChild(m_NoText);
    m_Ctx.Root.AddChild(m_ChoiceFrame);

    // 修改借用物件的外觀（MenuFrame 變小、X 按鈕移動位置）
    // 這些修改在 OnExit 裡還原，保持「誰修改誰還原」的原則。
    m_MenuFrame->SetScale({408.0f / 695.0f, 287.0f / 218.0f});
    m_ExitGameButton->SetPosition({192.3f, 9.0f});

    // 預設游標在 YES
    m_IsYesSelected = true;
    UpdateChoiceFramePosition();
}

// =============================================================================
// OnExit
// =============================================================================
void ExitConfirmScene::OnExit() {
    LOG_INFO("ExitConfirmScene::OnExit");

    // 私有 UI 元件移出渲染樹
    m_Ctx.Root.RemoveChild(m_ExitGame1Text);
    m_Ctx.Root.RemoveChild(m_YesText);
    m_Ctx.Root.RemoveChild(m_NoText);
    m_Ctx.Root.RemoveChild(m_ChoiceFrame);

    // 還原借用物件的外觀改動
    m_MenuFrame->SetScale({1.0f, 1.0f});
    m_ExitGameButton->SetPosition({331.0f, -14.0f});
}

// =============================================================================
// Update
// =============================================================================
Scene* ExitConfirmScene::Update() {

    // A / D 鍵切換游標
    if (Util::Input::IsKeyDown(Util::Keycode::A) ||
        Util::Input::IsKeyDown(Util::Keycode::D)) {
        m_IsYesSelected = !m_IsYesSelected;
        UpdateChoiceFramePosition();
    }

    // 滑鼠 hover 也可以切換游標
    if (m_YesText->IsMouseHovering() && !m_IsYesSelected) {
        m_IsYesSelected = true;
        UpdateChoiceFramePosition();
    }
    if (m_NoText->IsMouseHovering() && m_IsYesSelected) {
        m_IsYesSelected = false;
        UpdateChoiceFramePosition();
    }

    // ESC 或 X 按鈕 → 取消，回 MenuScene
    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE) ||
        m_ExitGameButton->IsLeftClicked()) {
        LOG_INFO("ExitConfirmScene: cancelled, back to MenuScene");
        return m_MenuScene;
    }

    // ENTER 或滑鼠點擊 → 依游標位置決定
    const bool confirmByKeyboard = Util::Input::IsKeyDown(Util::Keycode::RETURN);
    const bool confirmByMouse    = m_YesText->IsLeftClicked() || m_NoText->IsLeftClicked();

    if (confirmByKeyboard || confirmByMouse) {
        // 點擊滑鼠時，以滑鼠所在位置決定（而非 m_IsYesSelected）
        bool chooseYes = m_IsYesSelected;
        if (confirmByMouse) {
            chooseYes = m_YesText->IsLeftClicked();
        }

        if (chooseYes) {
            LOG_INFO("ExitConfirmScene: YES confirmed → ShouldQuit");
            m_Ctx.ShouldQuit = true;
            return nullptr; // App::Update 會偵測 ShouldQuit 後設 State::END
        }
        else {
            LOG_INFO("ExitConfirmScene: NO confirmed, back to MenuScene");
            return m_MenuScene;
        }
    }

    return nullptr;
}

// =============================================================================
// Private helper
// =============================================================================
void ExitConfirmScene::UpdateChoiceFramePosition() {
    // YES 在左（-120），NO 在右（+120）
    if (m_IsYesSelected) {
        m_ChoiceFrame->SetPosition({-120.0f, -180.0f});
    }
    else {
        m_ChoiceFrame->SetPosition({120.0f, -180.0f});
    }
}