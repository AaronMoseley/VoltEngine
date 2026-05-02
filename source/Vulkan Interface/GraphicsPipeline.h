#pragma once

#include <fstream>

#include "Vulkan Interface/VulkanCommonFunctions.h"

class VulkanWindow;

struct GraphicsPipelineCreateInfo {
	std::string m_vertexShaderFilePath;
	std::string m_fragmentShaderFilePath;
	VkDescriptorSetLayout m_descriptorSetLayout;
	VkDevice m_device;
	VulkanWindow* m_vulkanWindow;
	bool m_uiBasedPipeline = false;
};

class GraphicsPipeline {
public: 
	GraphicsPipeline(const GraphicsPipelineCreateInfo& pipelineCreateInfo);

	void CreatePipeline();
	void DestroyPipeline();

	void SetDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) { m_descriptorSetLayout = descriptorSetLayout; }

	VkPipeline GetVkPipeline() const { return m_graphicsPipeline; }
	VkPipelineLayout GetVkPipelineLayout() const { return m_pipelineLayout; }

private:
	void CreatePipelineLayout();
	VkShaderModule CreateShaderModule(const std::vector<char>& code) const;
	static std::vector<char> ReadFile(const std::string& filename);

	VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
	VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
	VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
	VkDevice m_device = VK_NULL_HANDLE;

	std::string m_vertexShaderFilePath;
	std::string m_fragmentShaderFilePath;

	bool m_uiBasedPipeline = false;

	VulkanWindow* m_vulkanWindow;
};