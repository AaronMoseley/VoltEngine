#pragma once

#include "Objects/ObjectComponent.h"
#include "Vulkan Interface/VulkanCommonFunctions.h"
#include "Vulkan Interface/GraphicsBuffer.h"

class UIMeshRenderer : public ObjectComponent {
public:
	UIMeshRenderer()
	{ };

	UIMeshRenderer(const std::vector<VulkanCommonFunctions::UIVertex>& vertices)
	{
		m_vertices = vertices;
	}

	UIMeshRenderer(const std::vector<VulkanCommonFunctions::UIVertex>& vertices, const std::vector<uint16_t>& indices)
	{
		m_vertices = vertices; m_indices = indices; m_useIndices = true;
	}

	virtual const std::vector<VulkanCommonFunctions::UIVertex>& GetVertices() { return m_vertices; }
	void SetVertices(const std::vector<VulkanCommonFunctions::UIVertex>& vertices);
	size_t GetVertexBufferSize() const { return m_vertexBufferSize; }

	virtual const std::vector<uint16_t>& GetIndices() { return m_indices; }
	void SetIndices(const std::vector<uint16_t>& indices);
	size_t GetIndexBufferSize() const { return m_indexBufferSize; }

	void SetDirtyData(bool dirty) { m_meshDataDirty = dirty; }
	bool IsMeshDataDirty() const { return m_meshDataDirty; }

	void SetVertexBuffer(const std::shared_ptr<GraphicsBuffer>& vertexBuffer) { m_vertexBuffer = vertexBuffer; }
	std::shared_ptr<GraphicsBuffer> GetVertexBuffer() { return m_vertexBuffer; }

	void SetIndexBuffer(const std::shared_ptr<GraphicsBuffer>& indexBuffer) { m_indexBuffer = indexBuffer; }
	std::shared_ptr<GraphicsBuffer> GetIndexBuffer() { return m_indexBuffer; }

protected:
	std::vector<VulkanCommonFunctions::UIVertex> m_vertices;
	std::vector<uint16_t> m_indices;

	std::shared_ptr<GraphicsBuffer> m_vertexBuffer = nullptr;
	std::shared_ptr<GraphicsBuffer> m_indexBuffer = nullptr;

	size_t m_indexBufferSize = 0;
	size_t m_vertexBufferSize = 0;

	bool m_useIndices = false;
	bool m_meshDataDirty = false;
};