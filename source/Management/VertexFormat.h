#ifndef VOLTENGINE_VERTEXFORMAT_H
#define VOLTENGINE_VERTEXFORMAT_H

#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>

class VertexFormat {
public:
	VertexFormat();

	void AddNewBinding(uint32_t bindingLocation, VkVertexInputRate currentInputRate);
	void AddAttributeToBinding(VkFormat attributeFormat);

	void Finalize();

	void GetBindingDescriptions(std::vector<VkVertexInputBindingDescription>& outBindingDescriptions) const { outBindingDescriptions = m_bindingDescriptions; }
	void GetAttributeDescriptions(std::vector<VkVertexInputAttributeDescription>& outAttributeDescriptions) const { outAttributeDescriptions = m_attributeDescriptions; }

private:
	void ApplyCurrentBindingDescription();
	static uint32_t GetAttributeSize(VkFormat attributeFormat);

	std::vector<VkVertexInputBindingDescription> m_bindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions;

	VkVertexInputRate m_currentInputRate;
	uint32_t m_currentBinding = 0;
	uint32_t m_currentOffset = 0;
	uint32_t m_currentLocation = 0;

	bool m_bindingValid = false;
};



#endif //VOLTENGINE_VERTEXFORMAT_H
