#include "realtime.h"
#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "settings.h"
#include "utils/shaderloader.h"
#include "shapes/allshapes.h"

// ================== Project 5: Lights, Camera

Realtime::Realtime(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_prev_mouse_pos = glm::vec2(size().width()/2, size().height()/2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;

    // If you must use this function, do not edit anything above this
}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    // delete vbo/vao stuff
    killShape(m_cone_vbo, m_cone_vao);
    killShape(m_cube_vbo, m_cube_vao);
    killShape(m_cyl_vbo, m_cyl_vao);
    killShape(m_sphere_vbo, m_sphere_vao);

    // delete fbo stuff
    // glDeleteTextures(1, &m_kitten_texture);
    glDeleteTextures(1, &m_fbo_texture);
    glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
    glDeleteFramebuffers(1, &m_fbo);

    // delete program
    glDeleteProgram(m_phong_shader);
    glDeleteProgram(m_texture_shader);

    this->doneCurrent();
}


void Realtime::initializeGL() {
    m_devicePixelRatio = this->devicePixelRatio();

    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();

    // Initializing GL.
    // GLEW (GL Extension Wrangler) provides access to OpenGL functions.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    // Allows OpenGL to draw objects appropriately on top of one another
    glEnable(GL_DEPTH_TEST);
    // Tells OpenGL to only draw the front face
    glEnable(GL_CULL_FACE);
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Set defaults
    m_defaultFBO = 2;
    m_fbo_width = size().width();
    m_fbo_height = size().height();

    glClearColor(0, 0, 0, 1);

    m_phong_shader = ShaderLoader::createShaderProgram(":/resources/shaders/default.vert", ":/resources/shaders/default.frag");
    m_texture_shader = ShaderLoader::createShaderProgram(":/resources/shaders/texture.vert", ":/resources/shaders/texture.frag");

    Cone cone;
    Cube cube;
    Cylinder cyl;
    Sphere sphere;

    // CONE
    generateShape(cone, m_cone_vbo, m_cone_vao, m_coneData);

    // CUBE
    generateShape(cube, m_cube_vbo, m_cube_vao, m_cubeData);
    std::cout << "m_cubeData size: " << m_cubeData.size() << std::endl;

    // CYLINDER
    generateShape(cyl, m_cyl_vbo, m_cyl_vao, m_cylData);

    // SPHERE
    generateShape(sphere, m_sphere_vbo, m_sphere_vao, m_sphereData);

    glUseProgram(m_phong_shader);
    sendBuildingLights(m_phong_shader);
    sendBuildingGlobalData(m_phong_shader);
    glUseProgram(0);

    glUseProgram(m_texture_shader);
    GLint texture0Loc = glGetUniformLocation(m_texture_shader, "kitten");
    if (texture0Loc != -1) {
        glUniform1i(texture0Loc, 0);
    }


    // Task 11: Fix this "fullscreen" quad's vertex  data
    // Task 12: Play around with different values!
    // Task 13: Add UV coordinates
    std::vector<GLfloat> fullscreen_quad_data =
    { //     POSITIONS    //
        -1.0f,  1.0f, 0.0f,
         0.0f,  1.0f,//
        -1.0f, -1.0f, 0.0f,
         0.0f,  0.0f,//
         1.0f, -1.0f, 0.0f,
         1.0f,  0.0f,//
         1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,//
        -1.0f,  1.0f, 0.0f,
         0.0f,  1.0f,//
         1.0f, -1.0f, 0.0f,
         1.0f,  0.0f//
    };

    // Generate and bind a VBO and a VAO for a fullscreen quad
    glGenBuffers(1, &m_fullscreen_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_fullscreen_vbo);
    glBufferData(GL_ARRAY_BUFFER, fullscreen_quad_data.size()*sizeof(GLfloat), fullscreen_quad_data.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &m_fullscreen_vao);
    glBindVertexArray(m_fullscreen_vao);

    // Task 14: modify the code below to add a second attribute to the vertex attribute array
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<void *>(0 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<void *>(3 * sizeof(GLfloat)));

    // Unbind the fullscreen quad's VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glUseProgram(0);

    makeFBO();
}

