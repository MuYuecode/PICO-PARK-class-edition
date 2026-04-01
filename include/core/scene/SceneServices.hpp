#ifndef SCENE_SERVICES_HPP
#define SCENE_SERVICES_HPP

#include "services/audio/IAudioService.hpp"
#include "services/session/IGlobalActors.hpp"
#include "services/session/ISessionState.hpp"
#include "services/theme/IVisualThemeService.hpp"

struct SceneServices {
    IAudioService& Audio;
    IVisualThemeService& Theme;
    ISessionState& Session;
    IGlobalActors& Actors;
};

#endif // SCENE_SERVICES_HPP


