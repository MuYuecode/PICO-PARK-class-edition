#ifndef EXIT_CONFIRM_SCENE_HPP
#define EXIT_CONFIRM_SCENE_HPP

#include "Scene.hpp"
#include "Character.hpp"
#include "GameText.hpp"

class ExitConfirmScene : public Scene {
public:
    explicit ExitConfirmScene(SceneServices services);
    ~ExitConfirmScene() override = default;

    void   OnEnter() override;
    void   OnExit()  override;
    void Update()  override;

private:
    std::shared_ptr<GameText>  m_ExitGame1Text;
    std::shared_ptr<GameText>  m_YesText;
    std::shared_ptr<GameText>  m_NoText;
    std::shared_ptr<Character> m_ChoiceFrame;

    std::shared_ptr<Character> m_MenuFrame;
    std::shared_ptr<Character> m_ExitGameButton;


    bool m_IsYesSelected = true;

    void UpdateChoiceFramePosition() const;
};

#endif // EXIT_CONFIRM_SCENE_HPP