#ifndef SCENE_SERVICES_HPP
#define SCENE_SERVICES_HPP

#include "services/IAudioService.hpp"
#include "services/IGlobalActors.hpp"
#include "services/ISessionState.hpp"
#include "services/IVisualThemeService.hpp"

struct SceneServices {
    IAudioService& Audio;
    IVisualThemeService& Theme;
    ISessionState& Session;
    IGlobalActors& Actors;
};

#endif // SCENE_SERVICES_HPP


