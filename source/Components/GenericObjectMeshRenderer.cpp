#include "GenericObjectMeshRenderer.h"
#include "Objects/RenderObject.h"

void GenericObjectMeshRenderer::SetColor(glm::vec3 color)
{
	m_color = color;
	GetOwner()->RequestInstanceBufferUpdate();
}

void GenericObjectMeshRenderer::SetLit(bool lit)
{
	m_lit = lit;
	GetOwner()->RequestInstanceBufferUpdate();
}

void GenericObjectMeshRenderer::SetOpacity(float opacity)
{
	m_opacity = opacity;
	GetOwner()->RequestInstanceBufferUpdate();
}

void GenericObjectMeshRenderer::SetShininess(float shininess)
{
	m_shininess = shininess;
	GetOwner()->RequestInstanceBufferUpdate();
}

void GenericObjectMeshRenderer::SetIsBillboarded(bool isBillboarded)
{
	m_isBillboarded = isBillboarded;
	GetOwner()->RequestInstanceBufferUpdate();
}

void GenericObjectMeshRenderer::OwnerSetCallback()
{
	GetOwner()->SetMaterialName(GenericObjectMaterial::GetNameStatic());
}