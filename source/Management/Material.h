#ifndef VOLTENGINE_MATERIAL_H
#define VOLTENGINE_MATERIAL_H

#include "MaterialRegistry.h"
#include "Vulkan Interface/GraphicsPipeline.h"
#include "Vulkan Interface/VulkanCommonFunctions.h"
#include "Objects/RenderObject.h"
#include "Management/VertexFormat.h"

class Material {
public:
	Material(const std::string& materialName) : m_materialName(materialName)
	{};

	void InitializeMaterial(const MaterialRegistry::MaterialCreationData& creationData);

	const std::string& GetMaterialName() { return m_materialName; };

	virtual size_t GetInstanceInfoSize() { return 0; }
	virtual void GetInstanceInfo(RenderObject* object, const std::vector<std::filesystem::path>& textureFilePaths, std::vector<std::byte>& outData) = 0;
	virtual size_t GetInstanceCount(RenderObject* object) { return 1; }

	std::shared_ptr<GraphicsPipeline> GetGraphicsPipeline() { return m_graphicsPipeline; }
	VkDescriptorSet& GetDescriptorSet(size_t frameIndex) { return m_descriptorSets[frameIndex]; }

	void CleanupMaterial();

protected:
	void CreateGraphicsPipeline();

	virtual void CreateVertexFormat() = 0;
	virtual void CreateDescriptorSetLayout(const MaterialRegistry::MaterialCreationData& creationData);
	virtual void CreateDescriptorSets(const MaterialRegistry::MaterialCreationData& creationData);
	virtual void CreateDescriptorPool(const MaterialRegistry::MaterialCreationData& creationData);

	virtual std::filesystem::path GetVertexShaderPath() { return ""; }
	virtual std::filesystem::path GetPixelShaderPath() { return ""; }

	std::array<VkDescriptorSet, VulkanCommonFunctions::MAX_FRAMES_IN_FLIGHT> m_descriptorSets;

	std::string m_materialName = "DefaultMaterial";

	std::shared_ptr<GraphicsPipeline> m_graphicsPipeline = nullptr;

	std::shared_ptr<VertexFormat> m_vertexFormat = nullptr;

	VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
	VmaAllocator m_allocator = VK_NULL_HANDLE;
	VkCommandPool m_commandPool = VK_NULL_HANDLE;
	VkQueue m_graphicsQueue = VK_NULL_HANDLE;
	VkDevice m_vkDevice = VK_NULL_HANDLE;
	VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
	VulkanWindow* m_vulkanWindow = nullptr;
};



#endif //VOLTENGINE_MATERIAL_H
