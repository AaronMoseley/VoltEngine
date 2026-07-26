#include "VertexFormat.h"

VertexFormat::VertexFormat()
{

}

void VertexFormat::Finalize()
{
	ApplyCurrentBindingDescription();
}

void VertexFormat::ApplyCurrentBindingDescription()
{
	VkVertexInputBindingDescription newBindingDescription;

	newBindingDescription.binding = m_currentBinding;
	newBindingDescription.stride = m_currentOffset;
	newBindingDescription.inputRate = m_currentInputRate;

	m_bindingDescriptions.push_back(newBindingDescription);
}

void VertexFormat::AddNewBinding(uint32_t bindingLocation, VkVertexInputRate currentInputRate)
{
	if (m_bindingValid)
	{
		ApplyCurrentBindingDescription();
	}

	m_currentBinding = bindingLocation;
	m_currentOffset = 0;
	m_currentInputRate = currentInputRate;
	m_bindingValid = true;
}

void VertexFormat::AddAttributeToBinding(VkFormat attributeFormat)
{
	VkVertexInputAttributeDescription newAttributeDescription;
	uint32_t attributeSize = GetAttributeSize(attributeFormat);

	newAttributeDescription.binding = m_currentBinding;
	newAttributeDescription.location = m_currentLocation;
	newAttributeDescription.format = attributeFormat;
	newAttributeDescription.offset = m_currentOffset;

	m_attributeDescriptions.push_back(newAttributeDescription);

	m_currentLocation++;
	m_currentOffset += attributeSize;
}

uint32_t VertexFormat::GetAttributeSize(VkFormat attributeFormat)
{
	switch (attributeFormat)
	{
	case VK_FORMAT_R16_UINT:
		return sizeof(uint16_t);
	case VK_FORMAT_R32_UINT:
	case VK_FORMAT_R32_SINT:
	case VK_FORMAT_R32_SFLOAT:
		return sizeof(float);
	case VK_FORMAT_R32G32_UINT:
	case VK_FORMAT_R32G32_SINT:
	case VK_FORMAT_R32G32_SFLOAT:
		return sizeof(float) * 2;
	case VK_FORMAT_R32G32B32_UINT:
	case VK_FORMAT_R32G32B32_SINT:
	case VK_FORMAT_R32G32B32_SFLOAT:
		return sizeof(float) * 3;
	case VK_FORMAT_R32G32B32A32_UINT:
	case VK_FORMAT_R32G32B32A32_SINT:
	case VK_FORMAT_R32G32B32A32_SFLOAT:
		return sizeof(float) * 4;
	case VK_FORMAT_R16G16_UINT:
	case VK_FORMAT_R16G16_SINT:
	case VK_FORMAT_R16G16_SFLOAT:
		return sizeof(uint16_t) * 2;
	case VK_FORMAT_R16G16B16A16_UINT:
	case VK_FORMAT_R16G16B16A16_SINT:
	case VK_FORMAT_R16G16B16A16_UNORM:
	case VK_FORMAT_R16G16B16A16_SNORM:
	case VK_FORMAT_R16G16B16A16_SFLOAT:
		return sizeof(uint16_t) * 4;
	case VK_FORMAT_R8G8B8A8_UINT:
	case VK_FORMAT_R8G8B8A8_SINT:
	case VK_FORMAT_R8G8B8A8_UNORM:
	case VK_FORMAT_R8G8B8A8_SNORM:
		return sizeof(uint8_t) * 4;
	}

	return 0;
}