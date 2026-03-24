#include "Titlescene.hpp"
#include "Menuscene.hpp"

#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"

TitleScene::TitleScene(GameContext& ctx, MenuScene* menuScene)
    : Scene(ctx), m_MenuScene(menuScene)
{
    const Util::Color orange(254, 133, 78, 255);

    m_TitleSub = std::make_shared<GameText>("- CLASSIC EDITED -", 46, orange);
    m_TitleSub->SetZIndex(0);
    m_TitleSub->SetPosition({0.0f, -22.5f});

    m_PressEnterText = std::make_shared<GameText>("PRESS ENTER KEY", 66, orange);
    m_PressEnterText->SetZIndex(0);
    m_PressEnterText->SetPosition({0.0f, -155.0f});
}

void TitleScene::OnEnter() {
    LOG_INFO("TitleScene::OnEnter");

    m_Ctx.Header->SetVisible(true);
    m_Ctx.Root.AddChild(m_TitleSub);
    m_Ctx.Root.AddChild(m_PressEnterText);

    m_FlashTimer = 0.0f;
    m_PressEnterText->SetVisible(true);

    // 建立 PhysicsAgent 清單
    m_Agents.clear();
    for (int i = 0; i < static_cast<int>(m_Ctx.StartupCats.size()); ++i) {
        auto& cat = m_Ctx.StartupCats[i];
        if (cat == nullptr) continue;

        cat->SetInputEnabled(i < 2);
        cat->SetCatAnimState(CatAnimState::STAND);

        PhysicsAgent agent;
        agent.actor = cat;
        // state 使用預設值(grounded=true, velocityY=0 等)
        m_Agents.push_back(agent);
    }
}

void TitleScene::OnExit() {
    LOG_INFO("TitleScene::OnExit");

    m_Ctx.Root.RemoveChild(m_TitleSub);
    m_Ctx.Root.RemoveChild(m_PressEnterText);

    for (auto& cat : m_Ctx.StartupCats) {
        if (cat != nullptr) cat->SetInputEnabled(false);
    }

    m_Agents.clear();
}

Scene* TitleScene::Update() {
    // PRESS ENTER 閃爍
    m_FlashTimer += Util::Time::GetDeltaTimeMs();
    if (m_FlashTimer >= 1000.0f) {
        m_PressEnterText->SetVisible(!m_PressEnterText->GetVisibility());
        m_FlashTimer = 0.0f;
    }

    // 讀取輸入(Scene 的責任：決定每隻角色想往哪走、是否想跳)
    for (auto& agent : m_Agents) {
        if (agent.actor == nullptr) continue;
        auto& st = agent.state;

        st.moveDir = 0;

        if (agent.actor->GetInputEnabled()) {
            const Util::Keycode lk = agent.actor->GetLeftKey();
            const Util::Keycode rk = agent.actor->GetRightKey();
            const Util::Keycode jk = agent.actor->GetJumpKey();

            const bool goLeft  = (lk != Util::Keycode::UNKNOWN) &&
                                  Util::Input::IsKeyPressed(lk);
            const bool goRight = (rk != Util::Keycode::UNKNOWN) &&
                                  Util::Input::IsKeyPressed(rk);

            if      (goLeft  && !goRight) st.moveDir = -1;
            else if (goRight && !goLeft)  st.moveDir =  1;

            // 直接設定 velocityY(System 在下一步施加重力)
            const bool wantJump = (jk != Util::Keycode::UNKNOWN) &&
                                   Util::Input::IsKeyDown(jk);

            if (st.grounded && wantJump) {
                CharacterPhysicsSystem::ApplyJump(st);
            }
        }
    }

    // 委託物理系統處理所有運算(含動畫)
    // m_Physics.Update(m_Agents, m_Ctx.Floor);
    CharacterPhysicsSystem::Update(m_Agents, m_Ctx.Floor);

    //場景切換
    if (Util::Input::IsKeyDown(Util::Keycode::RETURN)) {
        LOG_INFO("TitleScene: ENTER pressed → MenuScene");
        return m_MenuScene;
    }

    return nullptr;
}