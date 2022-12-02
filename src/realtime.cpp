#include "realtime.h"
#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "glm/ext/matrix_clip_space.hpp"
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

    // Students: anything requiring OpenGL calls when the program exits should be done here
    // delete vbo/vao stuff
    glDeleteBuffers(1, &m_cone_vbo);
    glDeleteVertexArrays(1, &m_cone_vao);
    glDeleteBuffers(1, &m_cube_vbo);
    glDeleteVertexArrays(1, &m_cube_vao);
    glDeleteBuffers(1, &m_cyl_vbo);
    glDeleteVertexArrays(1, &m_cyl_vao);
    glDeleteBuffers(1, &m_sphere_vbo);
    glDeleteVertexArrays(1, &m_sphere_vao);

    // delete fbo stuff
    // glDeleteTextures(1, &m_kitten_texture);
    glDeleteTextures(1, &m_fbo_texture);
    glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
    glDeleteFramebuffers(1, &m_fbo);

    this->doneCurrent();
}

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

    // Students: anything requiring OpenGL calls when the program starts should be done here
    m_defaultFBO = 2;
    m_fbo_width = size().width();
    m_fbo_height = size().height();

    glClearColor(0, 0, 0, 1);

    m_phong_shader = ShaderLoader::createShaderProgram(":/resources/shaders/default.vert", ":/resources/shaders/default.frag");
    m_texture_shader = ShaderLoader::createShaderProgram(":/resources/shaders/texture.vert", ":/resources/shaders/texture.frag");

    glUseProgram(m_texture_shader);
    GLint texture0Loc = glGetUniformLocation(m_texture_shader, "kitten");
    if (texture0Loc != -1) {
        glUniform1i(texture0Loc, 0);
    }


    // Task 11: Fix this "fullscreen" quad's vertex data
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
    // tell openGL which shader to use
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

        // model_mat uniform
        GLint matrixLocationModel = glGetUniformLocation(m_phong_shader, "model_mat");
        if (matrixLocationModel != -1) {
            glUniformMatrix4fv(matrixLocationModel, 1, GL_FALSE, &model[0][0]);
        }
        GLint matrixLocModelInv = glGetUniformLocation(m_phong_shader, "inv_tran_model_mat");
        if (matrixLocModelInv != -1) {
            glm::mat4 inv = glm::transpose(glm::inverse(model));//glm::mat3()
            glUniformMatrix4fv(matrixLocModelInv, 1, GL_FALSE, &inv[0][0]);
        }

        // view_mat uniform
        GLint matrixLocationView = glGetUniformLocation(m_phong_shader, "view_mat");
        if (matrixLocationView != -1) {
            glUniformMatrix4fv(matrixLocationView, 1, GL_FALSE, &view[0][0]);
        }

        // projection_mat uniform
        GLint matrixLocationProj = glGetUniformLocation(m_phong_shader, "projection_mat");
        if (matrixLocationProj != -1) {
            glUniformMatrix4fv(matrixLocationProj, 1, GL_FALSE, &proj[0][0]);
        }

        // pass in light data
        GLint intLocationNumLights = glGetUniformLocation(m_phong_shader, "numLights");
        GLint intLocationType, vecLocationPos, vecLocationDir, vecLocationColor, vecLocationCoeffs, vecLocationAngle, vecLocationPenumbra;
        int numLight = 0;
        bool inc = true;
        for (int i = 0; i < m_metaData.lights.size(); ++i) {
            if (numLight > 7) {
                break;
            }
            // light type
            intLocationType = glGetUniformLocation(m_phong_shader, ("lights[" + std::to_string(numLight) + "].type").c_str());
            if (intLocationNumLights != -1) {
                switch (m_metaData.lights.at(i).type) {
                case LightType::LIGHT_DIRECTIONAL:
                    glUniform1i(intLocationType, 0);
                    break;
                case LightType::LIGHT_POINT:
                    glUniform1i(intLocationType, 1);
                    break;
                case LightType::LIGHT_SPOT:
                    glUniform1i(intLocationType, 2);
                    break;
                default:
                    glUniform1i(intLocationType, 3); // check if 3, means unsupported light
                    inc = false;
                    break;
                }
            }
            if (!inc) {
                continue;
            }
            // light pos
            vecLocationPos = glGetUniformLocation(m_phong_shader, ("lights[" + std::to_string(numLight) + "].pos").c_str());
            if (vecLocationPos != -1) {
                glUniform4fv(vecLocationPos, 1, &m_metaData.lights.at(i).pos[0]);
            }
            // light dir
            vecLocationDir = glGetUniformLocation(m_phong_shader, ("lights[" + std::to_string(numLight) + "].dir").c_str());
            if (vecLocationDir != -1) {
                glUniform4fv(vecLocationDir, 1, &m_metaData.lights.at(i).dir[0]);
            }
            // light color
            vecLocationColor = glGetUniformLocation(m_phong_shader, ("lights[" + std::to_string(numLight) + "].color").c_str());
            if (vecLocationColor != -1) {
                glUniform4fv(vecLocationColor, 1, &m_metaData.lights.at(i).color[0]);
            }
            // point coeffs
            vecLocationCoeffs = glGetUniformLocation(m_phong_shader, ("lights[" + std::to_string(numLight) + "].point_coeffs").c_str());
            if (vecLocationCoeffs != -1) {
                glUniform3fv(vecLocationCoeffs, 1, &m_metaData.lights.at(i).function[0]);
            }
            // light angle
            vecLocationAngle = glGetUniformLocation(m_phong_shader, ("lights[" + std::to_string(numLight) + "].angle").c_str());
            if (vecLocationAngle != -1) {
                glUniform1f(vecLocationAngle, m_metaData.lights.at(i).angle);
            }
            // light penumbra
            vecLocationPenumbra = glGetUniformLocation(m_phong_shader, ("lights[" + std::to_string(numLight) + "].penumbra").c_str());
            if (vecLocationPenumbra != -1) {
                glUniform1f(vecLocationPenumbra, m_metaData.lights.at(i).penumbra);
            }
            numLight++;
        }
        // set numLights
        if (intLocationNumLights != -1) {
            glUniform1i(intLocationNumLights, numLight);
        }

        // material factors
        GLint floatLocationMaterialAmbO = glGetUniformLocation(m_phong_shader, "material_ambO");
        if (floatLocationMaterialAmbO != -1) {
            glUniform4fv(floatLocationMaterialAmbO, 1, &m_metaData.shapes.at(i).primitive.material.cAmbient[0]);
        }
        GLint floatLocationMaterialDifO = glGetUniformLocation(m_phong_shader, "material_difO");
        if (floatLocationMaterialDifO != -1) {
            glUniform4fv(floatLocationMaterialDifO, 1, &m_metaData.shapes.at(i).primitive.material.cDiffuse[0]);
        }
        GLint floatLocationMaterialSpecO = glGetUniformLocation(m_phong_shader, "material_specO");
        if (floatLocationMaterialSpecO != -1) {
            glUniform4fv(floatLocationMaterialSpecO, 1, &m_metaData.shapes.at(i).primitive.material.cSpecular[0]);
        }

        // k_a
        GLint floatLocation_ka = glGetUniformLocation(m_phong_shader, "k_a");
        if (floatLocation_ka != -1) {
            glUniform1f(floatLocation_ka, m_metaData.globalData.ka);
        }

        // k_d
        GLint floatLocation_kd = glGetUniformLocation(m_phong_shader, "k_d");
        if (floatLocation_kd != -1) {
            glUniform1f(floatLocation_kd, m_metaData.globalData.kd);
        }

        // k_s
        GLint floatLocation_ks = glGetUniformLocation(m_phong_shader, "k_s");
        if (floatLocation_ks != -1) {
            glUniform1f(floatLocation_ks, m_metaData.globalData.ks);
        }

        // shiny factor
        GLint floatLocation_N = glGetUniformLocation(m_phong_shader, "shiny");
        if (floatLocation_N != -1) {
            glUniform1f(floatLocation_N, m_metaData.shapes.at(i).primitive.material.shininess);
        }

        // camera position
        GLint vecLocation_camPos = glGetUniformLocation(m_phong_shader, "camera_pos");
        if (vecLocation_camPos != -1) {
            glUniform4fv(vecLocation_camPos, 1, camPos);
        }

        // invert
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
    // Students: anything requiring OpenGL calls every frame should be done here
    // remove this later
