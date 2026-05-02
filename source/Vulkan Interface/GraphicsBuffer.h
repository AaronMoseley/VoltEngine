#pragma once

//#include "vk_mem_alloc.h"

#include <cstdlib>
#include <stdexcept>
#include <memory>
#include <cstring>

#include "Vulkan Interface/VulkanCommonFunctions.h"

class GraphicsBuffer {
public:
	struct BufferCreateInfo {
		VmaAllocator allocator;
		VkDeviceSize size = 0;
		VkBufferUsageFlags usage;
		VkMemoryPropertyFlags properties;
		VkDevice device;
		VkCommandPool commandPool;
		VkQueue graphicsQueue;
	};

	GraphicsBuffer(const BufferCreateInfo& createInfo);

	VkBuffer GetVkBuffer() const { return m_buffer; }

	void CopyBuffer(const std::shared_ptr<GraphicsBuffer>& destintationBuffer, VkDeviceSize copySize) const;
	void LoadData(const void* data, size_t memorySize) const;
	void DestroyBuffer() const;

private:
	VkBuffer m_buffer = VK_NULL_HANDLE;
	VmaAllocation m_allocation = VK_NULL_HANDLE;

	VkMemoryPropertyFlags m_properties = 0;

	VmaAllocator m_allocator = VK_NULL_HANDLE;

	VkDevice m_device = VK_NULL_HANDLE;
	VkCommandPool m_commandPool = VK_NULL_HANDLE;
	VkQueue m_graphicsQueue = VK_NULL_HANDLE;

	size_t m_maxSize = 0;

	void* m_mappedData = nullptr;
};
