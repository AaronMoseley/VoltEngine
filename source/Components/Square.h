#ifndef POINTCLOUDAPP_SQUARE_H
#define POINTCLOUDAPP_SQUARE_H

#include "Components/GenericObjectMeshRenderer.h"
#include "Vulkan Interface/VulkanCommonFunctions.h"

class Square : public GenericObjectMeshRenderer {
public:
    Square() : GenericObjectMeshRenderer(kSquareVertices, kSquareIndices, "Square")
	{
        SetIsBillboarded(true);
	}

private:
    using MeshRenderer::SetIndices;
	using MeshRenderer::SetVertices;

	inline static const std::vector<GenericObjectMaterial::Vertex> kSquareVertices =
	{
		//positions                                     //normals                                       //texture coords
		{{-0.5f,  0.5f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}}, //top left
		{{ 0.5f,  0.5f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 0.0f}}, //top right
		{{ 0.5f, -0.5f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}}, //bottom right
		{{-0.5f, -0.5f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}}  //bottom left
	};

	inline static const std::vector<uint32_t> kSquareIndices =
	{
		0, 1, 2, 2, 3, 0
	};
};



#endif
