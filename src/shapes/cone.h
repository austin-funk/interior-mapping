#pragma once

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include "3DShape.h"

class Cone: public ThreeDShape
{
public:
    void updateParams(int param1, int param2);
    std::vector<float> generateShape() { return m_vertexData; }

private:
    enum NormType {
        Tip,
        Face,
        Side
    };

    void insertVec3(std::vector<float> &data, glm::vec3 v);
    void setVertexData();
    void makeTile(glm::vec3 topLeft,
                  glm::vec3 topRight,
                  glm::vec3 bottomLeft,
                  glm::vec3 bottomRight,
                  NormType norm);
    glm::vec3 cylToCartesian(glm::vec3 cyl);
    void makeSide(float currentTheta, float nextTheta);
    void makeCylinder();

    std::vector<float> m_vertexData;
    int m_param1;
    int m_param2;
    float m_radius = 0.5;
};
