#ifndef SCENE_SERVICES_HPP
#define SCENE_SERVICES_HPP

#include "IAudioService.hpp"
#include "IGlobalActors.hpp"
#include "ISessionState.hpp"
#include "IVisualThemeService.hpp"

struct SceneServices {
    IAudioService& Audio;
    IVisualThemeService& Theme;
    ISessionState& Session;
    IGlobalActors& Actors;
};

#endif // SCENE_SERVICES_HPP

