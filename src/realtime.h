#pragma once

// Defined before including GLEW to suppress deprecation messages on macOS
#include "utils/sceneparser.h"
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>

class Realtime : public QOpenGLWidget
{
public:
    Realtime(QWidget *parent = nullptr);
    void finish();                                      // Called on program exit
    void sceneChanged();
    void settingsChanged();

public slots:
    void tick(QTimerEvent* event);                      // Called once per tick of m_timer

protected:
    void initializeGL() override;                       // Called once at the start of the program
    void paintGL() override;                            // Called whenever the OpenGL context changes or by an update() request
    void resizeGL(int width, int height) override;      // Called when window size changes

private:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

    // Tick Related Variables
    int m_timer;                                        // Stores timer which attempts to run ~60 times per second
    QElapsedTimer m_elapsedTimer;                       // Stores timer which keeps track of actual time between frames

    // Input Related Variables
    bool m_mouseDown = false;                           // Stores state of left mouse button
    glm::vec2 m_prev_mouse_pos;                         // Stores mouse position
    std::unordered_map<Qt::Key, bool> m_keyMap;         // Stores whether keys are pressed or not

    // Device Correction Variables
    int m_devicePixelRatio;

    // my methods
    void paintScene();

    // metadata
    RenderData m_metaData;
    Camera m_camera = Camera(100, 100, m_metaData.cameraData);
    glm::mat3 rotateAboutAxis(glm::vec3 axis, float theta);
    glm::mat3 rotateAboutYAxis(float theta);

    // Shader stuff
    GLuint m_phong_shader;
    GLuint m_texture_shader;

    // FBO stuff
    void makeFBO();
    void paintTexture(GLuint texture);
    int m_fbo_width;
    int m_fbo_height;
    GLuint m_fbo;
    GLuint m_fbo_texture;
    GLuint m_fbo_renderbuffer;
    GLuint m_defaultFBO;
    GLuint m_fullscreen_vbo;
    GLuint m_fullscreen_vao;

    // VBO/VAO for shapes
    GLuint m_cone_vbo; // Stores id of vbo
    GLuint m_cube_vbo; // Stores id of vbo
    GLuint m_cyl_vbo; // Stores id of vbo
    GLuint m_sphere_vbo; // Stores id of vbo
    GLuint m_cone_vao; // Stores id of vao
    GLuint m_cube_vao; // Stores id of vao
    GLuint m_cyl_vao; // Stores id of vao
    GLuint m_sphere_vao; // Stores id of vao
    std::vector<GLfloat> m_coneData; // object space data
    std::vector<GLfloat> m_cubeData;
    std::vector<GLfloat> m_cylData;
    std::vector<GLfloat> m_sphereData;
};
