#ifndef VOLTENGINE_GLTFMODEL_H
#define VOLTENGINE_GLTFMODEL_H

#include <filesystem>
#include <tiny_gltf.h>
#include <glm.hpp>

#include "Components/GenericObjectMeshRenderer.h"
#include "Vulkan Interface/VulkanCommonFunctions.h"

class GLTFModel : public GenericObjectMeshRenderer {
public:
	GLTFModel();

	void Start() override;

	void SetSourcePath(const std::filesystem::path& sourceFilePath);

	template <typename T>
	bool GetAttribute(const std::string& attributeName, std::vector<T>& outAttributeData);

	void ReverseWindingOrder(bool changeFlag=true);

private:
	using MeshRenderer::SetIndices;
	using MeshRenderer::SetVertices;

	const std::string kPositionAttributeName = "POSITION";
	const std::string kNormalAttributeName = "NORMAL";
	const std::string kTextureCoordinateAttributeName = "TEXCOORD_0";
	const std::string kIndicesAttributeName = "INDICES";
	const std::string kColorAttributeName = "COLOR_0";

	glm::vec3 m_modelOrigin = glm::vec3(0.0f);

	void ReadModel();
	void ReadIndices();

	bool m_readFile = false;
	bool m_reverseWindingOrder = false;

	template <typename T>
	bool ReadAttribute(const std::string& attributeName, std::vector<uint8_t>& outBytes) const;

	template<typename T>
	bool AddAttributeFromNode(int nodeIndex, const std::string& attributeName, std::vector<uint8_t>& outBytes) const;

	tinygltf::Model m_model;

	std::filesystem::path m_sourcePath;

	std::map<std::string, std::vector<uint8_t>> m_customAttributes;
};



#endif //VOLTENGINE_GLTFMODEL_H
