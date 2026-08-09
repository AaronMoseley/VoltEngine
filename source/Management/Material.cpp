#include "Material.h"
#include "Vulkan Interface/TextureImage.h"
#include "Objects/RenderObject.h"

void Material::InitializeMaterial(const MaterialRegistry::MaterialCreationData& creationData)
{
	m_allocator = creationData.m_allocator;
	m_commandPool = creationData.m_commandPool;
	m_graphicsQueue = creationData.m_graphicsQueue;
	m_vulkanWindow = creationData.m_vulkanWindow;
	m_vkDevice = creationData.m_vkDevice;

	CreateVertexFormat();
	CreateDescriptorSetLayout(creationData);
	CreateGraphicsPipeline();
	CreateDescriptorPool(creationData);
	CreateDescriptorSets(creationData);
}

void Material::CleanupMaterial()
{
	m_graphicsPipeline->DestroyPipeline();
	vkDestroyDescriptorPool(m_vkDevice, m_descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(m_vkDevice, m_descriptorSetLayout, nullptr);
}

void Material::CreateGraphicsPipeline()
{
	if (m_graphicsPipeline != VK_NULL_HANDLE)
	{
		m_graphicsPipeline->SetDescriptorSetLayout(m_descriptorSetLayout);
		m_graphicsPipeline->CreatePipeline();
		return;
	}

	GraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.m_vertexShaderFilePath = GetVertexShaderPath();
	pipelineCreateInfo.m_fragmentShaderFilePath = GetPixelShaderPath();
	pipelineCreateInfo.m_descriptorSetLayout = m_descriptorSetLayout;
	pipelineCreateInfo.m_device = m_vkDevice;
	pipelineCreateInfo.m_vulkanWindow = m_vulkanWindow;
	pipelineCreateInfo.m_vertexFormat = m_vertexFormat;
	pipelineCreateInfo.m_allowTransparency = m_allowTransparency;
	m_graphicsPipeline = std::make_shared<GraphicsPipeline>(pipelineCreateInfo);
}

void Material::CreateDescriptorSetLayout(const MaterialRegistry::MaterialCreationData& creationData)
{
	if (m_descriptorSetLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(m_vkDevice, m_descriptorSetLayout, nullptr);
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
	samplerLayoutBinding.descriptorCount = creationData.m_textureFilePaths.size();
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 3> bindings = { globalInfoLayoutBinding, lightInfoBinding, samplerLayoutBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(m_vkDevice, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void Material::CreateDescriptorSets(const MaterialRegistry::MaterialCreationData& creationData)
{
	std::vector<VkDescriptorSetLayout> layouts(VulkanCommonFunctions::MAX_FRAMES_IN_FLIGHT, m_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(VulkanCommonFunctions::MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(m_vkDevice, &allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < VulkanCommonFunctions::MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = creationData.m_uniformBuffers[i]->GetVkBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(VulkanCommonFunctions::GlobalInfo);

        VkDescriptorBufferInfo lightBufferInfo{};
        lightBufferInfo.buffer = creationData.m_lightInfoBuffers[i]->GetVkBuffer();
        lightBufferInfo.offset = 0;
        lightBufferInfo.range = sizeof(VulkanCommonFunctions::LightInfo) * creationData.m_maxLightCount;

        std::vector<VkDescriptorImageInfo> imageInfos;

        for (auto it = creationData.m_textureFilePaths.begin(); it != creationData.m_textureFilePaths.end(); it++)
        {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = creationData.m_textureImages.at(*it)->GetImageView();
            imageInfo.sampler = creationData.m_textureImages.at(*it)->GetTextureSampler();

            imageInfos.push_back(imageInfo);
        }

        std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = m_descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &lightBufferInfo;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = m_descriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[2].descriptorCount = imageInfos.size();
        descriptorWrites[2].pImageInfo = imageInfos.data();

        vkUpdateDescriptorSets(m_vkDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void Material::CreateDescriptorPool(const MaterialRegistry::MaterialCreationData& creationData)
{
	if (m_descriptorPool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(m_vkDevice, m_descriptorPool, nullptr);
	}

	std::array<VkDescriptorPoolSize, 3> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(VulkanCommonFunctions::MAX_FRAMES_IN_FLIGHT);
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(VulkanCommonFunctions::MAX_FRAMES_IN_FLIGHT);
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[2].descriptorCount = static_cast<uint32_t>(VulkanCommonFunctions::MAX_FRAMES_IN_FLIGHT) * creationData.m_textureFilePaths.size();

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(VulkanCommonFunctions::MAX_FRAMES_IN_FLIGHT);

	if (vkCreateDescriptorPool(m_vkDevice, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}