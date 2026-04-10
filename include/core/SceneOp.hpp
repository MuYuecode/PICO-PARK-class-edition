#ifndef SCENE_OP_HPP
#define SCENE_OP_HPP

#include "SceneId.hpp"

enum class SceneOpType {
    None,
    PushOverlay,
    PopOverlay,
    RestartUnderlying,
    ClearToAndGoTo,
};

struct SceneOp {
    SceneOpType type = SceneOpType::None;
    SceneId     target = SceneId::None;
};

#endif // SCENE_OP_HPP


