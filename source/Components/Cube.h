#pragma once

#include "Components/MeshRenderer.h"
#include "Vulkan Interface/VulkanCommonFunctions.h"

class Cube : public MeshRenderer {
public:
    Cube() : MeshRenderer()
	{
        m_meshName = "Cube";
	}

    const std::vector<VulkanCommonFunctions::Vertex>& GetVertices() override { return kCubeVertices; };
	const std::vector<uint16_t>& GetIndices() override { return kCubeIndices; };

private:
    using MeshRenderer::SetIndices;
	using MeshRenderer::SetVertices;

    const std::vector<VulkanCommonFunctions::Vertex> kCubeVertices =
    {
        //top
        {{-0.5f, 0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}},
        {{-0.5f, 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 0.0f}},

        //bottom
        {{-0.5f, -0.5f, 0.5f, 0.0f}, {0.0f, -1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f, 0.0f}, {0.0f, -1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f, -0.5f, 0.0f}, {0.0f, -1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.5f, 0.0f}, {0.0f, -1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 0.0f}},

        //left
        {{-0.5f, 0.5f, 0.5f, 0.0f}, {-1.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}},
        {{-0.5f, -0.5f, 0.5f, 0.0f}, {-1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f, 0.0f}, {-1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}},
        {{-0.5f, 0.5f, -0.5f, 0.0f}, {-1.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 0.0f}},

        //right
        {{0.5f, 0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 0.0f}},

        //front
        {{-0.5f, 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 0.0f}},

        //back
        {{-0.5f, 0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}},
        {{-0.5f, -0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 0.0f}},
    };

    const std::vector<uint16_t> kCubeIndices =
    {
        //top
        0, 1, 3,  3, 1, 2,
        //bottom
        4, 7, 5,  7, 6, 5,

        //left
        8, 9, 11,  11, 9, 10,

        //right
        12, 15, 13,  15, 14, 13,

        //front
        16, 17, 19,  19, 17, 18,

        //back
        20, 23, 21,  23, 22, 21
    };
};