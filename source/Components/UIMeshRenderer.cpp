#include "UIMeshRenderer.h"

void UIMeshRenderer::SetVertices(const std::vector<VulkanCommonFunctions::UIVertex>& vertices)
{
	m_vertexBufferSize = vertices.size();
	m_vertices = vertices;

	SetDirtyData(true);
}

void UIMeshRenderer::SetIndices(const std::vector<uint16_t>& indices)
{
	m_indexBufferSize = indices.size();
	m_indices = indices;
	m_useIndices = true;

	SetDirtyData(true);
}