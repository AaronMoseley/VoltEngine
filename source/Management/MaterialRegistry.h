#ifndef VOLTENGINE_MATERIALREGISTRY_H
#define VOLTENGINE_MATERIALREGISTRY_H

#include <memory>
#include <map>
#include <vector>
#include <filesystem>

#include "Vulkan Interface/VulkanCommonFunctions.h"

class GraphicsBuffer;
class TextureImage;
class VulkanWindow;
class Material;

class MaterialRegistry {
public:
	struct MaterialCreationData
	{
		std::vector<std::filesystem::path> m_textureFilePaths;
		std::vector<std::shared_ptr<GraphicsBuffer>> m_uniformBuffers;
		std::vector<std::shared_ptr<GraphicsBuffer>> m_uiUniformBuffers;
		std::vector<std::shared_ptr<GraphicsBuffer>> m_lightInfoBuffers;
		std::map<std::filesystem::path, std::shared_ptr<TextureImage>> m_textureImages;
		size_t m_maxLightCount = 0;

		VulkanWindow* m_vulkanWindow = nullptr;
		VkDevice m_vkDevice = VK_NULL_HANDLE;
		VmaAllocator m_allocator = VK_NULL_HANDLE;
		VkCommandPool m_commandPool = VK_NULL_HANDLE;
		VkQueue m_graphicsQueue = VK_NULL_HANDLE;
	};

	MaterialRegistry();

	static std::shared_ptr<MaterialRegistry> Get();

	void RegisterMaterial(std::shared_ptr<Material> materialToRegister);
	void InitializeAllMaterials(const MaterialCreationData& creationData);

	const std::map<std::string, std::shared_ptr<Material>>& GetMaterials();
	std::shared_ptr<Material> GetMaterialByName(const std::string& materialName) const;

	void CleanupMaterials();

private:
	static std::shared_ptr<MaterialRegistry> s_materialRegistrySingleton;

	std::map<std::string, std::shared_ptr<Material>> m_registeredMaterials;
};

template<typename T>
struct MaterialAutoRegister {
	MaterialAutoRegister()
	{
		MaterialRegistry::Get()->RegisterMaterial(std::make_shared<T>());
	}
};


#endif //VOLTENGINE_MATERIALREGISTRY_H
