#pragma once

#include "Vulkan Interface/GraphicsBuffer.h"
#include "Vulkan Interface/VulkanCommonFunctions.h"

class GraphicsImage {
public:
	struct GraphicsImageCreateInfo {
		std::pair<size_t, size_t> imageSize;
		VkFormat format;
		VkImageTiling tiling;
		VkImageUsageFlags usage;
		VkMemoryPropertyFlags properties;
		VmaAllocator allocator;
		VkDevice device;
		VkCommandPool commandPool;
		VkQueue graphicsQueue;
	};

	GraphicsImage(const GraphicsImageCreateInfo& imageCreateInfo);
	GraphicsImage(const GraphicsImageCreateInfo& imageCreateInfo, VkImage rawImage, VkImageView imageView);

	VkImageView GetImageView() const { return (m_createdImageView) ? m_imageView : VK_NULL_HANDLE; }
	VkFormat GetImageFormat() const { return m_imageFormat; }

	void CreateImageView(VkImageAspectFlags aspectFlags);
	void CopyFromBuffer(const GraphicsBuffer* buffer) const;
	void TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout) const;
	VkImage GetVkImage() const { return m_image; }
	void DestroyImage() const;
	void DestroyImageView() const;

protected:
	VkImage m_image;
	VmaAllocation m_imageMemory;

	VkFormat m_imageFormat;
	VkImageView m_imageView = nullptr;

	VmaAllocator m_allocator;
	VkDevice m_device;
	VkCommandPool m_commandPool;
	VkQueue m_graphicsQueue;

	bool m_createdImageView = false;

	std::pair<size_t, size_t> m_imageSize;
};