void Realtime::paintScene() {
    // clear
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // define model, view, and projection matrices
    glm::mat4 model = glm::mat4(1.0f), view = glm::mat4(1.0f), proj = glm::mat4(1.0f);
    // size of vbo data
    int drawSize;
    // view matrix, reset every time settings/scene are changed
    view = m_camera.getViewMatrix();
    // assigns world space camera position
    float *camPos = &(m_camera.getEye())[0];//m_camera.getEye()
    // projection matrix, reset every time settings/scene are changed
    proj = m_camera.getPerspectiveMatrix();

    // if nothing in metadata, draw building
    if (m_metaData.shapes.size() == 0) {
        glm::vec4 p = m_camera.getEye();
        drawBuilding(m_phong_shader, view, proj, p);
        return;
    }

    glUseProgram(m_phong_shader);

    // iterate through every shape
    for (int i = 0; i < m_metaData.shapes.size(); ++i) {
        // set model matrix, resets with every shape
        model = m_metaData.shapes.at(i).ctm;

        // activate a different vao depending on the shape
        switch (m_metaData.shapes.at(i).primitive.type) {
        case PrimitiveType::PRIMITIVE_CONE:
            glBindVertexArray(m_cone_vao);
            drawSize = m_coneData.size();
            break;
        case PrimitiveType::PRIMITIVE_CUBE:
            glBindVertexArray(m_cube_vao);
            drawSize = m_cubeData.size();
            break;
        case PrimitiveType::PRIMITIVE_CYLINDER:
            glBindVertexArray(m_cyl_vao);
            drawSize = m_cylData.size();
            break;
        case PrimitiveType::PRIMITIVE_SPHERE:
            glBindVertexArray(m_sphere_vao);
            drawSize = m_sphereData.size();
            break;
        default:
            std::cerr << "unsupported shape" << std::endl;
            break;
        }

        sendAllMatrices(model, view, proj, m_phong_shader);

        // pass in light data
        sendLightData(m_phong_shader, m_metaData.lights);

        // material factors
        sendMaterialData(m_phong_shader, m_metaData.shapes.at(i).primitive.material);

        // k factors
        sendGlobalData(m_phong_shader, m_metaData.globalData);

        // camera position
        GLint vecLocation_camPos = glGetUniformLocation(m_phong_shader, "camera_pos");
        if (vecLocation_camPos != -1) {
            glUniform4fv(vecLocation_camPos, 1, camPos);
        }

        // invert, shouldn't be necessary
        GLint invertLocation = glGetUniformLocation(m_phong_shader, "invert");
        if (invertLocation != -1) {
            glUniform1i(invertLocation, settings.perPixelFilter);
        }

        glDrawArrays(GL_TRIANGLES, 0, drawSize / 6);

        glBindVertexArray(0);

    }
    glUseProgram(0);
}

void Realtime::makeFBO(){
    // Task 19: Generate and bind an empty texture, set its min/mag filter interpolation, then unbind
    glGenTextures(1, &m_fbo_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fbo_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_fbo_width, m_fbo_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Task 20: Generate and bind a renderbuffer of the right size, set its format, then unbind
    glGenRenderbuffers(1, &m_fbo_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_fbo_renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_fbo_width, m_fbo_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Task 18: Generate and bind an FBO
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Task 21: Add our texture as a color attachment, and our renderbuffer as a depth+stencil attachment, to our FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fbo_texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_fbo_renderbuffer);

    // Task 22: Unbind the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
}

void Realtime::paintGL() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glViewport(0, 0, m_fbo_width, m_fbo_height);

    // Clear screen color and depth before painting
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintScene();

    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Task 26: Clear the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Task 27: Call paintTexture to draw our FBO color attachment texture | Task 31: Set bool parameter to true
    paintTexture(m_fbo_texture);
}

void Realtime::paintTexture(GLuint texture){
    glUseProgram(m_texture_shader);
    // pass in processing bools
    int pix = settings.perPixelFilter, filt = settings.kernelBasedFilter;
    sendInt(m_texture_shader, "pixel_processing", pix);
    sendInt(m_texture_shader, "filter_processing", filt);

    // width and height
    float wd = m_fbo_width, ht = m_fbo_height;
    sendFloat(m_texture_shader, "width", wd);
    sendFloat(m_texture_shader, "height", ht);

    glBindVertexArray(m_fullscreen_vao);
    glBindTexture(GL_TEXTURE_2D, texture);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void Realtime::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here
    m_fbo_width = size().width();
    m_fbo_height = size().height();
    // Task 34: Regenerate your FBOs
    glDeleteTextures(1, &m_fbo_texture);
    glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
    glDeleteFramebuffers(1, &m_fbo);
    makeFBO();

    // reset projection matrix using glm::perspective equivalent
    m_camera.setWidth(size().width());
    m_camera.setHeight(size().height());
    m_camera.setViewMatrix();
    m_camera.setInverseViewMatrix();
    m_camera.setPerspectiveMatrix(settings.nearPlane, settings.farPlane);
}

