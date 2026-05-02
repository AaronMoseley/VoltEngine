#pragma once

#include "Objects/RenderObject.h"
#include "Vulkan Interface/VulkanCommonFunctions.h"
#include "Vulkan Interface/GraphicsBuffer.h"
#include "Vulkan Interface/GraphicsImage.h"
#include "Vulkan Interface/TextureImage.h"
#include "Vulkan Interface/GraphicsPipeline.h"
#include "Components/Text.h"
#include "Text Rendering/FontManager.h"

#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include <filesystem>

#include "Components/MeshRenderer.h"

class VulkanWindow;
class WindowManager;

class VulkanInterface {
public:
    VulkanInterface(WindowManager* windowManager);

    void DrawFrame(float deltaTime, const std::shared_ptr<Scene>& scene, const std::shared_ptr<FontManager>& fontManager);
	void SetSwapChainReady(const bool isReady) { m_swapChainReady=isReady; };

    bool HasRenderedFirstFrame() const { return m_renderedFirstFrame; };

    void Cleanup();

    std::shared_ptr<GraphicsBuffer> CreateVertexBuffer(const std::shared_ptr<MeshRenderer>& object) const;
    std::shared_ptr<GraphicsBuffer> CreateIndexBuffer(const std::shared_ptr<MeshRenderer>&  object) const;

    std::shared_ptr<GraphicsBuffer> CreateUIVertexBuffer(const std::shared_ptr<UIMeshRenderer>& imageObject) const;
    std::shared_ptr<GraphicsBuffer> CreateUIIndexBuffer(const std::shared_ptr<UIMeshRenderer>& imageObject) const;

    void CreateInstanceBuffer(const std::shared_ptr<MeshRenderer>& object);
	std::shared_ptr<GraphicsBuffer> CreateInstanceBuffer(size_t maxObjects) const;
    void UpdateObjectBuffers(const std::shared_ptr<MeshRenderer>& objectMesh);
    bool HasTexture(const std::filesystem::path& textureFilePath) { return std::find(m_textureFilePaths.begin(), m_textureFilePaths.end(), textureFilePath) != m_textureFilePaths.end(); };
    void UpdateTextureResources(const std::filesystem::path& newTextureFilePath, bool alreadyInitialized=true);
    void CreateDepthResources();

    void InitializeVulkan();

    void CleanupSwapChain() const;

private:
    void CreateVMAAllocator();

	void CreateDescriptorSetLayouts();
    void CreatePrimaryDescriptorSetLayout();
	void CreateUIDescriptorSetLayout();

    void CreateDescriptorPools();
	void CreatePrimaryDescriptorPool();
	void CreateUIDescriptorPool();

    void CreateAllDescriptorSets();
	void CreatePrimaryDescriptorSets();
	void CreateUIDescriptorSets();

    void CreateGraphicsPipelines();
    void CreatePrimaryGraphicsPipeline();
	void CreateUIGraphicsPipeline();

    void CreateTextureImage(const std::filesystem::path& textureFilePath);
    void CreateTextureImageView(const std::filesystem::path& textureFilePath);
    void CreateTextureSampler(const std::filesystem::path& textureFilePath);
    void CreateUniformBuffers();

    VkFormat FindDepthFormat() const;
    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
    void BeginDrawFrameCommandBuffer(VkCommandBuffer commandBuffer) const;
    void DrawInstancedObjectCommandBuffer(VkCommandBuffer commandBuffer, const std::string& objectName, size_t objectCount);
    void DrawSingleObjectCommandBuffer(VkCommandBuffer commandBuffer, const std::shared_ptr<RenderObject>& currentObject) const;
    void SwitchToUIPipeline(VkCommandBuffer commandBuffer) const;
	void DrawUIElementCommandBuffer(VkCommandBuffer commandBuffer, const std::shared_ptr<RenderObject>& currentObject, const std::shared_ptr<FontManager>& fontManager);
    static void EndDrawFrameCommandBuffer(VkCommandBuffer commandBuffer);
    static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    	VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    bool CheckValidationLayerSupport() const;
    void UpdateInstanceBuffer(const std::string& objectName,
    	const std::set<VulkanCommonFunctions::ObjectHandle>& objectHandles,
    	std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>>& objects);
    void UpdateUniformBuffer(uint32_t currentImage, std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>> objects) const;
    void DrawUIImageCommandBuffer(VkCommandBuffer commandBuffer, const std::shared_ptr<RenderObject>& currentObject) const;
    void DrawUITextCommandBuffer(VkCommandBuffer commandBuffer, const std::shared_ptr<RenderObject>& currentObject, const std::shared_ptr<FontManager>& fontManager);

    static constexpr int MAX_FRAMES_IN_FLIGHT = 3;

    VkInstance m_vulkanInstance{};
    VkDebugUtilsMessengerEXT m_debugMessenger{};
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_vkDevice{};
    VkQueue m_graphicsQueue{};
    VkSurfaceKHR m_vkSurface{};
    VkQueue m_presentQueue{};
    VkCommandPool m_commandPool{};

    std::shared_ptr<GraphicsPipeline> m_mainGraphicsPipeline = nullptr;
	std::shared_ptr<GraphicsPipeline> m_uiGraphicsPipeline = nullptr;

    std::map<std::string, std::shared_ptr<GraphicsBuffer>> m_vertexBuffers;
    std::map<std::string, std::shared_ptr<GraphicsBuffer>> m_indexBuffers;

    std::map<std::string, uint16_t> m_vertexBufferSizes;
    std::map<std::string, uint16_t> m_indexBufferSizes;

	std::vector<std::shared_ptr<GraphicsBuffer>> m_uniformBuffers;
	std::vector<std::shared_ptr<GraphicsBuffer>> m_lightInfoBuffers;
	std::vector<std::shared_ptr<GraphicsBuffer>> m_uiUniformBuffers;

    std::vector<std::filesystem::path> m_textureFilePaths;
    std::map<std::filesystem::path, size_t> m_texturePathToIndex;
    std::map<std::filesystem::path, std::shared_ptr<TextureImage>> m_textureImages;

	const std::filesystem::path kDefaultTexturePath = "textures/DefaultTexture.png";

    const size_t kMaxLightCount = 200;

    uint32_t m_currentFrameIndex = 0;

    VkDescriptorSetLayout m_primaryDescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_uiDescriptorSetLayout = VK_NULL_HANDLE;

    VkDescriptorPool m_primaryDescriptorPool = VK_NULL_HANDLE;
	VkDescriptorPool m_uiDescriptorPool = VK_NULL_HANDLE;

    std::vector<VkDescriptorSet> m_primaryDescriptorSets;
	std::vector<VkDescriptorSet> m_uiDescriptorSets;

    std::shared_ptr<GraphicsImage> m_depthImage;

    bool m_framebufferResized = false;

    std::array<std::map<std::string, std::shared_ptr<GraphicsBuffer>>, MAX_FRAMES_IN_FLIGHT> m_instanceBuffers;

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
};