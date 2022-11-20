#include "sphere.h"

void Sphere::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    setVertexData();
}

void Sphere::makeTile(glm::vec3 topLeft,
                      glm::vec3 topRight,
                      glm::vec3 bottomLeft,
                      glm::vec3 bottomRight) {
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, bottomRight);
    // right triangle
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, topRight);
}

float convertX(float r, float phi, float theta) {
    return r * glm::sin(phi) * glm::sin(theta);
}

float convertY(float r, float phi, float theta) {
    return r * glm::cos(phi);
}

float convertZ(float r, float phi, float theta) {
    return r * glm::sin(phi) * glm::cos(theta);
}

void Sphere::makeWedge(float currentTheta, float nextTheta) {
    // phi ranges from 0 to 180 degrees, (180 / m_param1) * i
    float actParam1 = (m_param1 > 1) ? m_param1 : 2;
    float topPhi, nextPhi, r = 0.5;
    glm::vec3 TL, TR, BL, BR;
    for (int i = 0; i < actParam1; i++) {
        topPhi = glm::radians((180.0f / actParam1) * i);
        nextPhi = glm::radians((180.0f / actParam1) * (i + 1));

        TL = {convertX(r, topPhi, currentTheta),
              convertY(r, topPhi, currentTheta),
              convertZ(r, topPhi, currentTheta)};
        TR = {convertX(r, topPhi, nextTheta),
              convertY(r, topPhi, nextTheta),
              convertZ(r, topPhi, nextTheta)};
        BL = {convertX(r, nextPhi, currentTheta),
              convertY(r, nextPhi, currentTheta),
              convertZ(r, nextPhi, currentTheta)};
        BR = {convertX(r, nextPhi, nextTheta),
              convertY(r, nextPhi, nextTheta),
              convertZ(r, nextPhi, nextTheta)};

        makeTile(TL, TR, BL, BR);
    }

}

void Sphere::makeSphere() {
    float actParam2 = (m_param2 > 2) ? m_param2 : 3;
    float thetaStep = glm::radians(360.f / actParam2);
    for (int i = 0; i < actParam2; i++) {
        makeWedge(i * thetaStep, (i + 1) * thetaStep);
    }
}

void Sphere::setVertexData() {
    makeSphere();
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Sphere::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