void Realtime::sceneChanged() {
    // Camera and scene parser are the same as ray
    m_metaData.lights.clear();

    SceneParser::parse(settings.sceneFilePath, m_metaData);

    m_camera = Camera(size().width(), size().height(), m_metaData.cameraData);

    settingsChanged();

    update(); // asks for a PaintGL() call to occur
}

void Realtime::settingsChanged() {
    makeCurrent();

    if (!glewExperimental) {
        return;
    }

    m_fbo_width = size().width();
    m_fbo_height = size().height();
    // Regenerate FBOs
    glDeleteTextures(1, &m_fbo_texture);
    glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
    glDeleteFramebuffers(1, &m_fbo);
    makeFBO();

    m_camera.setViewMatrix();
    m_camera.setInverseViewMatrix();
    m_camera.setPerspectiveMatrix(settings.nearPlane, settings.farPlane);

    Cone cone;
    Cube cube;
    Cylinder cyl;
    Sphere sphere;

    // CONE
    generateShape(cone, m_cone_vbo, m_cone_vao, m_coneData);

    // CUBE
    generateShape(cube, m_cube_vbo, m_cube_vao, m_cubeData);
    std::cout << "m_cubeData size: " << m_cubeData.size() << std::endl;

    // CYLINDER
    generateShape(cyl, m_cyl_vbo, m_cyl_vao, m_cylData);

    // SPHERE
    generateShape(sphere, m_sphere_vbo, m_sphere_vao, m_sphereData);

    update(); // asks for a PaintGL() call to occur
}

// ================== Project 6: Action!

void Realtime::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;
}

void Realtime::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void Realtime::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void Realtime::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}

void Realtime::mouseMoveEvent(QMouseEvent *event) {
    if (m_mouseDown) {
        int posX = event->position().x();
        int posY = event->position().y();
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        // Use deltaX and deltaY here to rotate
        glm::mat3 xShift = rotateAboutYAxis(-deltaX * 0.005);
        glm::vec3 crossed = glm::cross(glm::vec3(m_camera.getLook()), glm::vec3(m_camera.getUp()));
        glm::mat3 yShift = rotateAboutAxis(crossed, -deltaY * 0.005);

        glm::vec4 newLook = glm::vec4(yShift * xShift * glm::vec3(m_camera.getLook()), 1);
        glm::vec4 newUp = glm::vec4(yShift * xShift * glm::vec3(m_camera.getUp()), 1);

        m_camera.replaceLook(newLook);
        m_camera.replaceUp(newUp);

        m_camera.setViewMatrix();
        m_camera.setInverseViewMatrix();

        update(); // asks for a PaintGL() call to occur
    }
}

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    // Use deltaTime and m_keyMap here to move around
    if (m_keyMap[Qt::Key_W]) {
        m_camera.changePos(glm::normalize(m_camera.getLook()) * (5 * deltaTime));
        m_camera.setPerspectiveMatrix(settings.nearPlane, settings.farPlane);
    }
    if (m_keyMap[Qt::Key_S]) {
        m_camera.changePos(-glm::normalize(m_camera.getLook()) * (5 * deltaTime));
        m_camera.setPerspectiveMatrix(settings.nearPlane, settings.farPlane);
    }
    if (m_keyMap[Qt::Key_A]) {
        glm::vec3 crossed = glm::cross(glm::vec3(m_camera.getUp()), glm::vec3(m_camera.getLook()));
        m_camera.changePos(glm::vec4(glm::normalize(crossed), 0) * (5 * deltaTime));
        m_camera.setPerspectiveMatrix(settings.nearPlane, settings.farPlane);
    }
    if (m_keyMap[Qt::Key_D]) {
        glm::vec3 crossed = glm::cross(glm::vec3(m_camera.getLook()), glm::vec3(m_camera.getUp()));
        m_camera.changePos(glm::vec4(glm::normalize(crossed), 0) * (5 * deltaTime));
        m_camera.setPerspectiveMatrix(settings.nearPlane, settings.farPlane);
    }
    if (m_keyMap[Qt::Key_Space]) {
        m_camera.changePos(glm::vec4(0, 1, 0, 0) * (5 * deltaTime));
        m_camera.setPerspectiveMatrix(settings.nearPlane, settings.farPlane);
    }
    if (m_keyMap[Qt::Key_Control]) {
        m_camera.changePos(glm::vec4(0, -1, 0, 0) * (5 * deltaTime));
        m_camera.setPerspectiveMatrix(settings.nearPlane, settings.farPlane);
    }

    update(); // asks for a PaintGL() call to occur
}
