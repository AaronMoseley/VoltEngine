#pragma once

#include "Components/MeshRenderer.h"
#include "Vulkan Interface/VulkanCommonFunctions.h"
#include "Vulkan Interface/GraphicsBuffer.h"
#include "Materials/GenericUIObjectMaterial.h"

class GenericUIObjectMeshRenderer : public MeshRenderer<GenericUIObjectMaterial::UIVertex> {
public:
	GenericUIObjectMeshRenderer()
	{

	}

	GenericUIObjectMeshRenderer(const std::vector<GenericUIObjectMaterial::UIVertex>& vertices)
	{
		SetVertices(vertices);
	}

	GenericUIObjectMeshRenderer(const std::vector<GenericUIObjectMaterial::UIVertex>& vertices, const std::vector<uint32_t>& indices)
	{
		SetVertices(vertices);
		SetIndices(indices);
	}

	void OwnerSetCallback() override
	{
		GetOwner()->SetMaterialName(GenericUIObjectMaterial::GetNameStatic());
	}

protected:
};