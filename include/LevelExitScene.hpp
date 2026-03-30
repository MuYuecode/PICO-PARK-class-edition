#ifndef LEVEL_EXIT_SCENE_HPP
#define LEVEL_EXIT_SCENE_HPP

#include <array>
#include <memory>

#include "Character.hpp"
#include "GameText.hpp"
#include "Scene.hpp"

class LevelExitScene : public Scene {
public:
    explicit LevelExitScene(SceneServices services);
    ~LevelExitScene() override = default;

    void OnEnter() override;
    void OnExit() override;
    SceneId Update() override;

private:
    static constexpr int ROW_COUNT = 4;
    static constexpr float kTextX = 0.0f;
    static constexpr std::array<float, ROW_COUNT> kRowY = {70.0f, -5.0f, -80.0f, -155.0f};

    std::shared_ptr<Character> m_DimBg;
    std::shared_ptr<Character> m_ExitFrame;
    std::shared_ptr<Character> m_ExitButton;
    std::shared_ptr<Character> m_BlueCatRunIcon;
    std::shared_ptr<Character> m_ChoiceFrame;

    std::shared_ptr<GameText> m_ReturnGameText;
    std::shared_ptr<GameText> m_RetryText;
    std::shared_ptr<GameText> m_LevelSelectText;
    std::shared_ptr<GameText> m_TitleText;

    int m_SelectedRow = 0;
    bool m_WaitEscRelease = true;

    void UpdateChoiceFrame() const;
    void ConfirmSelection();
};

#endif // LEVEL_EXIT_SCENE_HPP

