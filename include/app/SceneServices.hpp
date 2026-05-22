#ifndef SCENE_SERVICES_HPP
#define SCENE_SERVICES_HPP

#include "systems/IAudioService.hpp"
#include "systems/IGlobalActors.hpp"
#include "systems/ISessionState.hpp"
#include "systems/IVisualThemeService.hpp"

struct SceneServices {
    IAudioService& Audio;
    IVisualThemeService& Theme;
    ISessionState& Session;
    IGlobalActors& Actors;
};

#endif // SCENE_SERVICES_HPP


