#pragma once

#include "Vulkan Interface/VulkanInterface.h"
#include "Vulkan Interface/VulkanCommonFunctions.h"
#include "Components/UIImage.h"
#include "Text Rendering/FontManager.h"

#include <memory>
#include <vector>
#include <functional>
#include <chrono>

class RenderObject;

class alignas(16) Scene {
public:
	Scene(WindowManager* windowManager, const std::shared_ptr<VulkanInterface>& vulkanInterface);

	void Update();

	void Cleanup();

	std::shared_ptr<Font> AddFont(const std::string& atlasFilePath, const std::string& descriptionFilePath) const;

	VulkanCommonFunctions::ObjectHandle AddObject(const std::shared_ptr <RenderObject>& newObject);
	bool RemoveObject(VulkanCommonFunctions::ObjectHandle objectToRemove);

	VulkanCommonFunctions::ObjectHandle AddUIObject(const std::shared_ptr <RenderObject>& newObject);
	bool RemoveUIObject(VulkanCommonFunctions::ObjectHandle objectToRemove);

	void FinalizeMesh(const std::shared_ptr<RenderObject>& updatedObject);
	void GenerateInstanceBuffer(const std::shared_ptr<RenderObject>& updatedObject) const;

	void FinalizeUIMesh(const std::shared_ptr<RenderObject>& updatedObject);

	void UpdateTexture(const std::filesystem::path& newTexturePath) const;

	std::shared_ptr<FontManager> GetFontManager() { return m_fontManager; }

	std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>> GetObjects() { return m_objects; };
	std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>> GetUIObjects() { return m_uiObjects; };
	std::map<std::string, std::set<VulkanCommonFunctions::ObjectHandle>> GetMeshNameToObjectMap() { return m_meshNameToObjectMap; }

	VulkanCommonFunctions::ObjectHandle GetObjectByTag(const std::string& tag);
	std::shared_ptr<RenderObject> GetRenderObject(VulkanCommonFunctions::ObjectHandle handle);
	std::shared_ptr<RenderObject> GetUIRenderObject(VulkanCommonFunctions::ObjectHandle handle);

	size_t GetObjectCount() const { return m_objects.size(); };

	void RegisterUpdateCallback(const std::function<void(float)>& callback)
	{
		m_updateCallbacks.push_back(callback);
	}

	void OnResize(QSize newSize, QSize oldSize);

private:
	void UpdateMeshData(const std::shared_ptr<RenderObject>& currentObject);
	void UpdateUIData(const std::shared_ptr<RenderObject>& currentObject);

	std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>> m_objects = {};
	std::map<std::string, std::set<VulkanCommonFunctions::ObjectHandle>> m_meshNameToObjectMap;

	std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>> m_uiObjects = {};

	WindowManager* m_windowManager;
	std::shared_ptr<VulkanInterface> m_vulkanInterface;
	std::shared_ptr<FontManager> m_fontManager;

	VulkanCommonFunctions::ObjectHandle m_currentObjectHandle = 0;
	VulkanCommonFunctions::ObjectHandle m_currentUIObjectHandle = 0;

	std::vector<std::function<void(float)>> m_updateCallbacks;

	std::vector<std::shared_ptr<GraphicsBuffer>> m_buffersToDestroy;

	double m_deltaTime = 0.0f;	// Time between current frame and last frame
	double m_lastFrameTime = -1.0f; // Time of last frame
};