//    if (settings.perPixelFilter) {
//        glClearColor(1, 1, 1, 1);
//    } else {
//        glClearColor(0, 0, 0, 1);
//    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glViewport(0, 0, m_fbo_width, m_fbo_height);

    // Clear screen color and depth before painting
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintScene();

    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    glViewport(0, 0, size().width(), size().height());

    // Task 26: Clear the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Task 27: Call paintTexture to draw our FBO color attachment texture | Task 31: Set bool parameter to true
    paintTexture(m_fbo_texture);
}

void Realtime::paintTexture(GLuint texture){
    glUseProgram(m_texture_shader);
    // pass in processing bools
    GLint ppLoc = glGetUniformLocation(m_texture_shader, "pixel_processing");
    if (ppLoc != -1) {
        glUniform1i(ppLoc, settings.perPixelFilter);
    }
    GLint kppLoc = glGetUniformLocation(m_texture_shader, "filter_processing");
    if (kppLoc != -1) {
        glUniform1i(kppLoc, settings.kernelBasedFilter);
    }
    // width and height
    GLint widLoc = glGetUniformLocation(m_texture_shader, "width");
    if (widLoc != -1) {
        glUniform1f(widLoc, (float)m_fbo_width);
    }
    GLint hLoc = glGetUniformLocation(m_texture_shader, "height");
    if (hLoc != -1) {
        glUniform1f(hLoc, (float)m_fbo_height);
    }

    glBindVertexArray(m_fullscreen_vao);
    // Task 10: Bind "texture" to slot 0
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
    m_camera = Camera(size().width(), size().height(), m_metaData.cameraData);
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
    // Task 34: Regenerate your FBOs
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
    // Generate and bind VBO
    glGenBuffers(1, &m_cone_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_cone_vbo);
    // Generate sphere data
    cone.updateParams(settings.shapeParameter1, settings.shapeParameter2);
    m_coneData = cone.generateShape();
    // Send data to VBO
    glBufferData(GL_ARRAY_BUFFER, m_coneData.size() * sizeof(GLfloat), m_coneData.data(), GL_STATIC_DRAW);
    // Generate, and bind vao
    glGenVertexArrays(1, &m_cone_vao);
    glBindVertexArray(m_cone_vao);

    // Enable and define attribute 0 to store vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void *>(0 * sizeof(GLfloat)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void *>(3 * sizeof(GLfloat)));

    // Clean-up bindings
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,0);

    // CUBE
    // Generate and bind VBO
    glGenBuffers(1, &m_cube_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_cube_vbo);
    // Generate sphere data
    cube.updateParams(settings.shapeParameter1);
    m_cubeData = cube.generateShape();
    // Send data to VBO
    glBufferData(GL_ARRAY_BUFFER, m_cubeData.size() * sizeof(GLfloat), m_cubeData.data(), GL_STATIC_DRAW);
    // Generate, and bind vao
    glGenVertexArrays(1, &m_cube_vao);
    glBindVertexArray(m_cube_vao);

    // Enable and define attribute 0 to store vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void *>(0 * sizeof(GLfloat)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void *>(3 * sizeof(GLfloat)));

    // Clean-up bindings
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,0);

    // CYLINDER
    // Generate and bind VBO
    glGenBuffers(1, &m_cyl_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_cyl_vbo);
    // Generate sphere data
    cyl.updateParams(settings.shapeParameter1, settings.shapeParameter2);
    m_cylData = cyl.generateShape();
    // Send data to VBO
    glBufferData(GL_ARRAY_BUFFER, m_cylData.size() * sizeof(GLfloat), m_cylData.data(), GL_STATIC_DRAW);
    // Generate, and bind vao
    glGenVertexArrays(1, &m_cyl_vao);
    glBindVertexArray(m_cyl_vao);

    // Enable and define attribute 0 to store vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void *>(0 * sizeof(GLfloat)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void *>(3 * sizeof(GLfloat)));

    // Clean-up bindings
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,0);

    // SPHERE
    // Generate and bind VBO
    glGenBuffers(1, &m_sphere_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_sphere_vbo);
    // Generate sphere data
    sphere.updateParams(settings.shapeParameter1, settings.shapeParameter2);
    m_sphereData = sphere.generateShape();
    // Send data to VBO
    glBufferData(GL_ARRAY_BUFFER, m_sphereData.size() * sizeof(GLfloat), m_sphereData.data(), GL_STATIC_DRAW);
    // Generate, and bind vao
    glGenVertexArrays(1, &m_sphere_vao);
    glBindVertexArray(m_sphere_vao);

    // Enable and define attribute 0 to store vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void *>(0 * sizeof(GLfloat)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void *>(3 * sizeof(GLfloat)));

    // Clean-up bindings
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,0);

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
