//
// Created by cody2 on 2026/3/14.
//

#ifndef PICOPART_SCENE_HPP
#define PICOPART_SCENE_HPP

#include "GameContext.hpp"

/**
 * @brief 所有場景的抽象基底類別
 *
 * 生命週期
 *
 *   App::Start()    → 建立所有 Scene 物件（此時不呼叫 OnEnter）
 *   TransitionTo(X) → 先呼叫前一個場景的 OnExit()，再呼叫 X 的 OnEnter()
 *   每個 frame      → Update() 被呼叫，回傳下一個場景的指標
 *
 * Update() 回傳值約定
 *
 *   nullptr   → 繼續待在這個場景，不切換
 *   Scene*    → 切換到該場景（指標由 App 持有，Scene 只借用，不能 delete）
 *
 * AddChild / RemoveChild 的使用規則
 *
 *   OnEnter() 裡呼叫 m_Ctx.Root.AddChild(xxx)   → 加入渲染樹
 *   OnExit()  裡呼叫 m_Ctx.Root.RemoveChild(xxx) → 從渲染樹移除
 *
 *   「一定要配對」，否則同一個 shared_ptr 會在渲染清單裡出現兩次，
 *   造成每次進入場景就多渲染一個重疊的圖層。
 *
 * 所有權
 *
 *   App 以 unique_ptr<Scene> 持有所有場景（負責生命週期）
 *   Scene 之間互傳的是 raw pointer（non-owning），絕對不能 delete
 */
class Scene {
public:
    explicit Scene(GameContext& ctx) : m_Ctx(ctx) {}

    virtual ~Scene() = default;

    // 場景不應被複製或移動
    Scene(const Scene&)            = delete;
    Scene& operator=(const Scene&) = delete;
    Scene(Scene&&)                 = delete;
    Scene& operator=(Scene&&)      = delete;

    // 純虛擬介面
    /**
     * @brief 進入此場景時呼叫一次
     * - 呼叫 m_Ctx.Root.AddChild() 把需要的物件加入渲染樹
     * - 重置場景內部狀態（計時器、選擇索引…）
     */
    virtual void OnEnter() = 0;

    /**
     * @brief 離開此場景時呼叫一次
     * - 呼叫 m_Ctx.Root.RemoveChild() 移除本場景加入的物件
     * - 必須與 OnEnter 裡的 AddChild 完全配對
     */
    virtual void OnExit()  = 0;

    /**
     * @brief 每 frame 呼叫一次
     * @return nullptr 繼續待在此場景；其他指標切換到該場景
     */
    virtual Scene* Update() = 0;

protected:
    GameContext& m_Ctx;
};

#endif //PICOPART_SCENE_HPP