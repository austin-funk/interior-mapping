#include "glm/ext/matrix_transform.hpp"
#include "realtime.h"
#include "settings.h"
#include <iostream>

// Deletes given VBO and VAO
void Realtime::killShape(GLuint shapeb, GLuint shapea) {
    glDeleteBuffers(1, &shapeb);
    glDeleteVertexArrays(1, &shapea);
}

// functions for axis rotation
glm::mat3 Realtime::rotateAboutAxis(glm::vec3 axis, float theta) {
    glm::mat3 rodrigues = glm::mat3(1.0f);

    float costheta = cos(theta);
    float sintheta = sin(theta);

    axis = glm::normalize(axis);

    // r, c
    rodrigues = {costheta + (powf(axis.x, 2) * (1 - costheta)), // 0,0
                (axis.x * axis.y * (1 - costheta)) + (axis.z * sintheta), // 1,0
                (axis.x * axis.z * (1 - costheta)) - (axis.y * sintheta), // 2,0
                (axis.x * axis.y * (1 - costheta)) - (axis.z * sintheta), // 0,1
                 costheta + (powf(axis.y, 2) * (1 - costheta)), // 1,1
                (axis.y * axis.z * (1 - costheta)) + (axis.x * sintheta), // 2,1
                (axis.x * axis.z * (1 - costheta)) + (axis.y * sintheta), // 0,2
                (axis.y * axis.z * (1 - costheta)) - (axis.x * sintheta), // 1,2
                 costheta + (powf(axis.z, 2) * (1 - costheta))}; // 2,2

    return rodrigues;
}

glm::mat3 Realtime::rotateAboutYAxis(float theta) {
    glm::mat3 rodrigues = glm::mat3(1.0f);

    float costheta = cos(theta);
    float sintheta = sin(theta);

    rodrigues = {costheta, 0, -sintheta, // column 1
                0, 1, 0, // column 2
                sintheta, 0, costheta}; // column 3

    return rodrigues;
}

// Create VBO and VAO for given shape
void Realtime::generateShape(ThreeDShape &shape, GLuint &vbo, GLuint &vao, std::vector<GLfloat> &data) {
    // CONE
    // Generate and bind VBO
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // Generate sphere data
    shape.updateParams(settings.shapeParameter1, settings.shapeParameter2);
    data = shape.generateShape(); // confusing, different function
    // Send data to VBO
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat), data.data(), GL_STATIC_DRAW);
    // Generate, and bind vao
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Enable and define attribute 0 to store vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void *>(0 * sizeof(GLfloat)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void *>(3 * sizeof(GLfloat)));

    // Clean-up bindings
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// Send matrix
void Realtime::sendMatrix(GLuint &shader, const GLchar *matName, glm::mat4 &mat) {
    GLint matrixLocation = glGetUniformLocation(shader, matName);
    if (matrixLocation != -1) {
        glUniformMatrix4fv(matrixLocation, 1, GL_FALSE, &mat[0][0]);
    }
}

void Realtime::sendAllMatrices(glm::mat4 &model, glm::mat4 &view, glm::mat4 &proj, GLuint &shader) {
    // model_mat uniform
    const GLchar *modelName = "model_mat";
    sendMatrix(shader, modelName, model);

    // inverse model matrix uniform
    glm::mat4 inv(glm::inverse(glm::transpose(glm::mat3(model))));
    const GLchar *invModelName = "inv_tran_model_mat";
    sendMatrix(shader, invModelName, inv);

    // view_mat uniform
    const GLchar *viewName = "view_mat";
    sendMatrix(shader, viewName, view);

    // projection_mat uniform
    const GLchar *projName = "projection_mat";
    sendMatrix(shader, projName, proj);
}

// Light stuff
void Realtime::sendVec3(GLuint &shader, const char* name, glm::vec3 &data) {
    GLint vectorLocation = glGetUniformLocation(shader, name);
    if (vectorLocation != -1) {
        glUniform3fv(vectorLocation, 1, &data[0]);
    }
}

void Realtime::sendVec4(GLuint &shader, const char* name, glm::vec4 &data) {
    GLint vectorLocation = glGetUniformLocation(shader, name);
    if (vectorLocation != -1) {
        glUniform4fv(vectorLocation, 1, &data[0]);
    }
}

void Realtime::sendInt(GLuint &shader, const char* name, int &data) {
    GLint vectorLocation = glGetUniformLocation(shader, name);
    if (vectorLocation != -1) {
        glUniform1i(vectorLocation, data);
    }
}

void Realtime::sendFloat(GLuint &shader, const char* name, float &data) {
    GLint vectorLocation = glGetUniformLocation(shader, name);
    if (vectorLocation != -1) {
        glUniform1f(vectorLocation, data);
    }
}

void Realtime::sendLightType(GLuint &shader, const char* name, LightType lt, bool &inc) {
    int type;
    switch (lt) {
    case LightType::LIGHT_DIRECTIONAL:
        type = 0;
        break;
    case LightType::LIGHT_POINT:
        type = 1;
        break;
    case LightType::LIGHT_SPOT:
        type = 2;
        break;
    default:
        type = 3; // check if 3, means unsupported light
        inc = false;
        break;
    }
    sendInt(shader, name, type);
}

