#ifndef VOLTENGINE_MATERIAL_H
#define VOLTENGINE_MATERIAL_H

#include "Scene.h"
#include "MaterialRegistry.h"
#include "Vulkan Interface/VulkanCommonFunctions.h"

class Material {
public:
	Material(const std::string& materialName) : m_materialName(materialName)
	{
		MaterialRegistry::Get()->RegisterMaterial(this);
	};

	void InitializeMaterial();

	virtual void UpdateMaterial(Scene* currentScene) = 0;
	virtual void UpdateTextureResources(const std::filesystem::path& newTexturePath) = 0;

	const std::string& GetMaterialName() { return m_materialName; };

protected:
	virtual void CreateGraphicsPipeline() = 0;
	virtual void CreateConstantBuffer() = 0;

	virtual void CreateDescriptorSetLayout() = 0;
	virtual void CreateDescriptorSets() = 0;

	virtual void CreateUniformBuffer() = 0;
	virtual void CreateInstanceBuffer() = 0;

	virtual std::filesystem::path GetVertexShaderPath() { return ""; }
	virtual std::filesystem::path GetPixelShaderPath() { return ""; }

	std::array<VkDescriptorSet, VulkanCommonFunctions::MAX_FRAMES_IN_FLIGHT> m_descriptorSets;
	std::vector<std::array<std::shared_ptr<GraphicsBuffer>, VulkanCommonFunctions::MAX_FRAMES_IN_FLIGHT>> m_uniformBuffers;
	std::array<std::map<std::string, std::shared_ptr<GraphicsBuffer>>, VulkanCommonFunctions::MAX_FRAMES_IN_FLIGHT> m_instanceBuffers;

	std::string m_materialName = "DefaultMaterial";

	std::shared_ptr<GraphicsPipeline> m_graphicsPipeline = nullptr;

	VulkanWindow* m_vulkanWindow = nullptr;
	VkDevice m_vkDevice = VK_NULL_HANDLE;
	VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
	VmaAllocator m_allocator = VK_NULL_HANDLE;
	VkCommandPool m_commandPool = VK_NULL_HANDLE;
	VkQueue m_graphicsQueue = VK_NULL_HANDLE;
};



#endif //VOLTENGINE_MATERIAL_H
