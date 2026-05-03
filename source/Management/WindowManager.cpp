#include "WindowManager.h"
#include "Management/Scene.h"
#include "Vulkan Interface/VulkanWindow.h"
#include "Vulkan Interface/VulkanInterface.h"

WindowManager::WindowManager(QWidget* parentProgram, size_t width, size_t height, const std::string& title) :
    QVBoxLayout(parentProgram), m_parentProgram(parentProgram), m_wrappingWidget(nullptr), m_menuLayout(nullptr),
    m_width(width), m_height(height), m_title(title)
{
}

void WindowManager::BeginRendering() const
{
    m_vulkanWindow->requestUpdate();
}

void WindowManager::InitializeWindow(QVulkanInstance* vulkanInstance)
{
	m_menuLayout = new QHBoxLayout();
	addLayout(m_menuLayout);

    m_vulkanWindow = new VulkanWindow(m_vulkanInterface, m_scene);
    m_vulkanWindow->setVulkanInstance(vulkanInstance);

    QObject::connect(m_vulkanWindow, &VulkanWindow::KeyDown, this, &WindowManager::AddKeyDown);
    QObject::connect(m_vulkanWindow, &VulkanWindow::KeyUp, this, &WindowManager::AddKeyUp);

    QObject::connect(m_vulkanWindow, &VulkanWindow::MouseButtonDown, this, &WindowManager::AddMouseButtonDown);
    QObject::connect(m_vulkanWindow, &VulkanWindow::MouseButtonUp, this, &WindowManager::AddMouseButtonUp);

    QObject::connect(m_vulkanWindow, &VulkanWindow::MouseMoved, this, &WindowManager::CursorMoved);
    QObject::connect(m_vulkanWindow, &VulkanWindow::ManualMouseMove, this, &WindowManager::UpdateManualMousePosition);

    QObject::connect(m_vulkanWindow, &VulkanWindow::Resized, this, &WindowManager::OnResize);

    m_wrappingWidget = QWidget::createWindowContainer(m_vulkanWindow);
    m_wrappingWidget->resize(m_width, m_height);
    m_wrappingWidget->setFocusPolicy(Qt::StrongFocus);
    m_wrappingWidget->setFocus();
    m_wrappingWidget->setMouseTracking(true);
    m_wrappingWidget->grabKeyboard();

	addWidget(m_wrappingWidget);
}

void WindowManager::NewFrame()
{
    m_keysPressedThisFrame.clear();

	m_mouseDelta = glm::vec2(0.0f, 0.0f);
	m_scrollDelta = glm::vec2(0.0f, 0.0f);
}

void WindowManager::OnResize(QSize newSize, QSize oldSize) const
{
    if (m_scene != nullptr)
    {
        m_scene->OnResize(newSize, oldSize);
    }
}

bool WindowManager::KeyPressed(Qt::Key keyCode) const
{
    return m_pressedKeys.contains(keyCode);
}

bool WindowManager::KeyPressedThisFrame(Qt::Key keyCode) const
{
    return m_keysPressedThisFrame.contains(keyCode);
}

bool WindowManager::MouseButtonPressed(Qt::MouseButton mouseButton) const
{
    return m_pressedMouseButtons.contains(mouseButton);
}

bool WindowManager::MouseButtonPressedThisFrame(Qt::MouseButton mouseButton) const
{
    return m_pressedMouseButtonsThisFrame.contains(mouseButton);
}

void WindowManager::AddKeyDown(Qt::Key pressedKey)
{
    m_keysPressedThisFrame.insert(pressedKey);
    m_pressedKeys.insert(pressedKey);
}

void WindowManager::AddKeyUp(Qt::Key releasedKey)
{
    m_keysPressedThisFrame.erase(releasedKey);
    m_pressedKeys.erase(releasedKey);
}

void WindowManager::AddMouseButtonDown(Qt::MouseButton pressedButton)
{
    m_pressedMouseButtons.insert(pressedButton);
    m_pressedMouseButtonsThisFrame.insert(pressedButton);
}

void WindowManager::AddMouseButtonUp(Qt::MouseButton releasedButton)
{
    m_pressedMouseButtons.erase(releasedButton);
    m_pressedMouseButtonsThisFrame.erase(releasedButton);
}

void WindowManager::UpdateManualMousePosition(float xpos, float ypos)
{
    m_currentMousePosition = {xpos, ypos};
    m_currentMousePosition.y = -m_currentMousePosition.y;
    m_lastMousePosition = m_currentMousePosition;
}

void WindowManager::CursorMoved(float xpos, float ypos)
{
    m_lastMousePosition = m_currentMousePosition;

    m_currentMousePosition = {xpos, ypos};
	m_currentMousePosition.y = -m_currentMousePosition.y;

    m_mouseDelta = m_currentMousePosition - m_lastMousePosition;
}

void WindowManager::Shutdown() const
{
    m_vulkanWindow->Shutdown();
    QMetaObject::invokeMethod(m_parentProgram, "close", Qt::QueuedConnection);
}

void WindowManager::SetLockCursor(bool lockCursor) const
{
    m_vulkanWindow->SetLockCursor(lockCursor);
}

bool WindowManager::IsCursorLocked() const
{
    return m_vulkanWindow->IsCursorLocked();
}

void WindowManager::SetIsTrackingMouse(bool isTracking) const
{
    m_vulkanWindow->SetTrackingMouse(isTracking);
}

bool WindowManager::IsTrackingMouse() const
{
    return m_vulkanWindow->IsTrackingMouse();
}

void WindowManager::RemoveWidgetFromMenu(const std::string& widgetName)
{
    if (!m_buttons.contains(widgetName))
    {
        qDebug() << "Menu widget with name " << widgetName << " does not exist!";
        return;
    }

    QWidget* widgetToRemove = m_menuWidgets[widgetName];
    m_menuLayout->removeWidget(widgetToRemove);
    delete widgetToRemove;
    m_menuWidgets.erase(widgetName);
}

void WindowManager::AddWidgetToMenu(const std::string& widgetName, QWidget* newWidget)
{
    if (m_buttons.contains(widgetName))
    {
        qDebug() << "Menu widget with name " << widgetName << " already exists!";
    }

    m_menuLayout->addWidget(newWidget);
    m_menuWidgets[widgetName] = newWidget;
}

void WindowManager::AddButton(const std::string& title, const std::function<void()>& callback)
{
    if (m_buttons.contains(title))
    {
		qDebug() << "Button with title " << title << " already exists!";
    }

    QPushButton* newButton = new QPushButton(QString::fromStdString(title));
    connect(newButton, &QPushButton::clicked, [callback]() {
        callback();
    });

	m_menuLayout->addWidget(newButton);
    m_buttons[title] = newButton;
}

void WindowManager::RemoveButton(const std::string& title)
{
    if (!m_buttons.contains(title))
    {
        qDebug() << "Button with title " << title << " does not exist!";
        return;
    }

    QPushButton* buttonToRemove = m_buttons[title];
    m_menuLayout->removeWidget(buttonToRemove);
    delete buttonToRemove;
    m_buttons.erase(title);
}