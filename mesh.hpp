#pragma once

#include <vector>
#include "libs/glm/glm.hpp"

struct Mesh
{
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    int VertexCount() { return static_cast<int>(vertices.size() / 3); }
};