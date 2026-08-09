#pragma once

#include "Objects/RenderObject.h"
#include "Vulkan Interface/VulkanCommonFunctions.h"
#include "Vulkan Interface/GraphicsBuffer.h"
#include "Vulkan Interface/GraphicsImage.h"
#include "Vulkan Interface/TextureImage.h"
#include "Vulkan Interface/GraphicsPipeline.h"
#include "Components/Text.h"
#include "Text Rendering/FontManager.h"
#include "Management/MaterialRegistry.h"

#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include <filesystem>

#include "Components/GenericObjectMeshRenderer.h"

class VulkanWindow;
class WindowManager;

class VulkanInterface {
public:
    VulkanInterface(WindowManager* windowManager);

    void DrawFrame(float deltaTime, const std::shared_ptr<Scene>& scene);
	void SetSwapChainReady(const bool isReady) { m_swapChainReady=isReady; };

    bool HasRenderedFirstFrame() const { return m_renderedFirstFrame; };

    void Cleanup();

    std::shared_ptr<GraphicsBuffer> CreateVertexBuffer(const std::shared_ptr<IMeshRenderer>& object) const;
    std::shared_ptr<GraphicsBuffer> CreateIndexBuffer(const std::shared_ptr<IMeshRenderer>&  object) const;

    void UpdateObjectBuffers(const std::shared_ptr<IMeshRenderer>& objectMesh);
    bool HasTexture(const std::filesystem::path& textureFilePath) { return std::find(m_textureFilePaths.begin(), m_textureFilePaths.end(), textureFilePath) != m_textureFilePaths.end(); };
    void UpdateTextureResources(const std::filesystem::path& newTextureFilePath, bool alreadyInitialized=true);
    void CreateDepthResources();

	bool HasVertexBuffer(const std::string& meshName) const { return m_vertexBuffers.contains(meshName); };
	bool HasIndexBuffer(const std::string& meshName) const { return m_indexBuffers.contains(meshName); };

    void InitializeVulkan();

	void CreateInstanceBuffersFromObject(std::shared_ptr<IMeshRenderer> objectMesh);
	std::shared_ptr<GraphicsBuffer> CreateInstanceBuffer(size_t maxObjects, size_t instanceInfoSize) const;

    void CleanupSwapChain() const;

	static void CreateVulkanInterface(WindowManager* windowManager);
	static std::shared_ptr<VulkanInterface> Get();

private:
	void GetMaterialCreationInfo(MaterialRegistry::MaterialCreationData& outCreationData);

    void CreateVMAAllocator();

    void CreateTextureImage(const std::filesystem::path& textureFilePath);
    void CreateTextureImageView(const std::filesystem::path& textureFilePath);
    void CreateTextureSampler(const std::filesystem::path& textureFilePath);
    void CreateUniformBuffers();

	void InitializeMaterials();

    VkFormat FindDepthFormat() const;
    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
    void BeginDrawFrameCommandBuffer(VkCommandBuffer commandBuffer) const;
    void DrawInstancedObjectCommandBuffer(VkCommandBuffer commandBuffer, const std::string& objectName, size_t objectCount);
    void DrawSingleObjectCommandBuffer(VkCommandBuffer commandBuffer, const std::shared_ptr<RenderObject>& currentObject, bool regenerateInstanceData) const;
    void SwitchToPipelineFromMaterial(VkCommandBuffer commandBuffer, std::shared_ptr<Material> material) const;
    static void EndDrawFrameCommandBuffer(VkCommandBuffer commandBuffer);
    static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    	VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    bool CheckValidationLayerSupport() const;
    void UpdateInstanceBuffer(const std::string& objectName,
    	const std::set<VulkanCommonFunctions::ObjectHandle>& objectHandles,
    	const std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>>& objects,
    	const std::map<VulkanCommonFunctions::ObjectHandle, size_t>& objectsToUpdate);
    void UpdateUniformBuffer(uint32_t currentImage, std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>> objects) const;

    VkInstance m_vulkanInstance{};
    VkDebugUtilsMessengerEXT m_debugMessenger{};
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_vkDevice{};
    VkQueue m_graphicsQueue{};
    VkSurfaceKHR m_vkSurface{};
    VkQueue m_presentQueue{};
    VkCommandPool m_commandPool{};

	std::array<std::map<std::string, std::shared_ptr<GraphicsBuffer>>, VulkanCommonFunctions::MAX_FRAMES_IN_FLIGHT> m_instanceBuffers;

    std::map<std::string, std::shared_ptr<GraphicsBuffer>> m_vertexBuffers;
    std::map<std::string, std::shared_ptr<GraphicsBuffer>> m_indexBuffers;

    std::map<std::string, size_t> m_vertexBufferSizes;
    std::map<std::string, size_t> m_indexBufferSizes;

	std::vector<std::shared_ptr<GraphicsBuffer>> m_uniformBuffers;
	std::vector<std::shared_ptr<GraphicsBuffer>> m_lightInfoBuffers;
	std::vector<std::shared_ptr<GraphicsBuffer>> m_uiUniformBuffers;

    std::vector<std::filesystem::path> m_textureFilePaths;
    std::map<std::filesystem::path, size_t> m_texturePathToIndex;
    std::map<std::filesystem::path, std::shared_ptr<TextureImage>> m_textureImages;

	const std::filesystem::path kDefaultTexturePath = "textures/DefaultTexture.png";

    const size_t kMaxLightCount = 200;

    uint32_t m_currentFrameIndex = 0;

    std::shared_ptr<GraphicsImage> m_depthImage;

    bool m_framebufferResized = false;

	VkDeviceSize m_uniformBufferAlignment = 1;

    VmaAllocator m_vmaAllocator{};

    WindowManager* m_windowManager;
    VulkanWindow* m_vulkanWindow{};

	const std::string m_customMeshName = "CustomMesh";

    const std::vector<const char*> m_validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> m_deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

#ifdef NDEBUG
    const bool m_enableValidationLayers = false;
#else
    const bool m_enableValidationLayers = true;
#endif

    bool m_renderedFirstFrame = false;
	bool m_swapChainReady = false;

	static std::shared_ptr<VulkanInterface> s_vulkanInterfaceSingleton;
};