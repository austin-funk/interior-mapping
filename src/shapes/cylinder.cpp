#include "cylinder.h"
#include "glm/gtc/constants.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <iostream>

void Cylinder::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    setVertexData();
}

void Cylinder::makeTile(glm::vec3 topLeft,
                        glm::vec3 topRight,
                        glm::vec3 bottomLeft,
                        glm::vec3 bottomRight,
                        NormType norm) {
    glm::vec3 TLnorm, TRnorm, BLnorm, BRnorm;
    glm::vec3 TLcart = cylToCartesian(topLeft); glm::vec3 TRcart = cylToCartesian(topRight);
    glm::vec3 BLcart = cylToCartesian(bottomLeft); glm::vec3 BRcart = cylToCartesian(bottomRight);

    switch (norm) {
    case Side:
    {
        TLnorm = glm::normalize(glm::vec3{TLcart.x, 0, TLcart.z});
        TRnorm = glm::normalize(glm::vec3{TRcart.x, 0, TRcart.z});
        BLnorm = glm::normalize(glm::vec3{BLcart.x, 0, BLcart.z});
        BRnorm = glm::normalize(glm::vec3{BRcart.x, 0, BRcart.z});
        break;
    }
    case Face:
    {
        TLnorm = TRnorm = BLnorm = BRnorm =
                glm::normalize(glm::cross(TLcart - BLcart, BRcart - BLcart));
        break;
    }
    default:
        break;
    }

    // left triangle
    insertVec3(m_vertexData, TLcart);
    insertVec3(m_vertexData, TLnorm);
    insertVec3(m_vertexData, BRcart);
    insertVec3(m_vertexData, BRnorm);
    insertVec3(m_vertexData, BLcart);
    insertVec3(m_vertexData, BLnorm);


    // right triangle
    insertVec3(m_vertexData, TLcart);
    insertVec3(m_vertexData, TLnorm);
    insertVec3(m_vertexData, TRcart);
    insertVec3(m_vertexData, TRnorm);
    insertVec3(m_vertexData, BRcart);
    insertVec3(m_vertexData, BRnorm);

}

glm::vec3 Cylinder::cylToCartesian(glm::vec3 cyl) {
    glm::vec3 cartesian;
    // cyl: r, theta, z
    cartesian.x = cyl.x * cos(cyl.y);
    cartesian.y = cyl.z;
    cartesian.z = cyl.x * sin(cyl.y);
    return cartesian;
}

void Cylinder::makeSide(float currentTheta, float nextTheta) {
    float topZ, nextZ, r = m_radius, zStep = 1.0f / m_param1, firstZ = 0.5;
    glm::vec3 TL, TR, BL, BR;
    // side
    for (int i = 0; i < m_param1; i++) {
        topZ = firstZ - ((float)i * zStep);
        nextZ = topZ - zStep;
        TL = {r, currentTheta, topZ};
        TR = {r, nextTheta, topZ};
        BL = {r, currentTheta, nextZ};
        BR = {r, nextTheta, nextZ};
        makeTile(TL, TR, BL, BR, Side);
    }
    // top
    float curR, nextR, rStep = 0.5f / m_param1;
    for (int i = 0; i < m_param1; i++) {
        curR = (float)i * rStep;
        nextR = (float)(i + 1) * rStep;
        TL = {curR, currentTheta, firstZ};
        TR = {curR, nextTheta, firstZ};
        BL = {nextR, currentTheta, firstZ};
        BR = {nextR, nextTheta, firstZ};
        makeTile(TL, TR, BL, BR, Face);
    }

    // bottom
    for (int i = 0; i < m_param1; i++) {
        curR = (float)i * rStep;
        nextR = (float)(i + 1) * rStep;
        TL = {curR, currentTheta, -firstZ};
        TR = {curR, nextTheta, -firstZ};
        BL = {nextR, currentTheta, -firstZ};
        BR = {nextR, nextTheta, -firstZ};
        makeTile(BR, TR, BL, TL, Face);
    }

}

void Cylinder::makeCylinder() {
    int actParam2 = (m_param2 > 2) ? m_param2 : 3;
    float thetaStep = glm::radians(360.f / actParam2);
    for (int i = 0; i < actParam2; i++) {
        makeSide(i * thetaStep, (i + 1) * thetaStep);
    }
}

void Cylinder::setVertexData() {
    makeCylinder();
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cylinder::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
