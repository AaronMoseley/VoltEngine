#pragma once

#include "Objects/ObjectComponent.h"
#include "Vulkan Interface/VulkanCommonFunctions.h"
#include "Vulkan Interface/GraphicsBuffer.h"

#include <glm.hpp>

#include <vector>

class MeshRenderer : public ObjectComponent {
public:
	inline const static std::string kCustomMeshName = "CustomMesh";

	MeshRenderer() { m_meshName = kCustomMeshName; };
	MeshRenderer(const std::vector<VulkanCommonFunctions::Vertex>& vertices, const std::string& name)
	{
		m_vertices = vertices; m_meshName = name;
	}

	MeshRenderer(const std::vector<VulkanCommonFunctions::Vertex>& vertices, const std::vector<uint16_t>& indices, const std::string& name)
	{
		m_vertices = vertices; m_indices = indices; m_useIndices = true; m_meshName = name;
	}

	virtual const std::vector<VulkanCommonFunctions::Vertex>& GetVertices() { return m_vertices; }
	void SetVertices(const std::vector<VulkanCommonFunctions::Vertex>& vertices);
	size_t GetVertexBufferSize() const { return m_vertexBufferSize; }

	virtual const std::vector<uint16_t>& GetIndices() { return m_indices; }
	void SetIndices(const std::vector<uint16_t>& indices);
	size_t GetIndexBufferSize() const { return m_indexBufferSize; }

	glm::vec3 GetColor() const { return m_color; }
	void SetColor(glm::vec3 color) { m_color = color; }

	bool IsIndexed() const { return m_useIndices; }
	void SetIndexed(bool useIndices) { m_useIndices = useIndices; }

	bool GetLit() const { return m_lit; }
	void SetLit(bool lit) { m_lit = lit; }

	bool GetTextured() const { return m_textured; }
	std::string GetTexturePath() { return m_texturePath; }

	void SetTexture(const std::string& texturePath)
	{
		m_texturePath = texturePath;
		m_textured = true;
		m_textureDataDirty = true;
	};
	void SetTextured(bool textured) { m_textured = textured; }

	void SetVertexBuffer(const std::shared_ptr<GraphicsBuffer>& vertexBuffer) { m_vertexBuffer = vertexBuffer; }
	std::shared_ptr<GraphicsBuffer> GetVertexBuffer() { return m_vertexBuffer; }

	void SetIndexBuffer(const std::shared_ptr<GraphicsBuffer>& indexBuffer) { m_indexBuffer = indexBuffer; }
	std::shared_ptr<GraphicsBuffer> GetIndexBuffer() { return m_indexBuffer; }

	std::string GetMeshName() { return m_meshName; }

	void SetOpacity(float opacity) { m_opacity = opacity; }
	float GetOpacity() const { return m_opacity; }

	void SetShininess(float shininess) { m_shininess = shininess; }
	float GetShininess() const { return m_shininess; }

	void SetDirtyData(bool dirty) { m_meshDataDirty = dirty; }
	bool IsMeshDataDirty() const { return m_meshDataDirty; }

	void SetTextureDataDirty(bool dirty) { m_textureDataDirty = dirty; }
	bool IsTextureDataDirty() const { return m_textureDataDirty; }

	void SetIsBillboarded(bool isBillboarded) { m_isBillboarded = isBillboarded; }
	bool IsBillboarded() const { return m_isBillboarded; }

protected:
	std::vector<VulkanCommonFunctions::Vertex> m_vertices;
	std::vector<uint16_t> m_indices;

	std::shared_ptr<GraphicsBuffer> m_vertexBuffer = nullptr;
	std::shared_ptr<GraphicsBuffer> m_indexBuffer = nullptr;

	size_t m_indexBufferSize = 0;
	size_t m_vertexBufferSize = 0;
	
	float m_opacity = 1.0f;
	float m_shininess = 4.0f;

	bool m_useIndices = false;
	bool m_lit = true;

	bool m_meshDataDirty = false;
	bool m_textureDataDirty = false;

	bool m_textured = false;
	std::string m_texturePath = "";

	bool m_isBillboarded = false;

	std::string m_meshName = "";
	alignas(16) glm::vec3 m_color = glm::vec3(1.0f);
};