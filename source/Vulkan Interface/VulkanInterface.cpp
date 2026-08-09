#include "VulkanInterface.h"
#include "Vulkan Interface/VulkanWindow.h"
#include "Management/WindowManager.h"
#include "Management/Material.h"

#include "stb_image.h"
#include "Components/Camera.h"
#include "Components/LightSource.h"

std::shared_ptr<VulkanInterface> VulkanInterface::s_vulkanInterfaceSingleton = nullptr;

VulkanInterface::VulkanInterface(WindowManager* windowManager)
{
    m_windowManager = windowManager;
}

void VulkanInterface::InitializeVulkan()
{
    m_vulkanWindow = m_windowManager->GetVulkanWindow();

    m_vulkanInstance = m_vulkanWindow->vulkanInstance()->vkInstance();
    m_vkSurface = QVulkanInstance::surfaceForWindow(m_vulkanWindow);
    m_physicalDevice = m_vulkanWindow->physicalDevice();
    m_vkDevice = m_vulkanWindow->device();
    m_commandPool = m_vulkanWindow->graphicsCommandPool();
    m_graphicsQueue = m_vulkanWindow->graphicsQueue();
    CreateVMAAllocator();
	UpdateTextureResources(kDefaultTexturePath, false);
    CreateUniformBuffers();
	InitializeMaterials();
}

void VulkanInterface::CreateTextureSampler(const std::filesystem::path& textureFilePath)
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);

	m_textureImages[textureFilePath]->CreateTextureSampler(properties.limits.maxSamplerAnisotropy);
}

void VulkanInterface::CreateTextureImageView(const std::filesystem::path& textureFilePath) {
	m_textureImages[textureFilePath]->CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
}

