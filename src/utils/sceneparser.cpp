#include "sceneparser.h"
#include "scenefilereader.h"
#include "glm/gtx/transform.hpp"

#include <chrono>
#include <memory>
#include <iostream>

void buildShapeData(SceneNode *node, glm::mat4 curCTM, RenderData *renderData) {
    // define local variables
    glm::mat4 tempCTM(1.0f);
    SceneTransformation* curTransformation;
    int numOfTransformations = node->transformations.size();
    // apply transformations to current CTM
    for (int x = numOfTransformations; x > 0; --x) {
        curTransformation = node->transformations.at(x - 1);
        switch (curTransformation->type) {
        case TransformationType::TRANSFORMATION_TRANSLATE:
            tempCTM = glm::translate(glm::mat4(1.0f), curTransformation->translate) * tempCTM;
            break;
        case TransformationType::TRANSFORMATION_SCALE:
            tempCTM = glm::scale(glm::mat4(1.0f), curTransformation->scale) * tempCTM;
            break;
        case TransformationType::TRANSFORMATION_ROTATE:
            tempCTM = glm::rotate(glm::mat4(1.0f), curTransformation->angle, curTransformation->rotate) * tempCTM;
            break;
        case TransformationType::TRANSFORMATION_MATRIX:
            tempCTM = curTransformation->matrix * tempCTM;
            break;
        }
    }
    glm::mat4 newCTM = curCTM * tempCTM;
    // if primitive, add to renderData with current CTM
    for (ScenePrimitive* prim : node->primitives) {
        renderData->shapes.push_back(RenderShapeData{*prim, newCTM});
    }
    // recur on all child nodes
    for (SceneNode* kidNode : node->children) {
        buildShapeData(kidNode, newCTM, renderData);
    }
}

bool SceneParser::parse(std::string filepath, RenderData &renderData) {
    ScenefileReader fileReader = ScenefileReader(filepath);
    bool success = fileReader.readXML();
    if (!success) {
        return false;
    }

    // TODO: Use your code from Lab 5 here.
    // Task 4: populate renderData with global data, lights, and camera data
    renderData = {fileReader.getGlobalData(), fileReader.getCameraData(), fileReader.getLights()};

    // Task 5: populate renderData's list of primitives and their transforms.
    //         This will involve traversing the scene graph, and we recommend you
    //         create a helper function to do so!
    renderData.shapes.clear();
    buildShapeData(fileReader.getRootNode(), glm::mat4(1.0f), &renderData);

    return true;
}
