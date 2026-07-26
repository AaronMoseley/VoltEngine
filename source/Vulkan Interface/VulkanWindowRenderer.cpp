#include "VulkanWindowRenderer.h"
#include "Vulkan Interface/VulkanInterface.h"

VulkanWindowRenderer::VulkanWindowRenderer(const std::shared_ptr<VulkanInterface>& vulkanInterface, const std::shared_ptr<Scene>& scene)
{
	m_scene = scene;
}

void VulkanWindowRenderer::preInitResources()
{
	
}

void VulkanWindowRenderer::initResources()
{
	VulkanInterface::Get()->InitializeVulkan();
}

void VulkanWindowRenderer::initSwapChainResources()
{
	VulkanInterface::Get()->CreateDepthResources();

	VulkanInterface::Get()->SetSwapChainReady(true);
}

void VulkanWindowRenderer::releaseResources()
{
	VulkanInterface::Get()->SetSwapChainReady(false);

	m_scene->Cleanup();
	VulkanInterface::Get()->Cleanup();
}

void VulkanWindowRenderer::releaseSwapChainResources()
{
	VulkanInterface::Get()->CleanupSwapChain();
}

void VulkanWindowRenderer::startNextFrame()
{
	m_scene->Update();

	if (!m_isShuttingDown)
	{
		VulkanInterface::Get()->DrawFrame(0.1f, m_scene);
	}
}

void VulkanWindowRenderer::Shutdown()
{
	m_isShuttingDown = true;
}