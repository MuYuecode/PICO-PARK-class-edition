//
// Created by cody2 on 2026/3/28.
//

#include "physics/body/StaticBody.hpp"

StaticBody::StaticBody(glm::vec2 center, glm::vec2 halfSize, BodyType type)
    : m_Position(center)
    , m_HalfSize(glm::abs(halfSize))
    , m_Traits{type, false, false} {}

