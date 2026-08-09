#ifndef POINTCLOUDAPP_POINTMESHRENDERER_H
#define POINTCLOUDAPP_POINTMESHRENDERER_H

#include "Components/MeshRenderer.h"
#include "Materials/PointMaterial.h"

class PointMeshRenderer : public MeshRenderer<PointMaterial::PointVertex> {
public:
	PointMeshRenderer(const std::vector<PointMaterial::PointVertex>& vertices, const std::vector<uint32_t>& indices, const std::string& name)
	{
		SetVertices(vertices);
		SetIndices(indices);
		m_meshName = name;
	}

	glm::vec3 GetColor() const { return m_color; }
	void SetColor(glm::vec3 color);
	void OwnerSetCallback() override;

private:
	glm::vec3 m_color = glm::vec3(1.0f);
};



#endif
