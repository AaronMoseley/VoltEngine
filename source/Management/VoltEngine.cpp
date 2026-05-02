#include "VoltEngine.h"

VoltEngine::VoltEngine(QWidget* parent, QVulkanInstance* vulkanInstance, int screenWidth, int screenHeight) : QWidget(parent)
{
    m_windowManager = new WindowManager(this, screenWidth, screenHeight, "Vulkan Lighting Demo");

    m_vulkanInterface = std::make_shared<VulkanInterface>(m_windowManager);

    m_windowManager->SetVulkanInterface(m_vulkanInterface);

    m_sceneManager = std::make_shared<Scene>(m_windowManager, m_vulkanInterface);

    m_windowManager->SetScene(m_sceneManager);

    m_windowManager->InitializeWindow(vulkanInstance);
    resize(screenWidth, screenHeight);

    //can add custom callbacks that get called each frame
    //auto frameCallback = std::bind(&VulkanLightingDemo::processInput, this, std::placeholders::_1);
    //sceneManager->RegisterUpdateCallback(frameCallback);
}

void VoltEngine::BeginRendering() const
{
    m_windowManager->BeginRendering();
}

void VoltEngine::RegisterUpdateCallback(const std::function<void(float)>& callback) const
{
    m_sceneManager->RegisterUpdateCallback(callback);
}
