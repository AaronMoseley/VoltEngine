#pragma once

#include "Components/MeshRenderer.h"
#include "Vulkan Interface/VulkanCommonFunctions.h"
#include "Vulkan Interface/GraphicsBuffer.h"
#include "Materials/GenericObjectMaterial.h"

#include <glm.hpp>

#include <vector>

class GenericObjectMeshRenderer : public MeshRenderer<GenericObjectMaterial::Vertex> {
public:
	GenericObjectMeshRenderer()
	{
		m_meshName = kCustomMeshName;
	};
	GenericObjectMeshRenderer(const std::vector<GenericObjectMaterial::Vertex>& vertices, const std::string& name)
	{
		SetVertices(vertices);
		m_meshName = name;
	}

	GenericObjectMeshRenderer(const std::vector<GenericObjectMaterial::Vertex>& vertices, const std::vector<uint32_t>& indices, const std::string& name)
	{
		SetVertices(vertices);
		SetIndices(indices);
		m_meshName = name;
	}

	glm::vec3 GetColor() const { return m_color; }
	void SetColor(glm::vec3 color) { m_color = color; }

	bool GetLit() const { return m_lit; }
	void SetLit(bool lit) { m_lit = lit; }

	void SetOpacity(float opacity) { m_opacity = opacity; }
	float GetOpacity() const { return m_opacity; }

	void SetShininess(float shininess) { m_shininess = shininess; }
	float GetShininess() const { return m_shininess; }

	void SetIsBillboarded(bool isBillboarded) { m_isBillboarded = isBillboarded; }
	bool IsBillboarded() const { return m_isBillboarded; }

	void OwnerSetCallback() override
	{
		GetOwner()->SetMaterialName(GenericObjectMaterial::GetNameStatic());
	}

protected:
	float m_opacity = 1.0f;
	float m_shininess = 4.0f;

	bool m_lit = true;

	bool m_meshDataDirty = false;
	bool m_textureDataDirty = false;

	bool m_textured = false;
	std::string m_texturePath = "";

	bool m_isBillboarded = false;

	glm::vec3 m_color = glm::vec3(1.0f);
};