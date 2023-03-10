#include "camera.h"
#include "utils/scenedata.h"

#include <stdexcept>

Camera::Camera(int wid, int hei, SceneCameraData &camera) {
    width = wid;
    height = hei;
    pos = camera.pos;
    look = camera.look;
    up = camera.up;
    heightAngle = camera.heightAngle;
    aperture = camera.aperture;
    focalLength = camera.focalLength;
}

glm::mat4 Camera::getViewMatrix() {
    return view;
}

void Camera::setViewMatrix() {
    // copied from lab 4
    glm::vec3 look3(look);
    glm::vec3 up3(up);

    glm::vec3 w3 = -glm::normalize(look3);
    glm::vec4 w = glm::vec4(w3, 0);


    glm::vec3 part_of_v = up3 - glm::dot(up3, w3) * w3;
    glm::vec3 v3 = glm::normalize(part_of_v);
    glm::vec4 v = glm::vec4(v3, 0);

    glm::vec3 u3 = cross(v3, w3);
    glm::vec4 u = glm::vec4(u3, 0);

    glm::vec4 last_col = glm::vec4(0, 0, 0, 1);

    glm::mat4 translation = glm::mat4(1, 0, 0, 0,
                                      0, 1, 0, 0,
                                      0, 0, 1, 0,
                                      -pos.x, -pos.y, -pos.z, 1);

    view = glm::transpose(glm::mat4(u, v, w, last_col)) * translation;
}

void Camera::setInverseViewMatrix() {
    invView = glm::inverse(view);
}

glm::mat4 Camera::getInverseViewMatrix() {
    return invView;
}

glm::mat4 Camera::getPerspectiveMatrix() {
    return proj;
}

void Camera::setPerspectiveMatrix(float near, float far) {
    float L = 1 / (far * tan(heightAngle / 2));
    float widAngle = 2 * atan(getAspectRatio() * tan(heightAngle / 2));
    float N = 1 / (far * tan(widAngle / 2));
    glm::mat4 scale = glm::mat4(N, 0, 0, 0,
                                0, L, 0, 0,
                                0, 0, 1 / far, 0,
                                0, 0, 0, 1);

    float c = -near / far;
    glm::mat4 Mpp = glm::mat4(1, 0, 0, 0,
                              0, 1, 0, 0,
                              0, 0, 1 / (1 + c), -1,
                              0, 0, -c / (1 + c), 0);

    glm::mat4 zMap = glm::mat4(1, 0, 0, 0,
                               0, 1, 0, 0,
                               0, 0, -2, 0,
                               0, 0, -1, 1);

    proj = zMap * Mpp * scale;
}

glm::vec4 Camera::getLook() {
    return look;
}

glm::vec4 Camera::getUp() {
    return up;
}

void Camera::changePos(glm::vec4 change) {
    pos += change;
    setViewMatrix();
    setInverseViewMatrix();
}

void Camera::replaceLook(glm::vec4 newLook) {
    look = newLook;
}

void Camera::replaceUp(glm::vec4 newUp) {
    up = newUp;
}

float Camera::getAspectRatio() const {
    return (height != 0) ? (float)width / (float)height : 0;
}

float Camera::getHeightAngle() const {
    return heightAngle;
}

glm::vec4 Camera::getEye() const {
    return pos;
}

void Camera::setWidth(int w) {
    width = w;
}

void Camera::setHeight(int h) {
    height = h;
}

float Camera::getFocalLength() const {
    return focalLength;
}

float Camera::getAperture() const {
    return aperture;
}

