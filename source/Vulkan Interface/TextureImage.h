#pragma once

#include "Vulkan Interface/GraphicsImage.h"

class TextureImage : public GraphicsImage {
public:
	TextureImage(const GraphicsImageCreateInfo& imageCreateInfo) : GraphicsImage(imageCreateInfo) {};

	void CreateTextureSampler(float maxAnisotropy);
	VkSampler GetTextureSampler() const { return m_textureSampler; }

	void DestroyTextureImage() const;

private:
	VkSampler m_textureSampler = VK_NULL_HANDLE;
};