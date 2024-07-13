#pragma once
#include "glm/glm.hpp"

struct GfxObject
{
    glm::mat4 globalTransform;
};

struct ObjectID
{
    size_t id;
};
