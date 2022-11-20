#pragma once

#include "utils/scenedata.h"
#include "utils/sceneparser.h"
#include "camera/camera.h"

class realtimescenedata
{
private:
    int m_width, m_height;
    RenderData m_metaData;
    Camera m_camera;
public:
    realtimescenedata(int width, int height, RenderData &metaData);

    // The getter of the width of the scene
    const int& w() const;

    // The getter of the height of the scene
    const int& h() const;

    // The getter of metaData
    const RenderData* getRData() const;

    // The getter of the global data of the scene
    const SceneGlobalData* getGlobalData() const;

    // The getter of the shared pointer to the camera instance of the scene
    const Camera* getCamera() const;
};

