#pragma once

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "Vulkan Interface/VulkanCommonFunctions.h"
#include "Objects/ObjectComponent.h"
#include "Vulkan Interface/GraphicsBuffer.h"

#include <filesystem>
#include <vector>
#include <string>
#include <memory>

class Scene;
class WindowManager;

class RenderObject {
public:
	RenderObject();

	template <typename T>
	std::shared_ptr<T> AddComponent()
	{
		std::shared_ptr<T> newComponent = std::make_shared<T>();
		m_components.push_back(newComponent);

		newComponent->SetOwner(this);

		return newComponent;
	}

	template <typename T>
	std::shared_ptr<T> GetComponent()
	{
		for (size_t i = 0; i < m_components.size(); i++)
		{
			std::shared_ptr<T> component = std::dynamic_pointer_cast<T>(m_components[i]);

			if (component != nullptr)
			{
				return component;
			}
		}

		return nullptr;
	}

	std::vector<std::shared_ptr<ObjectComponent>> GetAllComponents() { return m_components; }

	std::shared_ptr<GraphicsBuffer> GetInstanceBuffer(const std::vector<std::filesystem::path>& textureFilePaths);
	void SetInstanceBuffer(const std::shared_ptr<GraphicsBuffer>& instanceBuffer) { m_instanceBuffer = instanceBuffer; }
	size_t GetInstanceCount();

	void SetSceneManager(Scene* sceneManager) { m_sceneManager = sceneManager; }
	Scene* GetSceneManager() const { return m_sceneManager; }

	void SetWindowManager(WindowManager* windowManager) { m_windowManager = windowManager; }
	WindowManager* GetWindowManager() const { return m_windowManager; }

	bool IsInitialized() const { return m_initialized; }

	void SetTag(const std::string& tag) { m_tag = tag; }
	std::string GetTag() { return m_tag; }

	std::string GetMaterialName() { return m_materialName; }
	//Material name can only be set prior to adding to the scene, afterwards it has no effect
	void SetMaterialName(const std::string& materialName) { m_materialName = materialName; }

	void GetVertexBuffer(std::vector<size_t>& outBufferSizes, std::vector<std::shared_ptr<GraphicsBuffer>>& outBuffers);
	void GetIndexBuffer(std::vector<size_t>& outBufferSizes, std::vector<std::shared_ptr<GraphicsBuffer>>& outBuffers);
	bool IsIndexed();

protected:
	std::vector<std::shared_ptr<ObjectComponent>> m_components;
	WindowManager* m_windowManager = nullptr;
	std::shared_ptr<GraphicsBuffer> m_instanceBuffer = nullptr;

	Scene* m_sceneManager = nullptr;

	std::string m_tag = "";
	std::string m_materialName = "";

	bool m_initialized = false;
};