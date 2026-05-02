#pragma once

#include <QVulkanWindowRenderer>
#include "Management/Scene.h"

class VulkanInterface;

class VulkanWindowRenderer : public QVulkanWindowRenderer {
public:
	VulkanWindowRenderer(const std::shared_ptr<VulkanInterface>& vulkanInterface, const std::shared_ptr<Scene>& scene);

	void preInitResources() override;
	void initResources() override;
	void initSwapChainResources() override;
	void releaseSwapChainResources() override;
	void releaseResources() override;
	void startNextFrame() override;

	void Shutdown();

private:
	std::shared_ptr<VulkanInterface> m_vulkanInterface;
	std::shared_ptr<Scene> m_scene;

	bool m_isShuttingDown = false;
};