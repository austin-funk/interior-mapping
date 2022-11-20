#include "Cube.h"
#include <iostream>

void Cube::updateParams(int param1) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    setVertexData();
}

void Cube::makeTile(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {
    // left triangle
    glm::vec3 normal = glm::normalize(glm::cross(bottomRight - bottomLeft, topLeft - bottomLeft));
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normal);
    // right triangle
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, normal);
}

void Cube::makeFace(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {
    glm::vec3 diffRight = glm::normalize(topRight - topLeft);
    glm::vec3 diffDown = glm::normalize(topLeft - bottomLeft);

    glm::vec3 normal = glm::normalize(glm::cross(bottomRight - bottomLeft, topLeft - bottomLeft));

    for (int i = 0; i < m_param1; i++) {
        for (int j = 0; j < m_param1; j++) {
            glm::vec3 newTL, newTR, newBL, newBR, rightChange, downChange;

            rightChange = abs(glm::normalize(topLeft * diffRight)) * (1.0f / m_param1);
            downChange = abs(glm::normalize(topLeft * diffDown)) * (1.0f / m_param1);

            newTL = ((topLeft * diffRight) +
                     ((float)j * rightChange)) + ((topLeft * diffDown) - ((float)i * downChange)) + (normal * topLeft);
            newTR = ((topLeft * diffRight) +
                     ((float)(j + 1) * rightChange)) + ((topLeft * diffDown) - ((float)i * downChange)) + (normal * topLeft);
            newBL = ((topLeft * diffRight) +
                     ((float)j * rightChange)) + ((topLeft * diffDown) - ((float)(i + 1) * downChange)) + (normal * topLeft);
            newBR = ((topLeft * diffRight) +
                     ((float)(j + 1) * rightChange)) + ((topLeft * diffDown) - ((float)(i + 1) * downChange)) + (normal * topLeft);

            makeTile(newTL, newTR, newBL, newBR);
        }
    }


}

void Cube::setVertexData() {
    // front face
     makeFace(glm::vec3(-0.5f,  0.5f, 0.5f),
              glm::vec3( 0.5f,  0.5f, 0.5f),
              glm::vec3(-0.5f, -0.5f, 0.5f),
              glm::vec3( 0.5f, -0.5f, 0.5f));
     // left face
     makeFace(glm::vec3(-0.5f,  0.5f, 0.5f),
              glm::vec3(-0.5f,  0.5f, -0.5f),
              glm::vec3(-0.5f, -0.5f, 0.5f),
              glm::vec3(-0.5f, -0.5f, -0.5f));
     // back face
     makeFace(glm::vec3(0.5f,  0.5f, -0.5f),
              glm::vec3(0.5f, -0.5f, -0.5f),
              glm::vec3(-0.5f, 0.5f, -0.5f),
              glm::vec3(-0.5f, -0.5f, -0.5f));
     // right face
     makeFace(glm::vec3(0.5f,  -0.5f, 0.5f),
              glm::vec3(0.5f,  0.5f, 0.5f),
              glm::vec3(0.5f, -0.5f, -0.5f),
              glm::vec3(0.5f, 0.5f, -0.5f));
     // bottom face
     makeFace(glm::vec3( 0.5f, -0.5f, 0.5f),
              glm::vec3(-0.5f,  -0.5f, 0.5f),
              glm::vec3( 0.5f, -0.5f, -0.5f),
              glm::vec3(-0.5f, -0.5f, -0.5f));
     // top face
     makeFace(glm::vec3(-0.5f, 0.5f, 0.5f),
              glm::vec3(-0.5f, 0.5f, -0.5f),
              glm::vec3(0.5f, 0.5f, 0.5f),
              glm::vec3(0.5f, 0.5f, -0.5f));
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cube::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
