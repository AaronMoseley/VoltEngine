#include "RenderObject.h"

#include "Components/Transform.h"
#include "Management/Scene.h"
#include "Management/WindowManager.h"
#include "Management/Material.h"

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
	std::shared_ptr<MeshRenderer> meshComponent = GetComponent<MeshRenderer>();
	if (meshComponent != nullptr)
	{
		outBufferSizes.push_back(meshComponent->GetVertexBufferSize());
		outBuffers.push_back(meshComponent->GetVertexBuffer());
	}

	std::shared_ptr<UIImage> imageComponent = GetComponent<UIImage>();
	if (imageComponent != nullptr)
	{
		outBufferSizes.push_back(imageComponent->GetVertexBufferSize());
		outBuffers.push_back(imageComponent->GetVertexBuffer());
	}

	std::shared_ptr<Text> textComponent = GetComponent<Text>();
	if (textComponent != nullptr)
	{
		outBufferSizes.push_back(textComponent->GetVertexBufferSize());
		outBuffers.push_back(textComponent->GetVertexBuffer());
	}
}

void RenderObject::GetIndexBuffer(std::vector<size_t>& outBufferSizes, std::vector<std::shared_ptr<GraphicsBuffer>>& outBuffers)
{
	std::shared_ptr<MeshRenderer> meshComponent = GetComponent<MeshRenderer>();

	if (meshComponent != nullptr && meshComponent->IsIndexed())
	{
		outBufferSizes.push_back(meshComponent->GetIndexBufferSize());
		outBuffers.push_back(meshComponent->GetIndexBuffer());
	} else if (meshComponent != nullptr && !meshComponent->IsIndexed())
	{
		outBufferSizes.push_back(0);
		outBuffers.push_back(nullptr);
	}

	std::shared_ptr<UIImage> imageComponent = GetComponent<UIImage>();
	if (imageComponent != nullptr)
	{
		outBufferSizes.push_back(imageComponent->GetIndexBufferSize());
		outBuffers.push_back(imageComponent->GetIndexBuffer());
	}

	std::shared_ptr<Text> textComponent = GetComponent<Text>();
	if (textComponent != nullptr)
	{
		outBufferSizes.push_back(textComponent->GetIndexBufferSize());
		outBuffers.push_back(textComponent->GetIndexBuffer());
	}
}

bool RenderObject::IsIndexed()
{
	std::shared_ptr<MeshRenderer> meshComponent = GetComponent<MeshRenderer>();

	if (meshComponent != nullptr)
	{
		return meshComponent->IsIndexed();
	}

	return false;
}