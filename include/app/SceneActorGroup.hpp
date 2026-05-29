#ifndef SCENE_ACTOR_GROUP_HPP
#define SCENE_ACTOR_GROUP_HPP

#include <memory>
#include <vector>

#include "Util/GameObject.hpp"
#include "Util/Renderer.hpp"

class SceneActorGroup {
public:
    SceneActorGroup& Add(const std::shared_ptr<Util::GameObject>& actor) {
        if (actor != nullptr) {
            m_Actors.push_back(actor);
        }
        return *this;
    }

    void AddTo(Util::Renderer& root) const {
        for (const auto& actor : m_Actors) {
            root.AddChild(actor);
        }
    }

    void RemoveFrom(Util::Renderer& root) const {
        for (auto it = m_Actors.rbegin(); it != m_Actors.rend(); ++it) {
            root.RemoveChild(*it);
        }
    }

private:
    std::vector<std::shared_ptr<Util::GameObject>> m_Actors;
};

#endif // SCENE_ACTOR_GROUP_HPP
