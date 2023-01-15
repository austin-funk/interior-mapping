#pragma once

#include <glm/glm.hpp>
#include <vector>

class ThreeDShape
{
public:
    virtual void updateParams(int param1, int param2) = 0;
    virtual std::vector<float> generateShape() = 0;
};

