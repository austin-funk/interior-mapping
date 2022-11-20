#include "realtimescenedata.h"


realtimescenedata::realtimescenedata(int width, int height, RenderData& metaData):
    m_width(width), m_height(height), m_metaData(metaData), m_camera(Camera(width, height, metaData.cameraData)) {}

const int& realtimescenedata::w() const {
    // Optional TODO: implement the getter or make your own design
    return m_width;
}

const int& realtimescenedata::h() const {
    // Optional TODO: implement the getter or make your own design
    return m_height;
}

const RenderData* realtimescenedata::getRData() const {
    return &m_metaData;
}

const SceneGlobalData* realtimescenedata::getGlobalData() const {
    // Optional TODO: implement the getter or make your own design
    return &(m_metaData.globalData);
}

const Camera* realtimescenedata::getCamera() const {
    // Optional TODO: implement the getter or make your own design
    return &m_camera;
}
