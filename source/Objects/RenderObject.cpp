#include "RenderObject.h"

#include "Components/Transform.h"
#include "Management/Scene.h"
#include "Management/WindowManager.h"
#include "Management/Material.h"
#include "Management/Scene.h"

RenderObject::RenderObject()
{

}

bool RenderObject::RequiresInstanceBufferRegeneration()
{
	if (m_instanceBuffer == nullptr)
	{
		return true;
	}

	std::shared_ptr<Material> material = MaterialRegistry::Get()->GetMaterialByName(GetMaterialName());

	size_t instanceInfoSize = material->GetInstanceInfoSize();
	size_t instanceCount = GetInstanceCount();

	size_t requiredBufferSize = instanceInfoSize * instanceCount;

	return requiredBufferSize > m_instanceBuffer->GetSize();
}

void RenderObject::SetInstanceBuffer(const std::shared_ptr<GraphicsBuffer>& instanceBuffer)
{
	if (m_instanceBuffer != nullptr)
	{
		m_instanceBuffer->DestroyBuffer();
	}

	m_instanceBuffer = instanceBuffer;
}

size_t RenderObject::GetInstanceCount()
{
	std::shared_ptr<Material> material = MaterialRegistry::Get()->GetMaterialByName(GetMaterialName());

	return material->GetInstanceCount(this);
}

size_t RenderObject::GetInstanceDataSize()
{
	std::shared_ptr<Material> material = MaterialRegistry::Get()->GetMaterialByName(GetMaterialName());

	size_t instanceInfoSize = material->GetInstanceInfoSize();
	return instanceInfoSize;
}

std::shared_ptr<GraphicsBuffer> RenderObject::GetInstanceBuffer(const std::vector<std::filesystem::path>& textureFilePaths)
{
	if (m_instanceBuffer == nullptr)
	{
		return nullptr;
	}

	std::shared_ptr<Material> material = MaterialRegistry::Get()->GetMaterialByName(GetMaterialName());

	size_t instanceInfoSize = material->GetInstanceInfoSize();
	size_t instanceCount = GetInstanceCount();
	std::vector<std::byte> instanceData(instanceInfoSize * instanceCount);

	material->GetInstanceInfo(this, textureFilePaths, instanceData);

	m_instanceBuffer->LoadData(instanceData.data(), instanceInfoSize * instanceCount);

	return m_instanceBuffer;
}

void RenderObject::GetVertexBuffer(std::vector<size_t>& outBufferSizes, std::vector<std::shared_ptr<GraphicsBuffer>>& outBuffers)
{
	std::shared_ptr<IMeshRenderer> meshComponent = GetComponent<IMeshRenderer>();
	if (meshComponent != nullptr)
	{
		outBufferSizes.push_back(meshComponent->GetVertexCount());
		outBuffers.push_back(meshComponent->GetVertexBuffer());
	}
}

void RenderObject::GetIndexBuffer(std::vector<size_t>& outBufferSizes, std::vector<std::shared_ptr<GraphicsBuffer>>& outBuffers)
{
	std::shared_ptr<IMeshRenderer> meshComponent = GetComponent<IMeshRenderer>();

	if (meshComponent != nullptr && meshComponent->IsIndexed())
	{
		outBufferSizes.push_back(meshComponent->GetIndexCount());
		outBuffers.push_back(meshComponent->GetIndexBuffer());
	} else if (meshComponent != nullptr && !meshComponent->IsIndexed())
	{
		outBufferSizes.push_back(0);
		outBuffers.push_back(nullptr);
	}
}

bool RenderObject::IsIndexed()
{
	std::shared_ptr<IMeshRenderer> meshComponent = GetComponent<IMeshRenderer>();

	if (meshComponent != nullptr)
	{
		return meshComponent->IsIndexed();
	}

	return false;
}

void RenderObject::RequestInstanceBufferUpdate()
{
	if (GetSceneManager() == nullptr)
	{
		return;
	}

	GetSceneManager()->RequestInstanceBufferUpdate(m_objectHandle);
}