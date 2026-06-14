#ifndef VOLTENGINE_GLTFMODEL_H
#define VOLTENGINE_GLTFMODEL_H

#include <filesystem>
#include <tiny_gltf.h>
#include <glm.hpp>

#include "Components/MeshRenderer.h"
#include "Vulkan Interface/VulkanCommonFunctions.h"

class GLTFModel : public MeshRenderer {
public:
	GLTFModel();

	void SetSourcePath(const std::filesystem::path& sourceFilePath);

	template <typename T>
	void GetAttribute(const std::string& attributeName, std::vector<T>& outAttributeData);

	void ReverseWindingOrder();

	const std::vector<VulkanCommonFunctions::Vertex>& GetVertices() override { return m_vertices; };
	const std::vector<uint16_t>& GetIndices() override { return m_indices; };

private:
	using MeshRenderer::SetIndices;
	using MeshRenderer::SetVertices;

	const std::string kPositionAttributeName = "POSITION";
	const std::string kNormalAttributeName = "NORMAL";
	const std::string kTextureCoordinateAttributeName = "TEXCOORD_0";
	const std::string kIndicesAttributeName = "INDICES";

	glm::vec3 m_modelOrigin = glm::vec3(0.0f);

	void ReadModel();
	void ReadIndices();

	template <typename T>
	void ReadAttribute(const std::string& attributeName, std::vector<uint8_t>& outBytes) const;

	template<typename T>
	void AddAttributeFromNode(int nodeIndex, const std::string& attributeName, std::vector<uint8_t>& outBytes) const;

	tinygltf::Model m_model;

	std::filesystem::path m_sourcePath;

	std::vector<VulkanCommonFunctions::Vertex> m_vertices;
	std::vector<uint16_t> m_indices;
	std::map<std::string, std::vector<uint8_t>> m_customAttributes;
};



#endif //VOLTENGINE_GLTFMODEL_H
