#pragma once

#include "utils/scenedata.h"
#include <glm/glm.hpp>

// A class representing a virtual camera.

// Feel free to make your own design choices for Camera class, the functions below are all optional / for your convenience.
// You can either implement and use these getters, or make your own design.
// If you decide to make your own design, feel free to delete these as TAs won't rely on them to grade your assignments.

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
    glm::mat4 proj = glm::mat4(1.0f);

public:
    // Returns the view matrix for the current camera settings.
    Camera(int wid, int hei, SceneCameraData &camera);
    // You might also want to define another function that return the inverse of the view matrix.
    void setViewMatrix();
    glm::mat4 getViewMatrix();
    glm::mat4 getInverseViewMatrix();

    // perspective matrix, c = -near/far, already have heightAngle and can get widthAngle from ratio
    void setPerspectiveMatrix(float near, float far);
    glm::mat4 getPerspectiveMatrix();

    // Returns the aspect ratio of the camera.
    float getAspectRatio() const;

    // Returns the height angle of the camera in RADIANS.
    float getHeightAngle() const;

    // TODO
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

