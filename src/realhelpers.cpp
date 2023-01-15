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
    glm::mat4 inv = glm::transpose(glm::inverse(model));
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

void Realtime::sendLightType(GLuint &shader, const char* name, int i, bool &inc) {
    int type;
    switch (m_metaData.lights.at(i).type) {
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

void Realtime::sendLightData(GLuint &shader) {
    // Initial values
    GLint intLocationNumLights = glGetUniformLocation(shader, "numLights");
    int numLight = 0;
    bool inc = true;

    // Loop through lights
    for (int i = 0; i < m_metaData.lights.size(); ++i) {
        if (numLight > 7) {
            break;
        }

        auto light = "lights[" + std::to_string(numLight) + "]";

        // light type
        sendLightType(shader, (light + ".type").c_str(), i, inc);
        if (!inc) {
            continue;
        }
        // light pos
        sendVec4(shader, (light + ".pos").c_str(), m_metaData.lights.at(i).pos);
        // light dir
        sendVec4(shader, (light + ".dir").c_str(), m_metaData.lights.at(i).dir);
        // light color
        sendVec4(shader, (light + ".color").c_str(), m_metaData.lights.at(i).color);
        // point coeffs
        sendVec3(shader, (light + ".point_coeffs").c_str(), m_metaData.lights.at(i).function);
        // light angle
        sendFloat(shader, (light + ".angle").c_str(), m_metaData.lights.at(i).angle);
        // light penumbra
        sendFloat(shader, (light + ".penumbra").c_str(), m_metaData.lights.at(i).penumbra);

        numLight++;
    }

    // set numLights
    if (intLocationNumLights != -1) {
        glUniform1i(intLocationNumLights, numLight);
    }
}

// Send material data
void Realtime::sendMaterialData(GLuint &shader, int i) {
    sendVec4(shader, (const char*) "material_ambO", m_metaData.shapes.at(i).primitive.material.cAmbient);
    sendVec4(shader, (const char*) "material_difO", m_metaData.shapes.at(i).primitive.material.cDiffuse);
    sendVec4(shader, (const char*) "material_specO", m_metaData.shapes.at(i).primitive.material.cSpecular);
    sendFloat(shader, (const char*) "shiny", m_metaData.shapes.at(i).primitive.material.shininess);
}

// Send k factors
void Realtime::sendKStuff(GLuint &shader) {
    sendFloat(shader, (const char*) "k_a", m_metaData.globalData.ka);
    sendFloat(shader, (const char*) "k_d", m_metaData.globalData.kd);
    sendFloat(shader, (const char*) "k_s", m_metaData.globalData.ks);
}
