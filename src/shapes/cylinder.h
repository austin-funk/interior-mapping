#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "3DShape.h"

class Cylinder: public ThreeDShape
{
public:
    void updateParams(int param1, int param2);
    std::vector<float> generateShape() { return m_vertexData; }

private:
    enum NormType {
        Face,
        Side
    };

    void insertVec3(std::vector<float> &data, glm::vec3 v);
    void setVertexData();
    void makeCylinder();
    void makeTile(glm::vec3 topLeft,
                  glm::vec3 topRight,
                  glm::vec3 bottomLeft,
                  glm::vec3 bottomRight,
                  NormType norm);
    void makeSide(float currTheta, float nextTheta);
    glm::vec3 cylToCartesian(glm::vec3 cyl);

    std::vector<float> m_vertexData;
    int m_param1;
    int m_param2;
    float m_radius = 0.5;
};
