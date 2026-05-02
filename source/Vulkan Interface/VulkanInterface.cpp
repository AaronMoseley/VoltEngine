#include "VulkanInterface.h"
#include "Vulkan Interface/VulkanWindow.h"
#include "Management/WindowManager.h"

#include "stb_image.h"
#include "Components/Camera.h"
#include "Components/LightSource.h"

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
    CreateDescriptorSetLayouts();
    CreateGraphicsPipelines();
    CreateUniformBuffers();
    CreateDescriptorPools();
    CreateAllDescriptorSets();
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

void VulkanInterface::UpdateTextureResources(const std::filesystem::path& textureFilePath, bool alreadyInitialized)
{
	m_textureFilePaths.push_back(textureFilePath);
	m_texturePathToIndex[textureFilePath] = m_textureFilePaths.size() - 1;
	CreateTextureImage(textureFilePath);

    CreateTextureSampler(textureFilePath);
	CreateTextureImageView(textureFilePath);

    if (alreadyInitialized)
    {
        CreateDescriptorPools();
        CreateDescriptorSetLayouts();
        CreateAllDescriptorSets();
        CreateGraphicsPipelines();
    }
}

void VulkanInterface::CreateAllDescriptorSets() {
    CreatePrimaryDescriptorSets();
    CreateUIDescriptorSets();
}

