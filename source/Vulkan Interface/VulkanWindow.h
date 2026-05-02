#pragma once

#include "Vulkan Interface/VulkanWindowRenderer.h"

#include <QVulkanWindow>

#include <QKeyEvent>
#include <QMouseEvent>

class VulkanInterface;
class Scene;

class VulkanWindow : public QVulkanWindow {

	Q_OBJECT

public:
	VulkanWindow(const std::shared_ptr<VulkanInterface>& vulkanInterface, const std::shared_ptr<Scene>& scene);

	QVulkanWindowRenderer* createRenderer() override;

	void RegisterMouseMoveCallback(const std::function<void(QMouseEvent*)>& callback) { m_mouseMoveCallbacks.push_back(callback); };

	void Shutdown() const;

	void SetLockCursor(bool lock);
	bool IsCursorLocked() const { return m_lockCursor; }

	void SetTrackingMouse(bool isTracking) { m_isTrackingMouse = isTracking; };
	bool IsTrackingMouse() const { return m_isTrackingMouse; }

signals:
	void KeyDown(Qt::Key pressedKey);
	void KeyUp(Qt::Key releasedKey);

	void MouseButtonDown(Qt::MouseButton pressedButton);
	void MouseButtonUp(Qt::MouseButton releasedButton);

	void MouseMoved(float x, float y);
	void ManualMouseMove(float x, float y);

	void Resized(QSize newSize, QSize oldSize);

protected:
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

	// Keyboard events
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

	void resizeEvent(QResizeEvent* event) override;

private:
	std::shared_ptr<VulkanInterface> m_vulkanInterface;
	std::shared_ptr<Scene> m_scene;
	VulkanWindowRenderer* m_vulkanWindowRenderer = nullptr;

	std::vector<std::function<void(QMouseEvent*)>> m_mouseMoveCallbacks;

	VkPhysicalDeviceDescriptorIndexingFeatures m_indexingFeatures{};

	bool m_manuallyMovingMouse = false;
	bool m_lockCursor = false;
	bool m_isTrackingMouse = true;
};