void Realtime::createPointLight(SceneLightData &l, int &id, glm::vec3 &pos) {
    l.id = id;
    l.type = LightType::LIGHT_POINT;
    l.pos = glm::vec4(pos, 1);
    l.dir = glm::vec4(-pos, 0);
    l.color = glm::vec4(1, 1, 1, 1);
}

void Realtime::sendLightData(GLuint &shader, std::vector<SceneLightData> &lights) {
    // Initial values
    GLint intLocationNumLights = glGetUniformLocation(shader, "numLights");
    int numLight = 0;
    bool inc = true;

    // Loop through lights
    for (int i = 0; i < lights.size(); ++i) {
        std::cout << "light" << i << std::endl;
        if (numLight > 7) {
            break;
        }

        auto light = "lights[" + std::to_string(numLight) + "]";

        // light type
        sendLightType(shader, (light + ".type").c_str(), lights.at(i).type, inc);
        if (!inc) {
            continue;
        }
        // light pos
        sendVec4(shader, (light + ".pos").c_str(), lights.at(i).pos);
        // light dir
        sendVec4(shader, (light + ".dir").c_str(), lights.at(i).dir);
        // light color
        sendVec4(shader, (light + ".color").c_str(), lights.at(i).color);
        // point coeffs
        sendVec3(shader, (light + ".point_coeffs").c_str(), lights.at(i).function);
        // light angle
        sendFloat(shader, (light + ".angle").c_str(), lights.at(i).angle);
        // light penumbra
        sendFloat(shader, (light + ".penumbra").c_str(), lights.at(i).penumbra);

        numLight++;
    }

    // set numLights
    if (intLocationNumLights != -1) {
        glUniform1i(intLocationNumLights, numLight);
    }
}

// Send material data
void Realtime::sendMaterialData(GLuint &shader, SceneMaterial material) {
    sendVec4(shader, (const char*) "material_ambO", material.cAmbient);
    sendVec4(shader, (const char*) "material_difO", material.cDiffuse);
    sendVec4(shader, (const char*) "material_specO", material.cSpecular);
    sendFloat(shader, (const char*) "shiny", material.shininess);
}

// Send global data
void Realtime::sendGlobalData(GLuint &shader, SceneGlobalData globalData) {
    sendFloat(shader, (const char*) "k_a", globalData.ka);
    sendFloat(shader, (const char*) "k_d", globalData.kd);
    sendFloat(shader, (const char*) "k_s", globalData.ks);
}

// Specific to building
void Realtime::sendBuildingLights(GLuint &shader) {
    SceneLightData l1, l2, l3;
    int id0 = 0, id1 = 1, id2 = 2;
    glm::vec3 pos1(10, 10, 0), pos2(0, 10, 10), pos3(0, 10, 0);
    createPointLight(l1, id0, pos1);
    createPointLight(l2, id1, pos2);
    createPointLight(l3, id2, pos3);

    std::vector<SceneLightData> lightsToSend = std::vector{l1, l2, l3};
    sendLightData(shader, lightsToSend);
}

void Realtime::sendBuildingMaterial(GLuint &shader) {
    SceneMaterial buildingMaterial;
    buildingMaterial.cAmbient = glm::vec4(1, 0, 0, 1);
    buildingMaterial.cDiffuse = glm::vec4(0, 1, 0, 1);
    buildingMaterial.cSpecular = glm::vec4(1, 1, 1, 1);
    buildingMaterial.shininess = 20;

    sendMaterialData(shader, buildingMaterial);
}

void Realtime::sendBuildingGlobalData(GLuint &shader) {
    SceneGlobalData buildingGlobalData;
    buildingGlobalData.ka = 0.5f;
    buildingGlobalData.kd = 0.5f;
    buildingGlobalData.ks = 0.5f;

    sendGlobalData(shader, buildingGlobalData);
}

void Realtime::drawBuilding(GLuint &shader, glm::mat4 &view, glm::mat4 &proj, glm::vec4 &camPos) {
    glUseProgram(shader);

    glBindVertexArray(m_cube_vao);

    GLint vecLocation_camPos = glGetUniformLocation(shader, "camera_pos");
    if (vecLocation_camPos != -1) {
        glUniform4fv(vecLocation_camPos, 1, &camPos[0]);
    }

    sendBuildingMaterial(shader);

    // build ctm
    glm::mat4 ctm(1.0f);
    ctm = glm::scale(ctm, glm::vec3(2, 5, 2));
    ctm = glm::translate(ctm, glm::vec3(0, 1, 0));

    // draw stuff
    float drawSize = m_cubeData.size();

    sendAllMatrices(ctm, view, proj, shader);

    glDrawArrays(GL_TRIANGLES, 0, drawSize / 6);

    glBindVertexArray(0);

    glUseProgram(0);
}