void VulkanInterface::CreatePrimaryDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_primaryDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_primaryDescriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    m_primaryDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(m_vkDevice, &allocInfo, m_primaryDescriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_uniformBuffers[i]->GetVkBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(VulkanCommonFunctions::GlobalInfo);

        VkDescriptorBufferInfo lightBufferInfo{};
        lightBufferInfo.buffer = m_lightInfoBuffers[i]->GetVkBuffer();
        lightBufferInfo.offset = 0;
        lightBufferInfo.range = sizeof(VulkanCommonFunctions::LightInfo) * kMaxLightCount;

        std::vector<VkDescriptorImageInfo> imageInfos;

        for (auto it = m_textureFilePaths.begin(); it != m_textureFilePaths.end(); it++)
        {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = m_textureImages[*it]->GetImageView();
            imageInfo.sampler = m_textureImages[*it]->GetTextureSampler();

            imageInfos.push_back(imageInfo);
        }

        std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_primaryDescriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = m_primaryDescriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &lightBufferInfo;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = m_primaryDescriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[2].descriptorCount = imageInfos.size();
        descriptorWrites[2].pImageInfo = imageInfos.data();

        vkUpdateDescriptorSets(m_vkDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void VulkanInterface::CreateUIDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_uiDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_uiDescriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    m_uiDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(m_vkDevice, &allocInfo, m_uiDescriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        std::vector<VkDescriptorImageInfo> imageInfos;

        VkDescriptorBufferInfo globalBufferInfo{};
        globalBufferInfo.buffer = m_uiUniformBuffers[i]->GetVkBuffer();
        globalBufferInfo.offset = 0;
        globalBufferInfo.range = sizeof(VulkanCommonFunctions::UIGlobalInfo);

        for (auto it = m_textureFilePaths.begin(); it != m_textureFilePaths.end(); it++)
        {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = m_textureImages[*it]->GetImageView();
            imageInfo.sampler = m_textureImages[*it]->GetTextureSampler();

            imageInfos.push_back(imageInfo);
        }

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_uiDescriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &globalBufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = m_uiDescriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = imageInfos.size();
        descriptorWrites[1].pImageInfo = imageInfos.data();

        vkUpdateDescriptorSets(m_vkDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void VulkanInterface::CreateDescriptorPools() {
    CreatePrimaryDescriptorPool();
    CreateUIDescriptorPool();
}

void VulkanInterface::CreatePrimaryDescriptorPool() {
    if (m_primaryDescriptorPool != VK_NULL_HANDLE)
    {
		vkDestroyDescriptorPool(m_vkDevice, m_primaryDescriptorPool, nullptr);
    }
    
    std::array<VkDescriptorPoolSize, 3> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[2].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * m_textureFilePaths.size();

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(m_vkDevice, &poolInfo, nullptr, &m_primaryDescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void VulkanInterface::CreateUIDescriptorPool() {
    if (m_uiDescriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(m_vkDevice, m_uiDescriptorPool, nullptr);
    }

    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * m_textureFilePaths.size();

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(m_vkDevice, &poolInfo, nullptr, &m_uiDescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void VulkanInterface::CreateUniformBuffers() {
    VkDeviceSize uniformBufferSize = sizeof(VulkanCommonFunctions::GlobalInfo);
    m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkDeviceSize lightBufferSize = sizeof(VulkanCommonFunctions::LightInfo) * kMaxLightCount;
    m_lightInfoBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkDeviceSize uiUniformBufferSize = sizeof(VulkanCommonFunctions::UIGlobalInfo);
	m_uiUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

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

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        std::shared_ptr<GraphicsBuffer> uniformBuffer = std::make_shared<GraphicsBuffer>(uniformBufferCreateInfo);
        std::shared_ptr<GraphicsBuffer> lightBuffer = std::make_shared<GraphicsBuffer>(lightBufferCreateInfo);
        std::shared_ptr<GraphicsBuffer> uiUniformBuffer = std::make_shared<GraphicsBuffer>(uiUniformBufferCreateInfo);

		m_uniformBuffers[i] = uniformBuffer;
		m_lightInfoBuffers[i] = lightBuffer;
		m_uiUniformBuffers[i] = uiUniformBuffer;
    }
}

void VulkanInterface::CreateDescriptorSetLayouts() {
    CreatePrimaryDescriptorSetLayout();
    CreateUIDescriptorSetLayout();
}

void VulkanInterface::CreatePrimaryDescriptorSetLayout() {
    if (m_primaryDescriptorSetLayout != VK_NULL_HANDLE)
    {
		vkDestroyDescriptorSetLayout(m_vkDevice, m_primaryDescriptorSetLayout, nullptr);
    }
    
    VkDescriptorSetLayoutBinding globalInfoLayoutBinding{};
    globalInfoLayoutBinding.binding = 0;
    globalInfoLayoutBinding.descriptorCount = 1;
    globalInfoLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    globalInfoLayoutBinding.pImmutableSamplers = nullptr;
    globalInfoLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding lightInfoBinding{};
    lightInfoBinding.binding = 1;
    lightInfoBinding.descriptorCount = 1;
    lightInfoBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    lightInfoBinding.pImmutableSamplers = nullptr;
    lightInfoBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 2;
    samplerLayoutBinding.descriptorCount = m_textureFilePaths.size();
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 3> bindings = { globalInfoLayoutBinding, lightInfoBinding, samplerLayoutBinding };
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(m_vkDevice, &layoutInfo, nullptr, &m_primaryDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void VulkanInterface::CreateUIDescriptorSetLayout() 
{
    if (m_uiDescriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(m_vkDevice, m_uiDescriptorSetLayout, nullptr);
    }

    VkDescriptorSetLayoutBinding globalInfoLayoutBinding{};
    globalInfoLayoutBinding.binding = 0;
    globalInfoLayoutBinding.descriptorCount = 1;
    globalInfoLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    globalInfoLayoutBinding.pImmutableSamplers = nullptr;
    globalInfoLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = m_textureFilePaths.size();
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = { globalInfoLayoutBinding, samplerLayoutBinding };
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(m_vkDevice, &layoutInfo, nullptr, &m_uiDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void VulkanInterface::BeginDrawFrameCommandBuffer(VkCommandBuffer commandBuffer) const
{
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    //renderPassInfo.renderPass = renderPass;
    renderPassInfo.renderPass = m_vulkanWindow->defaultRenderPass();
    renderPassInfo.framebuffer = m_vulkanWindow->currentFramebuffer();
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = { static_cast<uint32_t>(m_vulkanWindow->swapChainImageSize().width()), static_cast<uint32_t>(m_vulkanWindow->swapChainImageSize().height())};

    std::array<VkClearValue, 2> clearValues{};

    //clearValues[0].color = { {0.345098039f, 0.52156862f, 0.6862745098039216f, 1.0f} };
    clearValues[0].color = { {0.1f, 0.1f, 0.1f, 1.0f} };
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_mainGraphicsPipeline->GetVkPipeline());

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

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_mainGraphicsPipeline->GetVkPipelineLayout(), 0, 1, &m_primaryDescriptorSets[m_currentFrameIndex], 0, nullptr);
}

void VulkanInterface::DrawInstancedObjectCommandBuffer(VkCommandBuffer commandBuffer, const std::string& objectName, size_t objectCount) {
    if (objectCount <= 0)
        return;
    
    VkBuffer objectVertexBuffer[] = { m_vertexBuffers[objectName]->GetVkBuffer(), m_instanceBuffers[m_currentFrameIndex][objectName]->GetVkBuffer()};
    VkDeviceSize offsets[] = { 0, 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 2, objectVertexBuffer, offsets);

    if (m_indexBufferSizes[objectName] > 0)
    {
        vkCmdBindIndexBuffer(commandBuffer, m_indexBuffers[objectName]->GetVkBuffer(), 0, VK_INDEX_TYPE_UINT16);

        vkCmdDrawIndexed(commandBuffer, m_indexBufferSizes[objectName], objectCount, 0, 0, 0);
    }
    else {
        vkCmdDraw(commandBuffer, m_vertexBufferSizes[objectName], objectCount, 0, 0);
    }
}

void VulkanInterface::DrawSingleObjectCommandBuffer(VkCommandBuffer commandBuffer, const std::shared_ptr<RenderObject>& renderObject) const
{
	std::shared_ptr<MeshRenderer> meshComponent = renderObject->GetComponent<MeshRenderer>();

    if (meshComponent == nullptr)
    {
        return;
    }

    if (meshComponent->GetMeshName() != MeshRenderer::kCustomMeshName)
    {
        return;
    }

    VkBuffer objectVertexBuffer[] = { meshComponent->GetVertexBuffer()->GetVkBuffer(), renderObject->GetInstanceBuffer(m_textureFilePaths)->GetVkBuffer()};
    VkDeviceSize offsets[] = { 0, 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 2, objectVertexBuffer, offsets);

    if (meshComponent->IsIndexed())
    {
        vkCmdBindIndexBuffer(commandBuffer, meshComponent->GetIndexBuffer()->GetVkBuffer(), 0, VK_INDEX_TYPE_UINT16);

        vkCmdDrawIndexed(commandBuffer, meshComponent->GetIndexBufferSize(), 1, 0, 0, 0);
        //vkCmdDrawIndexed(commandBuffer, indexBufferSizes[objectName], meshNameToObjectMap[objectName].size(), 0, 0, 0);
    }
    else {
        vkCmdDraw(commandBuffer, meshComponent->GetVertexBufferSize(), 1, 0, 0);
    }
}

void VulkanInterface::EndDrawFrameCommandBuffer(VkCommandBuffer commandBuffer)
{
    vkCmdEndRenderPass(commandBuffer);
}

void VulkanInterface::CreateGraphicsPipelines() 
{
    CreatePrimaryGraphicsPipeline();
    CreateUIGraphicsPipeline();
}

void VulkanInterface::CreatePrimaryGraphicsPipeline() 
{
    if (m_mainGraphicsPipeline != VK_NULL_HANDLE)
    {
        m_mainGraphicsPipeline->SetDescriptorSetLayout(m_primaryDescriptorSetLayout);
        m_mainGraphicsPipeline->CreatePipeline();
        return;
    }

	GraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.m_vertexShaderFilePath = "shaders/HLSL/VertexShader.spv";
	pipelineCreateInfo.m_fragmentShaderFilePath = "shaders/HLSL/PixelShader.spv";
	pipelineCreateInfo.m_descriptorSetLayout = m_primaryDescriptorSetLayout;
	pipelineCreateInfo.m_device = m_vkDevice;
	pipelineCreateInfo.m_vulkanWindow = m_vulkanWindow;
    pipelineCreateInfo.m_uiBasedPipeline = false;
	m_mainGraphicsPipeline = std::make_shared<GraphicsPipeline>(pipelineCreateInfo);
}

void VulkanInterface::CreateUIGraphicsPipeline()
{
    if (m_uiGraphicsPipeline != VK_NULL_HANDLE)
    {
        m_uiGraphicsPipeline->SetDescriptorSetLayout(m_uiDescriptorSetLayout);
        m_uiGraphicsPipeline->CreatePipeline();
        return;
    }

    GraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.m_vertexShaderFilePath = "shaders/HLSL/UIVertexShader.spv";
    pipelineCreateInfo.m_fragmentShaderFilePath = "shaders/HLSL/UIPixelShader.spv";
    pipelineCreateInfo.m_descriptorSetLayout = m_uiDescriptorSetLayout;
    pipelineCreateInfo.m_device = m_vkDevice;
    pipelineCreateInfo.m_vulkanWindow = m_vulkanWindow;
	pipelineCreateInfo.m_uiBasedPipeline = true;
    m_uiGraphicsPipeline = std::make_shared<GraphicsPipeline>(pipelineCreateInfo);
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

std::shared_ptr<GraphicsBuffer> VulkanInterface::CreateUIVertexBuffer(const std::shared_ptr<UIMeshRenderer>& imageObject) const
{
    VkDeviceSize bufferSize = sizeof(VulkanCommonFunctions::UIVertex) * imageObject->GetVertices().size();

    GraphicsBuffer::BufferCreateInfo stagingBufferCreateInfo = {};
    stagingBufferCreateInfo.size = bufferSize;
    stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingBufferCreateInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    stagingBufferCreateInfo.allocator = m_vmaAllocator;
    stagingBufferCreateInfo.commandPool = m_commandPool;
    stagingBufferCreateInfo.graphicsQueue = m_graphicsQueue;
    stagingBufferCreateInfo.device = m_vkDevice;

    const std::vector< VulkanCommonFunctions::UIVertex>& vertices = imageObject->GetVertices();

    std::shared_ptr<GraphicsBuffer> stagingBuffer = std::make_shared<GraphicsBuffer>(stagingBufferCreateInfo);
    stagingBuffer->LoadData((void*)vertices.data(), (size_t)bufferSize);

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

std::shared_ptr<GraphicsBuffer> VulkanInterface::CreateVertexBuffer(const std::shared_ptr<MeshRenderer>& meshInfo) const
{
    VkDeviceSize bufferSize = sizeof(VulkanCommonFunctions::Vertex) * meshInfo->GetVertices().size();

    GraphicsBuffer::BufferCreateInfo stagingBufferCreateInfo = {};
	stagingBufferCreateInfo.size = bufferSize;
	stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingBufferCreateInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	stagingBufferCreateInfo.allocator = m_vmaAllocator;
	stagingBufferCreateInfo.commandPool = m_commandPool;
	stagingBufferCreateInfo.graphicsQueue = m_graphicsQueue;
    stagingBufferCreateInfo.device = m_vkDevice;

	const std::vector< VulkanCommonFunctions::Vertex>& vertices = meshInfo->GetVertices();

	std::shared_ptr<GraphicsBuffer> stagingBuffer = std::make_shared<GraphicsBuffer>(stagingBufferCreateInfo);
    stagingBuffer->LoadData((void*)vertices.data(), (size_t)bufferSize);

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

void VulkanInterface::UpdateObjectBuffers(const std::shared_ptr<MeshRenderer>& objectMesh)
{
    if (objectMesh->GetMeshName() == MeshRenderer::kCustomMeshName)
    {
        return;
    }

    if (m_vertexBuffers.contains(objectMesh->GetMeshName()) || (objectMesh->IsIndexed() && m_indexBuffers.contains(objectMesh->GetMeshName())))
    {
        return;
    }

    std::shared_ptr<GraphicsBuffer> indexBuffer = CreateIndexBuffer(objectMesh);
    m_indexBuffers[objectMesh->GetMeshName()] = indexBuffer;
    m_indexBufferSizes[objectMesh->GetMeshName()] = static_cast<uint16_t>(objectMesh->GetIndices().size());

    std::shared_ptr<GraphicsBuffer> vertexBuffer = CreateVertexBuffer(objectMesh);
    m_vertexBuffers[objectMesh->GetMeshName()] = vertexBuffer;
    m_vertexBufferSizes[objectMesh->GetMeshName()] = static_cast<uint16_t>(objectMesh->GetVertices().size());

    CreateInstanceBuffer(objectMesh);
}

std::shared_ptr<GraphicsBuffer> VulkanInterface::CreateUIIndexBuffer(const std::shared_ptr<UIMeshRenderer>& imageObject) const
{
    VkDeviceSize bufferSize = sizeof(uint16_t) * imageObject->GetIndices().size();

    GraphicsBuffer::BufferCreateInfo stagingBufferCreateInfo = {};
    stagingBufferCreateInfo.size = bufferSize;
    stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingBufferCreateInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    stagingBufferCreateInfo.allocator = m_vmaAllocator;
    stagingBufferCreateInfo.commandPool = m_commandPool;
    stagingBufferCreateInfo.graphicsQueue = m_graphicsQueue;
    stagingBufferCreateInfo.device = m_vkDevice;

    const std::vector<uint16_t>& indices = imageObject->GetIndices();

    std::shared_ptr<GraphicsBuffer> stagingBuffer = std::make_shared<GraphicsBuffer>(stagingBufferCreateInfo);
    stagingBuffer->LoadData((void*)indices.data(), (size_t)bufferSize);

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

std::shared_ptr<GraphicsBuffer> VulkanInterface::CreateIndexBuffer(const std::shared_ptr<MeshRenderer>&  meshInfo) const
{
    VkDeviceSize bufferSize = sizeof(uint16_t) * meshInfo->GetIndices().size();

    GraphicsBuffer::BufferCreateInfo stagingBufferCreateInfo = {};
    stagingBufferCreateInfo.size = bufferSize;
    stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingBufferCreateInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    stagingBufferCreateInfo.allocator = m_vmaAllocator;
    stagingBufferCreateInfo.commandPool = m_commandPool;
    stagingBufferCreateInfo.graphicsQueue = m_graphicsQueue;
    stagingBufferCreateInfo.device = m_vkDevice;

	const std::vector<uint16_t>& indices = meshInfo->GetIndices();

    std::shared_ptr<GraphicsBuffer> stagingBuffer = std::make_shared<GraphicsBuffer>(stagingBufferCreateInfo);
    stagingBuffer->LoadData((void*)indices.data(), (size_t)bufferSize);

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

void VulkanInterface::CreateInstanceBuffer(const std::shared_ptr<MeshRenderer>& object)
{
    if (object->GetMeshName() == MeshRenderer::kCustomMeshName)
    {
        return;
    }

    for (uint32_t frameIndex = 0; frameIndex < MAX_FRAMES_IN_FLIGHT; frameIndex++)
    {
        std::shared_ptr<GraphicsBuffer> instanceBuffer = CreateInstanceBuffer(VulkanCommonFunctions::MAX_OBJECTS);
		m_instanceBuffers[frameIndex][object->GetMeshName()] = instanceBuffer;
    }
}

std::shared_ptr<GraphicsBuffer> VulkanInterface::CreateInstanceBuffer(size_t maxObjects) const
{
    VkDeviceSize bufferSize = sizeof(VulkanCommonFunctions::InstanceInfo) * maxObjects;

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

void VulkanInterface::UpdateInstanceBuffer(const std::string& objectName, const std::set<VulkanCommonFunctions::ObjectHandle>& objectHandles, std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>>& objects)
{
    std::vector<VulkanCommonFunctions::InstanceInfo> objectInfo;

    for (auto it = objectHandles.begin(); it != objectHandles.end(); it++)
    {
        VulkanCommonFunctions::ObjectHandle currentHandle = *it;

        if (currentHandle == VulkanCommonFunctions::INVALID_OBJECT_HANDLE || !objects.contains(currentHandle))
        {
            continue;
        }

        std::shared_ptr<RenderObject> object = objects[currentHandle];

		std::shared_ptr<MeshRenderer> meshRenderer = object->GetComponent<MeshRenderer>();

        if (meshRenderer == nullptr)
            continue;

        if (!meshRenderer->IsEnabled())
            continue;

        VulkanCommonFunctions::InstanceInfo info = object->GetInstanceInfo(m_textureFilePaths);

        objectInfo.push_back(info);
    }

    if (objectInfo.empty())
    {
        return; 
    }

    VkDeviceSize bufferSize = objectHandles.size() * sizeof(VulkanCommonFunctions::InstanceInfo);

	m_instanceBuffers[m_currentFrameIndex][objectName]->LoadData(objectInfo.data(), (size_t)bufferSize);
}

void VulkanInterface::SwitchToUIPipeline(VkCommandBuffer commandBuffer) const
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_uiGraphicsPipeline->GetVkPipeline());

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_uiGraphicsPipeline->GetVkPipelineLayout(), 0, 1, &m_uiDescriptorSets[m_currentFrameIndex], 0, nullptr);
}

void VulkanInterface::DrawUIImageCommandBuffer(VkCommandBuffer commandBuffer, const std::shared_ptr<RenderObject>& currentObject) const
{
    std::shared_ptr<UIImage> imageComponent = currentObject->GetComponent<UIImage>();
    VkBuffer objectVertexBuffer[] = { imageComponent->GetVertexBuffer()->GetVkBuffer(), currentObject->GetUIInstanceBuffer(m_textureFilePaths)->GetVkBuffer() };
    VkDeviceSize offsets[] = { 0, 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 2, objectVertexBuffer, offsets);

    vkCmdBindIndexBuffer(commandBuffer, imageComponent->GetIndexBuffer()->GetVkBuffer(), 0, VK_INDEX_TYPE_UINT16);

    vkCmdDrawIndexed(commandBuffer, imageComponent->GetIndexBufferSize(), 1, 0, 0, 0);
}

void VulkanInterface::DrawUITextCommandBuffer(VkCommandBuffer commandBuffer, const std::shared_ptr<RenderObject>& currentObject, const std::shared_ptr<FontManager>& fontManager)
{
    std::shared_ptr<Text> textComponent = currentObject->GetComponent<Text>();

    GraphicsBuffer::BufferCreateInfo createInfo;
    createInfo.allocator = m_vmaAllocator;
    createInfo.commandPool = m_commandPool;
    createInfo.device = m_vkDevice;
    createInfo.graphicsQueue = m_graphicsQueue;
    createInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    createInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    std::pair<size_t, size_t> screenSize = {m_vulkanWindow->swapChainImageSize().width(), m_vulkanWindow->swapChainImageSize().height() };

    std::string fontName = textComponent->GetFontName();
    std::shared_ptr<Font> font = fontManager->GetFontByName(fontName);

    std::string atlasFilePath = font->GetAtlasFilePath();

    if (!m_texturePathToIndex.contains(atlasFilePath))
    {
        std::cerr << "Font atlas hasn't been loaded as a texture image: " << atlasFilePath << std::endl;
        return;
    }

    size_t textureIndex = m_texturePathToIndex[atlasFilePath];

    textComponent->UpdateInstanceBuffer(screenSize, font, textureIndex, createInfo);

    VkBuffer objectVertexBuffer[] = { textComponent->GetVertexBuffer()->GetVkBuffer(), textComponent->GetInstanceBuffer()->GetVkBuffer()};
    VkDeviceSize offsets[] = { 0, 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 2, objectVertexBuffer, offsets);

    vkCmdBindIndexBuffer(commandBuffer, textComponent->GetIndexBuffer()->GetVkBuffer(), 0, VK_INDEX_TYPE_UINT16);

    vkCmdDrawIndexed(commandBuffer, textComponent->GetIndexBufferSize(), textComponent->GetTextString().size(), 0, 0, 0);
}

void VulkanInterface::DrawUIElementCommandBuffer(VkCommandBuffer commandBuffer, const std::shared_ptr<RenderObject>& currentObject, const std::shared_ptr<FontManager>& fontManager)
{
    std::shared_ptr<UIImage> imageComponent = currentObject->GetComponent<UIImage>();

    if (imageComponent != nullptr)
    {
        DrawUIImageCommandBuffer(commandBuffer, currentObject);
    }

    std::shared_ptr<Text> textComponent = currentObject->GetComponent<Text>();
    
    if (textComponent != nullptr)
    {
        DrawUITextCommandBuffer(commandBuffer, currentObject, fontManager);
    }
}

void VulkanInterface::DrawFrame(float deltaTime, const std::shared_ptr<Scene>& scene, const std::shared_ptr<FontManager>& fontManager) {
    vkDeviceWaitIdle(m_vkDevice);

    if (m_swapChainReady == false)
    {
        m_vulkanWindow->requestUpdate();
        return;
    }

    std::map<std::string, std::set<VulkanCommonFunctions::ObjectHandle>> objectHandles = scene->GetMeshNameToObjectMap();
    std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>> objects = scene->GetObjects();
    std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>> uiObjects = scene->GetUIObjects();

    for (auto it = objectHandles.begin(); it != objectHandles.end(); it++)
    {
        if (it->first != MeshRenderer::kCustomMeshName)
        {
            UpdateInstanceBuffer(it->first, it->second, objects);
        }
    }

    UpdateUniformBuffer(m_currentFrameIndex, objects);

    VkCommandBuffer commandBuffer = m_vulkanWindow->currentCommandBuffer();

    BeginDrawFrameCommandBuffer(commandBuffer);

    for (auto it = objectHandles.begin(); it != objectHandles.end(); it++)
    {
        if (it->first == MeshRenderer::kCustomMeshName)
        {
            for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
            {
				VulkanCommonFunctions::ObjectHandle currentHandle = *it2;
				DrawSingleObjectCommandBuffer(commandBuffer, objects[currentHandle]);
            }
        }
        else {
            DrawInstancedObjectCommandBuffer(commandBuffer, it->first, it->second.size());
        }
    }

    //update to UI pipeline
	SwitchToUIPipeline(commandBuffer);

    //draw UI elements
    for (auto it = uiObjects.begin(); it != uiObjects.end(); it++)
    {
		DrawUIElementCommandBuffer(commandBuffer, it->second, fontManager);
    }

    EndDrawFrameCommandBuffer(commandBuffer);

    m_currentFrameIndex = (m_currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
    m_renderedFirstFrame = true;

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

	m_mainGraphicsPipeline->DestroyPipeline();
    m_uiGraphicsPipeline->DestroyPipeline();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		m_uniformBuffers[i]->DestroyBuffer();
		m_lightInfoBuffers[i]->DestroyBuffer();
        m_uiUniformBuffers[i]->DestroyBuffer();
    }

    for (auto it = m_textureFilePaths.begin(); it != m_textureFilePaths.end(); it++)
    {
		m_textureImages[*it]->DestroyTextureImage();
    }

    vkDestroyDescriptorPool(m_vkDevice, m_primaryDescriptorPool, nullptr);
	vkDestroyDescriptorPool(m_vkDevice, m_uiDescriptorPool, nullptr);

    vkDestroyDescriptorSetLayout(m_vkDevice, m_primaryDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(m_vkDevice, m_uiDescriptorSetLayout, nullptr);

    for (auto it = m_indexBuffers.begin(); it != m_indexBuffers.end(); it++)
    {
		it->second->DestroyBuffer();
    }

    for (auto it = m_vertexBuffers.begin(); it != m_vertexBuffers.end(); it++)
    {
		it->second->DestroyBuffer();
    }

    for (uint32_t frameIndex = 0; frameIndex < MAX_FRAMES_IN_FLIGHT; frameIndex++)
    {
        for (auto it = m_instanceBuffers[frameIndex].begin(); it != m_instanceBuffers[frameIndex].end(); it++)
        {
            it->second->DestroyBuffer();
        }
    }

    vmaDestroyAllocator(m_vmaAllocator);
}