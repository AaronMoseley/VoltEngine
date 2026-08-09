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

class Scene {
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

	void UpdateTexture(const std::filesystem::path& newTexturePath) const;

	const std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>>& GetObjects() { return m_objects; };
	const std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>>& GetUIObjects() { return m_uiObjects; };
	const std::map<std::string, std::map<std::string, std::set<VulkanCommonFunctions::ObjectHandle>>>& GetMaterialAndNameToObjectMap() { return m_materialAndNameToObjectHandle; }
	const std::map<VulkanCommonFunctions::ObjectHandle, size_t>& GetObjectsToUpdate() { return m_objectsToUpdateInstanceInfo; }

	VulkanCommonFunctions::ObjectHandle GetObjectByTag(const std::string& tag);
	std::shared_ptr<RenderObject> GetRenderObject(VulkanCommonFunctions::ObjectHandle handle);
	std::shared_ptr<RenderObject> GetUIRenderObject(VulkanCommonFunctions::ObjectHandle handle);

	size_t GetObjectCount() const { return m_objects.size(); };

	void RegisterUpdateCallback(const std::function<void(float)>& callback)
	{
		m_updateCallbacks.push_back(callback);
	}

	void OnResize(QSize newSize, QSize oldSize);

	bool MeshAlreadyAdded(const std::string& meshName) const;

	void RequestInstanceBufferUpdate(VulkanCommonFunctions::ObjectHandle renderObjectHandle)
	{
		m_objectsToUpdateInstanceInfo.insert( {renderObjectHandle, VulkanCommonFunctions::MAX_FRAMES_IN_FLIGHT} );
	}
	void DecrementFrameCountersForObjects()
	{
		for (auto it = m_objectsToUpdateInstanceInfo.begin(); it != m_objectsToUpdateInstanceInfo.end();)
		{
			it->second--;

			if (it->second == 0)
			{
				it = m_objectsToUpdateInstanceInfo.erase(it);
			} else
			{
				it++;
			}
		}
	}

private:
	void UpdateMeshData(const std::shared_ptr<RenderObject>& currentObject);

	std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>> m_objects = {};
	std::map<std::string, std::map<std::string, std::set<VulkanCommonFunctions::ObjectHandle>>> m_materialAndNameToObjectHandle;

	std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>> m_uiObjects = {};

	WindowManager* m_windowManager;

	VulkanCommonFunctions::ObjectHandle m_currentObjectHandle = 0;
	VulkanCommonFunctions::ObjectHandle m_currentUIObjectHandle = 0;

	std::vector<std::function<void(float)>> m_updateCallbacks;

	std::vector<std::shared_ptr<GraphicsBuffer>> m_buffersToDestroy;

	std::map<VulkanCommonFunctions::ObjectHandle, size_t> m_objectsToUpdateInstanceInfo;

	double m_deltaTime = 0.0f;	// Time between current frame and last frame
	double m_lastFrameTime = -1.0f; // Time of last frame
};