void VulkanInterface::CreateDepthResources() {
    VkFormat depthFormat = FindDepthFormat();

	GraphicsImage::GraphicsImageCreateInfo depthImageCreateInfo{};
	depthImageCreateInfo.imageSize = { m_vulkanWindow->swapChainImageSize().width(), m_vulkanWindow->swapChainImageSize().height()};
	depthImageCreateInfo.format = depthFormat;
	depthImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	depthImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	depthImageCreateInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	depthImageCreateInfo.allocator = m_vmaAllocator;
	depthImageCreateInfo.device = m_vkDevice;
	depthImageCreateInfo.commandPool = m_commandPool;
	depthImageCreateInfo.graphicsQueue = m_graphicsQueue;
	m_depthImage = std::make_shared<GraphicsImage>(depthImageCreateInfo);

	m_depthImage->CreateImageView(VK_IMAGE_ASPECT_DEPTH_BIT);

	m_depthImage->TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

VkFormat VulkanInterface::FindDepthFormat() const
{
    return FindSupportedFormat(
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

VkFormat VulkanInterface::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const
{
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

void VulkanInterface::CreateTextureImage(const std::filesystem::path& textureFilePath) {
    int texWidth, texHeight, texChannels;

    stbi_uc* pixels = stbi_load(textureFilePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image: " + textureFilePath.string());
    }

    GraphicsBuffer::BufferCreateInfo stagingBufferCreateInfo{};
	stagingBufferCreateInfo.allocator = m_vmaAllocator;
	stagingBufferCreateInfo.size = imageSize;
	stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBufferCreateInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	stagingBufferCreateInfo.device = m_vkDevice;
	stagingBufferCreateInfo.commandPool = m_commandPool;
	stagingBufferCreateInfo.graphicsQueue = m_graphicsQueue;
	std::unique_ptr<GraphicsBuffer> stagingBuffer = std::make_unique<GraphicsBuffer>(stagingBufferCreateInfo);

	stagingBuffer->LoadData(pixels, static_cast<size_t>(imageSize));

    stbi_image_free(pixels);

	GraphicsImage::GraphicsImageCreateInfo textureImageCreateInfo{};
	textureImageCreateInfo.imageSize = { static_cast<size_t>(texWidth), static_cast<size_t>(texHeight) };
	textureImageCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	textureImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	textureImageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	textureImageCreateInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	textureImageCreateInfo.allocator = m_vmaAllocator;
	textureImageCreateInfo.device = m_vkDevice;
	textureImageCreateInfo.commandPool = m_commandPool;
	textureImageCreateInfo.graphicsQueue = m_graphicsQueue;

	std::shared_ptr<TextureImage> currentImage = std::make_shared<TextureImage>(textureImageCreateInfo);
	m_textureImages[textureFilePath] = currentImage;

	currentImage->TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	currentImage->CopyFromBuffer(stagingBuffer.get());
	currentImage->TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	stagingBuffer->DestroyBuffer();
}

void VulkanInterface::InitializeMaterials()
{
	MaterialRegistry::MaterialCreationData creationData;
	GetMaterialCreationInfo(creationData);
	MaterialRegistry::Get()->InitializeAllMaterials(creationData);
}

void VulkanInterface::UpdateTextureResources(const std::filesystem::path& textureFilePath, bool alreadyInitialized)
{
	m_textureFilePaths.push_back(textureFilePath);
	m_texturePathToIndex[textureFilePath] = m_textureFilePaths.size() - 1;
	CreateTextureImage(textureFilePath);

    CreateTextureSampler(textureFilePath);
	CreateTextureImageView(textureFilePath);

    if (alreadyInitialized)
    {
    	InitializeMaterials();
    }
}

void VulkanInterface::CreateUniformBuffers() {
    VkDeviceSize uniformBufferSize = sizeof(VulkanCommonFunctions::GlobalInfo);
    m_uniformBuffers.resize(VulkanCommonFunctions::MAX_FRAMES_IN_FLIGHT);

    VkDeviceSize lightBufferSize = sizeof(VulkanCommonFunctions::LightInfo) * kMaxLightCount;
    m_lightInfoBuffers.resize(VulkanCommonFunctions::MAX_FRAMES_IN_FLIGHT);

	VkDeviceSize uiUniformBufferSize = sizeof(VulkanCommonFunctions::UIGlobalInfo);
	m_uiUniformBuffers.resize(VulkanCommonFunctions::MAX_FRAMES_IN_FLIGHT);

	GraphicsBuffer::BufferCreateInfo uniformBufferCreateInfo{};
	uniformBufferCreateInfo.allocator = m_vmaAllocator;
	uniformBufferCreateInfo.size = uniformBufferSize;
	uniformBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	uniformBufferCreateInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	uniformBufferCreateInfo.device = m_vkDevice;
	uniformBufferCreateInfo.commandPool = m_commandPool;
	uniformBufferCreateInfo.graphicsQueue = m_graphicsQueue;

	GraphicsBuffer::BufferCreateInfo lightBufferCreateInfo{};
	lightBufferCreateInfo.allocator = m_vmaAllocator;
	lightBufferCreateInfo.size = lightBufferSize;
	lightBufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	lightBufferCreateInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	lightBufferCreateInfo.device = m_vkDevice;
	lightBufferCreateInfo.commandPool = m_commandPool;
	lightBufferCreateInfo.graphicsQueue = m_graphicsQueue;

    GraphicsBuffer::BufferCreateInfo uiUniformBufferCreateInfo{};
    uiUniformBufferCreateInfo.allocator = m_vmaAllocator;
    uiUniformBufferCreateInfo.size = uiUniformBufferSize;
    uiUniformBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    uiUniformBufferCreateInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    uiUniformBufferCreateInfo.device = m_vkDevice;
    uiUniformBufferCreateInfo.commandPool = m_commandPool;
    uiUniformBufferCreateInfo.graphicsQueue = m_graphicsQueue;

    for (size_t i = 0; i < VulkanCommonFunctions::MAX_FRAMES_IN_FLIGHT; i++) {
        std::shared_ptr<GraphicsBuffer> uniformBuffer = std::make_shared<GraphicsBuffer>(uniformBufferCreateInfo);
        std::shared_ptr<GraphicsBuffer> lightBuffer = std::make_shared<GraphicsBuffer>(lightBufferCreateInfo);
        std::shared_ptr<GraphicsBuffer> uiUniformBuffer = std::make_shared<GraphicsBuffer>(uiUniformBufferCreateInfo);

		m_uniformBuffers[i] = uniformBuffer;
		m_lightInfoBuffers[i] = lightBuffer;
		m_uiUniformBuffers[i] = uiUniformBuffer;
    }
}

void VulkanInterface::BeginDrawFrameCommandBuffer(VkCommandBuffer commandBuffer) const
{
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_vulkanWindow->defaultRenderPass();
    renderPassInfo.framebuffer = m_vulkanWindow->currentFramebuffer();
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = { static_cast<uint32_t>(m_vulkanWindow->swapChainImageSize().width()), static_cast<uint32_t>(m_vulkanWindow->swapChainImageSize().height())};

    std::array<VkClearValue, 2> clearValues{};

    clearValues[0].color = { {0.1f, 0.1f, 0.1f, 1.0f} };
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_vulkanWindow->swapChainImageSize().width());
    viewport.height = static_cast<float>(m_vulkanWindow->swapChainImageSize().height());
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = { static_cast<uint32_t>(m_vulkanWindow->swapChainImageSize().width()), static_cast<uint32_t>(m_vulkanWindow->swapChainImageSize().height()) };
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

std::shared_ptr<GraphicsBuffer> VulkanInterface::CreateInstanceBuffer(size_t maxObjects, size_t instanceInfoSize) const
{
	VkDeviceSize bufferSize = instanceInfoSize * maxObjects;

	GraphicsBuffer::BufferCreateInfo instanceBufferCreateInfo = {};
	instanceBufferCreateInfo.size = bufferSize;
	instanceBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	instanceBufferCreateInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	instanceBufferCreateInfo.allocator = m_vmaAllocator;
	instanceBufferCreateInfo.commandPool = m_commandPool;
	instanceBufferCreateInfo.graphicsQueue = m_graphicsQueue;
	instanceBufferCreateInfo.device = m_vkDevice;

	std::shared_ptr<GraphicsBuffer> instanceBuffer = std::make_shared<GraphicsBuffer>(instanceBufferCreateInfo);
	return instanceBuffer;
}

void VulkanInterface::CreateInstanceBuffersFromObject(std::shared_ptr<IMeshRenderer> objectMesh)
{
	if (objectMesh->GetMeshName() == IMeshRenderer::kCustomMeshName)
	{
		return;
	}

	std::shared_ptr<MaterialRegistry> materialRegistry = MaterialRegistry::Get();
	std::string materialName = objectMesh->GetOwner()->GetMaterialName();
	std::shared_ptr<Material> material = materialRegistry->GetMaterialByName(materialName);
	size_t instanceInfoSize = material->GetInstanceInfoSize();

	for (uint32_t frameIndex = 0; frameIndex < VulkanCommonFunctions::MAX_FRAMES_IN_FLIGHT; frameIndex++)
	{
		std::shared_ptr<GraphicsBuffer> instanceBuffer = CreateInstanceBuffer(VulkanCommonFunctions::MAX_OBJECTS, instanceInfoSize);
		m_instanceBuffers[frameIndex][objectMesh->GetMeshName()] = instanceBuffer;
	}
}

void VulkanInterface::DrawInstancedObjectCommandBuffer(VkCommandBuffer commandBuffer, const std::string& objectName, size_t objectCount) {
    if (objectCount == 0)
    {
	    return;
    }

    VkBuffer objectVertexBuffer[] = { m_vertexBuffers[objectName]->GetVkBuffer(), m_instanceBuffers[m_currentFrameIndex][objectName]->GetVkBuffer()};
    VkDeviceSize offsets[] = { 0, 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 2, objectVertexBuffer, offsets);

    if (m_indexBufferSizes[objectName] > 0)
    {
        vkCmdBindIndexBuffer(commandBuffer, m_indexBuffers[objectName]->GetVkBuffer(), 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(commandBuffer, m_indexBufferSizes[objectName], objectCount, 0, 0, 0);
    }
    else {
        vkCmdDraw(commandBuffer, m_vertexBufferSizes[objectName], objectCount, 0, 0);
    }
}

void VulkanInterface::DrawSingleObjectCommandBuffer(VkCommandBuffer commandBuffer, const std::shared_ptr<RenderObject>& renderObject, bool regenerateInstanceData) const
{
	std::vector<std::shared_ptr<GraphicsBuffer>> vertexBuffers;
	std::vector<size_t> vertexBufferSizes;
	renderObject->GetVertexBuffer(vertexBufferSizes, vertexBuffers);

	std::vector<std::shared_ptr<GraphicsBuffer>> indexBuffers;
	std::vector<size_t> indexBufferSizes;
	renderObject->GetIndexBuffer(indexBufferSizes, indexBuffers);

	if (renderObject->RequiresInstanceBufferRegeneration())
	{
		std::shared_ptr<Material> material = MaterialRegistry::Get()->GetMaterialByName(renderObject->GetMaterialName());

		size_t instanceInfoSize = material->GetInstanceInfoSize();
		std::shared_ptr<GraphicsBuffer> instanceBuffer = CreateInstanceBuffer(renderObject->GetInstanceCount(), instanceInfoSize);
		renderObject->SetInstanceBuffer(instanceBuffer);
	}

	std::shared_ptr<GraphicsBuffer> instanceBuffer = renderObject->GetInstanceBuffer(m_textureFilePaths);

	size_t instanceCount = renderObject->GetInstanceCount();

	for (size_t i = 0; i < vertexBuffers.size(); i++)
	{
		VkBuffer objectVertexBuffer[] = { vertexBuffers[i]->GetVkBuffer(), instanceBuffer->GetVkBuffer()};
		VkDeviceSize offsets[] = { 0, 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 2, objectVertexBuffer, offsets);

		if (indexBuffers[i] != nullptr && indexBufferSizes[i] > 0)
		{
			vkCmdBindIndexBuffer(commandBuffer, indexBuffers[i]->GetVkBuffer(), 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(commandBuffer, indexBufferSizes[i], instanceCount, 0, 0, 0);
		}
		else {
			vkCmdDraw(commandBuffer, vertexBufferSizes[i], instanceCount, 0, 0);
		}
	}
}

void VulkanInterface::EndDrawFrameCommandBuffer(VkCommandBuffer commandBuffer)
{
    vkCmdEndRenderPass(commandBuffer);
}

void VulkanInterface::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanInterface::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

bool VulkanInterface::CheckValidationLayerSupport() const
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : m_validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

std::shared_ptr<GraphicsBuffer> VulkanInterface::CreateVertexBuffer(const std::shared_ptr<IMeshRenderer>& meshInfo) const
{
    VkDeviceSize bufferSize = meshInfo->GetVertexDataSize() * meshInfo->GetVertexCount();

    GraphicsBuffer::BufferCreateInfo stagingBufferCreateInfo = {};
	stagingBufferCreateInfo.size = bufferSize;
	stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingBufferCreateInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	stagingBufferCreateInfo.allocator = m_vmaAllocator;
	stagingBufferCreateInfo.commandPool = m_commandPool;
	stagingBufferCreateInfo.graphicsQueue = m_graphicsQueue;
    stagingBufferCreateInfo.device = m_vkDevice;

	std::vector<std::byte> vertexData;
	meshInfo->GetVertexData(vertexData);

	std::shared_ptr<GraphicsBuffer> stagingBuffer = std::make_shared<GraphicsBuffer>(stagingBufferCreateInfo);
    stagingBuffer->LoadData(vertexData.data(), vertexData.size());

    GraphicsBuffer::BufferCreateInfo vertexBufferCreateInfo = {};
    vertexBufferCreateInfo.size = bufferSize;
    vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vertexBufferCreateInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    vertexBufferCreateInfo.allocator = m_vmaAllocator;
    vertexBufferCreateInfo.commandPool = m_commandPool;
    vertexBufferCreateInfo.graphicsQueue = m_graphicsQueue;
    vertexBufferCreateInfo.device = m_vkDevice;

    std::shared_ptr<GraphicsBuffer> vertexBuffer = std::make_shared<GraphicsBuffer>(vertexBufferCreateInfo);

    stagingBuffer->CopyBuffer(vertexBuffer, bufferSize);
    stagingBuffer->DestroyBuffer();

    return vertexBuffer;
}

void VulkanInterface::UpdateObjectBuffers(const std::shared_ptr<IMeshRenderer>& objectMesh)
{
	const std::string meshName = objectMesh->GetMeshName();

    if (meshName == IMeshRenderer::kCustomMeshName)
    {
        return;
    }

    if (m_vertexBuffers.contains(meshName) || (objectMesh->IsIndexed() && m_indexBuffers.contains(meshName)))
    {
        return;
    }

    size_t vertexCount = objectMesh->GetVertexCount();
    size_t indexCount = objectMesh->GetIndexCount();

    if (vertexCount == 0)
    {
        return;
    }

    if (objectMesh->IsIndexed() && indexCount > 0)
    {
        std::shared_ptr<GraphicsBuffer> indexBuffer = CreateIndexBuffer(objectMesh);
        m_indexBuffers[meshName] = indexBuffer;
        m_indexBufferSizes[meshName] = indexCount;
    }

    std::shared_ptr<GraphicsBuffer> vertexBuffer = CreateVertexBuffer(objectMesh);
    m_vertexBuffers[meshName] = vertexBuffer;
    m_vertexBufferSizes[meshName] = vertexCount;

    CreateInstanceBuffersFromObject(objectMesh);
}

std::shared_ptr<GraphicsBuffer> VulkanInterface::CreateIndexBuffer(const std::shared_ptr<IMeshRenderer>& meshInfo) const
{
    VkDeviceSize bufferSize = sizeof(uint32_t) * meshInfo->GetIndexCount();

    GraphicsBuffer::BufferCreateInfo stagingBufferCreateInfo = {};
    stagingBufferCreateInfo.size = bufferSize;
    stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingBufferCreateInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    stagingBufferCreateInfo.allocator = m_vmaAllocator;
    stagingBufferCreateInfo.commandPool = m_commandPool;
    stagingBufferCreateInfo.graphicsQueue = m_graphicsQueue;
    stagingBufferCreateInfo.device = m_vkDevice;

	std::vector<uint32_t> indices;
	meshInfo->GetIndices(indices);

    std::shared_ptr<GraphicsBuffer> stagingBuffer = std::make_shared<GraphicsBuffer>(stagingBufferCreateInfo);
    stagingBuffer->LoadData(indices.data(), bufferSize);

    GraphicsBuffer::BufferCreateInfo indexBufferCreateInfo = {};
    indexBufferCreateInfo.size = bufferSize;
    indexBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    indexBufferCreateInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    indexBufferCreateInfo.allocator = m_vmaAllocator;
    indexBufferCreateInfo.commandPool = m_commandPool;
    indexBufferCreateInfo.graphicsQueue = m_graphicsQueue;
    indexBufferCreateInfo.device = m_vkDevice;

    std::shared_ptr<GraphicsBuffer> indexBuffer = std::make_shared<GraphicsBuffer>(indexBufferCreateInfo);

	stagingBuffer->CopyBuffer(indexBuffer, bufferSize);

	stagingBuffer->DestroyBuffer();

	return indexBuffer;
}

void VulkanInterface::GetMaterialCreationInfo(MaterialRegistry::MaterialCreationData& outCreationData)
{
	outCreationData.m_allocator = m_vmaAllocator;
	outCreationData.m_commandPool = m_commandPool;
	outCreationData.m_graphicsQueue = m_graphicsQueue;
	outCreationData.m_vkDevice = m_vkDevice;
	outCreationData.m_vulkanWindow = m_vulkanWindow;
	outCreationData.m_maxLightCount = kMaxLightCount;
	outCreationData.m_lightInfoBuffers = m_lightInfoBuffers;
	outCreationData.m_textureFilePaths = m_textureFilePaths;
	outCreationData.m_textureImages = m_textureImages;
	outCreationData.m_uniformBuffers = m_uniformBuffers;
	outCreationData.m_uiUniformBuffers = m_uiUniformBuffers;
}

void VulkanInterface::CreateVMAAllocator()
{
    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    allocatorCreateInfo.physicalDevice = m_physicalDevice;
    allocatorCreateInfo.device = m_vkDevice;
    allocatorCreateInfo.instance = m_vulkanInstance;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

    vmaCreateAllocator(&allocatorCreateInfo, &m_vmaAllocator);
}

void VulkanInterface::UpdateInstanceBuffer(const std::string& objectName,
	const std::set<VulkanCommonFunctions::ObjectHandle>& objectHandles,
	const std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>>& objects,
	const std::map<VulkanCommonFunctions::ObjectHandle, size_t>& objectsToUpdate)
{
    std::vector<std::byte> objectInfo;

	bool shouldUpdate = false;
	for (auto it = objectHandles.begin(); it != objectHandles.end(); it++)
	{
		VulkanCommonFunctions::ObjectHandle currentHandle = *it;

		if (currentHandle == VulkanCommonFunctions::INVALID_OBJECT_HANDLE || !objects.contains(currentHandle))
		{
			continue;
		}

		shouldUpdate |= objectsToUpdate.contains(currentHandle);

		if (shouldUpdate)
		{
			break;
		}
	}

	if (shouldUpdate == false)
	{
		return;
	}

	size_t instanceDataSize = 0;
    for (auto it = objectHandles.begin(); it != objectHandles.end(); it++)
    {
        VulkanCommonFunctions::ObjectHandle currentHandle = *it;

        if (currentHandle == VulkanCommonFunctions::INVALID_OBJECT_HANDLE || !objects.contains(currentHandle))
        {
            continue;
        }

        const std::shared_ptr<RenderObject> object = objects.at(currentHandle);

    	if (instanceDataSize == 0)
    	{
    		instanceDataSize = object->GetInstanceDataSize();
    	}

		std::shared_ptr<IMeshRenderer> meshRenderer = object->GetComponent<IMeshRenderer>();

        if (meshRenderer != nullptr && meshRenderer->IsEnabled())
        {
        	std::shared_ptr<Material> material = MaterialRegistry::Get()->GetMaterialByName(object->GetMaterialName());

        	std::vector<std::byte> currentObjectInfo(material->GetInstanceInfoSize());
        	material->GetInstanceInfo(object.get(), m_textureFilePaths, currentObjectInfo);

        	objectInfo.insert(objectInfo.end(), currentObjectInfo.begin(), currentObjectInfo.end());
        }
    }

    if (objectInfo.empty())
    {
        return; 
    }

    VkDeviceSize bufferSize = objectHandles.size() * instanceDataSize;

	std::shared_ptr<GraphicsBuffer> instanceBuffer = m_instanceBuffers[m_currentFrameIndex][objectName];

	instanceBuffer->LoadData(objectInfo.data(), (size_t)bufferSize);
}

void VulkanInterface::SwitchToPipelineFromMaterial(VkCommandBuffer commandBuffer, std::shared_ptr<Material> material) const
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->GetGraphicsPipeline()->GetVkPipeline());

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->GetGraphicsPipeline()->GetVkPipelineLayout(), 0, 1, &material->GetDescriptorSet(m_currentFrameIndex), 0, nullptr);
}

void VulkanInterface::DrawFrame(float deltaTime, const std::shared_ptr<Scene>& scene) {
    vkDeviceWaitIdle(m_vkDevice);

    if (m_swapChainReady == false)
    {
        m_vulkanWindow->requestUpdate();
        return;
    }

    const std::map<std::string, std::map<std::string, std::set<VulkanCommonFunctions::ObjectHandle>>>& objectHandles = scene->GetMaterialAndNameToObjectMap();
    const std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>>& objects = scene->GetObjects();
    const std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>>& uiObjects = scene->GetUIObjects();
	const std::map<VulkanCommonFunctions::ObjectHandle, size_t>& objectsToUpdate = scene->GetObjectsToUpdate();

	for (auto it = objectHandles.begin(); it != objectHandles.end(); it++)
	{
		for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
		{
			std::string objectName = it2->first;

			if (objectName != IMeshRenderer::kCustomMeshName)
			{
				UpdateInstanceBuffer(objectName, it2->second, objects, objectsToUpdate);
			}
		}
	}

    UpdateUniformBuffer(m_currentFrameIndex, objects);

    VkCommandBuffer commandBuffer = m_vulkanWindow->currentCommandBuffer();

    BeginDrawFrameCommandBuffer(commandBuffer);

    for (auto it = objectHandles.begin(); it != objectHandles.end(); it++)
    {
    	std::string materialName = it->first;

    	SwitchToPipelineFromMaterial(commandBuffer, MaterialRegistry::Get()->GetMaterialByName(materialName));

        for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
        {
            std::string objectName = it2->first;

            if (objectName == IMeshRenderer::kCustomMeshName)
            {
            	for (auto it3 = it2->second.begin(); it3 != it2->second.end(); it3++)
            	{
            		VulkanCommonFunctions::ObjectHandle currentHandle = *it3;
            		DrawSingleObjectCommandBuffer(commandBuffer, objects.at(currentHandle), objectsToUpdate.contains(currentHandle));
            	}
            } else
            {
            	DrawInstancedObjectCommandBuffer(commandBuffer, objectName, it2->second.size());
            }
        }
    }

	std::string activeMaterialName = "";

	for (auto it = uiObjects.begin(); it != uiObjects.end(); it++)
	{
		std::string currentMaterialName = it->second->GetMaterialName();

		if (currentMaterialName != activeMaterialName)
		{
			SwitchToPipelineFromMaterial(commandBuffer, MaterialRegistry::Get()->GetMaterialByName(currentMaterialName));
			activeMaterialName = currentMaterialName;
		}

		DrawSingleObjectCommandBuffer(commandBuffer, it->second, objectsToUpdate.contains(it->first));
	}

    EndDrawFrameCommandBuffer(commandBuffer);

    m_currentFrameIndex = (m_currentFrameIndex + 1) % VulkanCommonFunctions::MAX_FRAMES_IN_FLIGHT;
    m_renderedFirstFrame = true;

	scene->DecrementFrameCountersForObjects();

    m_vulkanWindow->frameReady();
    m_vulkanWindow->requestUpdate();
}

void VulkanInterface::UpdateUniformBuffer(uint32_t currentImage, std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>> objects) const
{
    VulkanCommonFunctions::GlobalInfo globalInfo{};
    float aspectRatio = static_cast<float>(m_vulkanWindow->swapChainImageSize().width()) / static_cast<float>(m_vulkanWindow->swapChainImageSize().height());

	bool cameraFound = false;
    for (auto it = objects.begin(); it != objects.end(); it++)
    {
		std::shared_ptr<Camera> camera = it->second->GetComponent<Camera>();
        if (camera == nullptr)
        {
            continue;
        }

        if (!camera->IsMainCamera())
        {
            continue;
        }

		globalInfo.m_viewMatrix = camera->GetViewMatrix();
		globalInfo.m_projectionMatrix = glm::perspective(glm::radians(camera->GetFOV()), aspectRatio, camera->GetNearPlane(), camera->GetFarPlane());
        globalInfo.m_projectionMatrix[1][1] *= -1;
		globalInfo.m_cameraPosition = glm::vec4(it->second->GetComponent<Transform>()->GetPosition(), 1.0f);
		cameraFound = true;
        break;
    }

    if (!cameraFound)
    {
		throw std::runtime_error("No camera found in the scene. Please add a camera to render the scene.");
    }

    std::vector<VulkanCommonFunctions::LightInfo> lightInfos;

    for (auto it = objects.begin(); it != objects.end(); it++)
    {
		std::shared_ptr<LightSource> light = it->second->GetComponent<LightSource>();

        if (light == nullptr)
        {
            continue;
        }

        if (lightInfos.size() >= kMaxLightCount)
        {
			std::cout << "Warning: Maximum light count exceeded, additional lights will be ignored in rendering." << std::endl;
            break;
        }

        if (!light->IsEnabled())
        {
            continue;
        }

        VulkanCommonFunctions::LightInfo lightInfo = light->GetLightInfo();
		lightInfos.push_back(lightInfo);
    }

    globalInfo.m_lightCount.x = lightInfos.size();

	VulkanCommonFunctions::UIGlobalInfo uiGlobalInfo{};
	uiGlobalInfo.m_screenSize.x = m_vulkanWindow->swapChainImageSize().width();
	uiGlobalInfo.m_screenSize.y = m_vulkanWindow->swapChainImageSize().height();

	m_lightInfoBuffers[currentImage]->LoadData(lightInfos.data(), lightInfos.size() * sizeof(VulkanCommonFunctions::LightInfo));
	m_uniformBuffers[currentImage]->LoadData(&globalInfo, sizeof(globalInfo));
	m_uiUniformBuffers[currentImage]->LoadData(&uiGlobalInfo, sizeof(uiGlobalInfo));
}

void VulkanInterface::CleanupSwapChain() const
{
    vkDeviceWaitIdle(m_vkDevice);
	m_depthImage->DestroyImage();
}

void VulkanInterface::Cleanup() {
    vkDeviceWaitIdle(m_vkDevice);

    for (size_t i = 0; i < VulkanCommonFunctions::MAX_FRAMES_IN_FLIGHT; i++) {
		m_uniformBuffers[i]->DestroyBuffer();
		m_lightInfoBuffers[i]->DestroyBuffer();
        m_uiUniformBuffers[i]->DestroyBuffer();
    }

	MaterialRegistry::Get()->CleanupMaterials();

    for (auto it = m_textureFilePaths.begin(); it != m_textureFilePaths.end(); it++)
    {
		m_textureImages[*it]->DestroyTextureImage();
    }

    for (auto it = m_indexBuffers.begin(); it != m_indexBuffers.end(); it++)
    {
    	it->second->DestroyBuffer();
    }

    for (auto it = m_vertexBuffers.begin(); it != m_vertexBuffers.end(); it++)
    {
    	it->second->DestroyBuffer();
    }

    for (uint32_t frameIndex = 0; frameIndex < VulkanCommonFunctions::MAX_FRAMES_IN_FLIGHT; frameIndex++)
    {
        for (auto it = m_instanceBuffers[frameIndex].begin(); it != m_instanceBuffers[frameIndex].end(); it++)
        {
            it->second->DestroyBuffer();
        }
    }

    vmaDestroyAllocator(m_vmaAllocator);
}

void VulkanInterface::CreateVulkanInterface(WindowManager* windowManager)
{
	if (s_vulkanInterfaceSingleton != nullptr)
	{
		return;
	}

	s_vulkanInterfaceSingleton = std::make_shared<VulkanInterface>(windowManager);
}

std::shared_ptr<VulkanInterface> VulkanInterface::Get()
{
	return s_vulkanInterfaceSingleton;
}