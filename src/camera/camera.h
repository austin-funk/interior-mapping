#pragma once

#include "utils/scenedata.h"
#include <glm/glm.hpp>

class Camera {
private:
    int width;
    int height;
    glm::vec4 pos;
    glm::vec4 look;
    glm::vec4 up;
    float heightAngle;
    float aperture;
    float focalLength;
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 invView = glm::mat4(1.0f);
    glm::mat4 proj = glm::mat4(1.0f);

public:
    // Returns the view matrix for the current camera settings.
    Camera(int wid, int hei, SceneCameraData &camera);

    // View matrix functions
    void setViewMatrix();
    void setInverseViewMatrix();
    glm::mat4 getViewMatrix();
    glm::mat4 getInverseViewMatrix();

    // Perspective matrix functions, c = -near/far, already have heightAngle and can get widthAngle from ratio
    void setPerspectiveMatrix(float near, float far);
    glm::mat4 getPerspectiveMatrix();

    // Returns the aspect ratio of the camera.
    float getAspectRatio() const;

    // Returns the height angle of the camera in RADIANS.
    float getHeightAngle() const;

    // Returns the position of the camera in camera space
    glm::vec4 getEye() const;

    // Returns the focal length of this camera.
    // This is for the depth of field extra-credit feature only;
    // You can ignore if you are not attempting to implement depth of field.
    float getFocalLength() const;

    // Returns the focal length of this camera.
    // This is for the depth of field extra-credit feature only;
    // You can ignore if you are not attempting to implement depth of field.
    float getAperture() const;
};

