#ifndef VOLTENGINE_MESHRENDERER_H
#define VOLTENGINE_MESHRENDERER_H

#include "Objects/ObjectComponent.h"
#include "Vulkan Interface/GraphicsBuffer.h"

class IMeshRenderer : public ObjectComponent
{
public:
	inline const static std::string kCustomMeshName = "CustomMesh";

	IMeshRenderer()
	{

	}

	virtual bool IsIndexed() const = 0;

	virtual size_t GetVertexCount() const = 0;
	virtual size_t GetIndexCount() const = 0;

	virtual size_t GetVertexDataSize() const = 0;

	virtual void SetDirtyData(bool dirty) = 0;
	virtual bool IsMeshDataDirty() const = 0;

	virtual void SetVertexBuffer(const std::shared_ptr<GraphicsBuffer>& vertexBuffer) = 0;
	virtual std::shared_ptr<GraphicsBuffer> GetVertexBuffer() = 0;

	virtual void SetIndexBuffer(const std::shared_ptr<GraphicsBuffer>& indexBuffer) = 0;
	virtual std::shared_ptr<GraphicsBuffer> GetIndexBuffer() = 0;

	virtual std::string GetMeshName() = 0;

	virtual void GetVertexData(std::vector<std::byte>& outVertexData) = 0;
	virtual void GetIndices(std::vector<uint32_t>& outIndices) = 0;
	virtual void SetIndexed(bool newIndexedValue) = 0;

	virtual bool GetTextured() const = 0;
	virtual std::string GetTexturePath() = 0;

	virtual void SetTexture(const std::string& texturePath) = 0;
	virtual void SetTextured(bool textured) = 0;

	virtual void SetTextureDataDirty(bool dirty) = 0;
	virtual bool IsTextureDataDirty() const = 0;

private:
};

template<typename T>
class MeshRenderer : public IMeshRenderer {
public:
	MeshRenderer()
	{
		m_vertexDataSize = sizeof(T);
	}

	bool IsIndexed() const override { return m_indexed; }

	size_t GetVertexCount() const override { return m_vertexCount; }
	size_t GetIndexCount() const override { return m_indexCount; }

	size_t GetVertexDataSize() const override { return m_vertexDataSize; }

	void SetDirtyData(bool dirty) override { m_meshDataDirty = dirty; }
	bool IsMeshDataDirty() const override { return m_meshDataDirty; }

	void SetVertexBuffer(const std::shared_ptr<GraphicsBuffer>& vertexBuffer) override { m_vertexBuffer = vertexBuffer; }
	std::shared_ptr<GraphicsBuffer> GetVertexBuffer() override { return m_vertexBuffer; }

	void SetIndexBuffer(const std::shared_ptr<GraphicsBuffer>& indexBuffer) override { m_indexBuffer = indexBuffer; }
	std::shared_ptr<GraphicsBuffer> GetIndexBuffer() override { return m_indexBuffer; }

	std::string GetMeshName() override { return m_meshName; }

	void GetVertexData(std::vector<std::byte>& outVertexData) override
	{
		outVertexData.resize(m_vertexCount * GetVertexDataSize());
		memcpy(outVertexData.data(), m_vertices.data(), outVertexData.size());
	}

	void GetVertices(std::vector<T>& outVertices) { outVertices = m_vertices; }
	void SetVertices(const std::vector<T>& vertices)
	{
		m_vertexCount = vertices.size();
		m_vertices = vertices;

		SetDirtyData(true);
	}

	void GetIndices(std::vector<uint32_t>& outIndices) override
	{
		outIndices = m_indices;
	}

	void SetIndices(const std::vector<uint32_t>& indices)
	{
		m_indexCount = indices.size();
		m_indices = indices;
		m_indexed = true;

		SetDirtyData(true);
	}

	void SetIndexed(bool newIndexedValue) override { m_indexed = newIndexedValue; }

	bool GetTextured() const override { return m_textured; }
	std::string GetTexturePath() override { return m_texturePath; }

	void SetTexture(const std::string& texturePath) override
	{
		m_texturePath = texturePath;
		m_textured = true;
		m_textureDataDirty = true;
		TextureSetCallback();
	};
	void SetTextured(bool textured) override { m_textured = textured; }

	void SetTextureDataDirty(bool dirty) override { m_textureDataDirty = dirty; }
	bool IsTextureDataDirty() const override { return m_textureDataDirty; }

	virtual void TextureSetCallback() {};

protected:
	std::vector<T> m_vertices;
	std::vector<uint32_t> m_indices;

	bool m_meshDataDirty = false;
	bool m_indexed = false;

	bool m_textured = false;
	bool m_textureDataDirty = false;
	std::string m_texturePath = "";

	std::shared_ptr<GraphicsBuffer> m_vertexBuffer = nullptr;
	std::shared_ptr<GraphicsBuffer> m_indexBuffer = nullptr;

	size_t m_vertexDataSize = 0;

	size_t m_indexCount = 0;
	size_t m_vertexCount = 0;

	std::string m_meshName = kCustomMeshName;
};

#endif //VOLTENGINE_MESHRENDERER_H