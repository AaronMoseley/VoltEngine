#pragma once

#include <string>
#include <set>

#include <glm.hpp>

#include <qwidget.h>
#include <QVBoxLayout>
#include <QVulkanInstance>
#include <QMetaMethod>
#include <QPushButton>

class Scene;
class VulkanWindow;
class VulkanInterface;

class WindowManager : public QVBoxLayout {

	Q_OBJECT

public:
	const char* DEFAULT_TITLE = "GLFW Window";

	WindowManager(QWidget* parentProgram, size_t width, size_t height, const std::string& title);

	void SetVulkanInterface(const std::shared_ptr<VulkanInterface>& vulkanInterface) { m_vulkanInterface = vulkanInterface; }
	void SetScene(const std::shared_ptr<Scene>& scene) { m_scene = scene; }
	void BeginRendering() const;

	void AddWidgetToMenu(const std::string& widgetName, QWidget* newWidget);
	void RemoveWidgetFromMenu(const std::string& widgetName);

	void AddButton(const std::string& title, const std::function<void()>& callback);
	void RemoveButton(const std::string& title);

	void InitializeWindow(QVulkanInstance* vulkanInstance);

	QWidget* GetWrappingWidget() const { return m_wrappingWidget; }

	size_t GetWidth() const { return m_width; }
	size_t GetHeight() const { return m_height; }

	glm::vec2 GetMouseDelta() const { return m_mouseDelta; };
	glm::vec2 GetScrollDelta() const { return m_scrollDelta; };

	VulkanWindow* GetVulkanWindow() const { return m_vulkanWindow; }

	void NewFrame();

	bool KeyPressed(Qt::Key keyCode) const;
	bool KeyPressedThisFrame(Qt::Key keyCode) const;

	bool MouseButtonPressed(Qt::MouseButton mouseButton) const;
	bool MouseButtonPressedThisFrame(Qt::MouseButton mouseButton) const;

	void SetFrameBufferResized() { m_framebufferResized = true; };

	void Shutdown() const;

	void SetLockCursor(bool lockCursor) const;
	bool IsCursorLocked() const;

	void SetIsTrackingMouse(bool isTracking) const;
	bool IsTrackingMouse() const;

public slots:
	void AddKeyDown(Qt::Key pressedKey);
	void AddKeyUp(Qt::Key releasedKey);

	void AddMouseButtonDown(Qt::MouseButton pressedButton);
	void AddMouseButtonUp(Qt::MouseButton releasedButton);

	void CursorMoved(float xpos, float ypos);
	void UpdateManualMousePosition(float xpos, float ypos);

	void OnResize(QSize newSize, QSize oldSize) const;

private:
	VulkanWindow* m_vulkanWindow = nullptr;
	std::shared_ptr<VulkanInterface> m_vulkanInterface = nullptr;
	std::shared_ptr<Scene> m_scene = nullptr;

	QWidget* m_parentProgram;
	QWidget* m_wrappingWidget;
	QHBoxLayout* m_menuLayout;

	std::map<std::string, QWidget*> m_menuWidgets;
	std::map<std::string, QPushButton*> m_buttons;

	//members
	bool m_framebufferResized = false;
	size_t m_width = 0;
	size_t m_height = 0;

	glm::vec2 m_currentMousePosition = glm::vec2(0.0f, 0.0f);
	glm::vec2 m_lastMousePosition = glm::vec2(0.0f, 0.0f);
	glm::vec2 m_mouseDelta = glm::vec2(0.0f, 0.0f);
	bool m_firstMouseMovement = true;

	glm::vec2 m_scrollDelta = glm::vec2(0.0f, 0.0f);

	std::set<Qt::Key> m_pressedKeys;
	std::set<Qt::Key> m_keysPressedThisFrame;

	std::set<Qt::MouseButton> m_pressedMouseButtons;
	std::set<Qt::MouseButton> m_pressedMouseButtonsThisFrame;

	std::string m_title = "";
};