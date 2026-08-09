#include "PointMeshRenderer.h"
#include "Objects/RenderObject.h"

void PointMeshRenderer::SetColor(glm::vec3 color)
{
	m_color = color;
	GetOwner()->RequestInstanceBufferUpdate();
}

void PointMeshRenderer::OwnerSetCallback()
{
	GetOwner()->SetMaterialName(PointMaterial::GetNameStatic());
}