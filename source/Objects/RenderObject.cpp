#include "RenderObject.h"
#include "Management/Scene.h"
#include "Management/WindowManager.h"

RenderObject::RenderObject()
{

}

VulkanCommonFunctions::InstanceInfo RenderObject::GetInstanceInfo(const std::vector<std::filesystem::path>& textureFilePaths)
{
	VulkanCommonFunctions::InstanceInfo result {};

	std::shared_ptr<Transform> transform = GetComponent<Transform>();

	if (transform == nullptr)
	{
		return result;
	}

	std::shared_ptr<MeshRenderer> meshRenderer = GetComponent<MeshRenderer>();

	if (meshRenderer == nullptr)
	{
		return result;
	}

	result.modelMatrix = glm::mat4(1.0f);
	result.modelMatrix = glm::translate(result.modelMatrix, transform->GetWorldPosition());

	result.modelMatrix = glm::scale(result.modelMatrix, transform->GetWorldScale());

	glm::vec3 rotation = transform->GetWorldRotation();

	result.modelMatrix = glm::rotate(result.modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	result.modelMatrix = glm::rotate(result.modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	result.modelMatrix = glm::rotate(result.modelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	//need to transpose the matrix because hlsl expects column major matrices
	result.modelMatrix = glm::transpose(result.modelMatrix);

	result.modelMatrixInverse = glm::inverse(result.modelMatrix);

	result.scale = glm::vec4(transform->GetWorldScale(), 1.0f);

	result.ambient = glm::vec4(meshRenderer->GetColor(), 1.0f);
	result.diffuse = glm::vec4(meshRenderer->GetColor(), 1.0f);
	result.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	result.opacityAndShininess.y = std::pow(2.0f, meshRenderer->GetShininess());

	result.opacityAndShininess.x = meshRenderer->GetOpacity();

	result.displayProperties.x = (meshRenderer->GetLit()) ? 1 : 0;

	result.displayProperties.y = (meshRenderer->GetTextured()) ? 1 : 0;

	result.displayProperties.w = (meshRenderer->IsBillboarded()) ? 1 : 0;

	auto iterator = std::find(textureFilePaths.begin(), textureFilePaths.end(), meshRenderer->GetTexturePath());

	result.displayProperties.z = std::distance(textureFilePaths.begin(), iterator);
	if (result.displayProperties.z > textureFilePaths.size())
	{
		result.displayProperties.z = 0;
	}

	return result;
}

VulkanCommonFunctions::UIInstanceInfo RenderObject::GetUIInstanceInfo(const std::vector<std::filesystem::path>& textureFilePaths)
{
	VulkanCommonFunctions::UIInstanceInfo result {};
	std::shared_ptr<Transform> transform = GetComponent<Transform>();
	if (transform == nullptr)
	{
		return result;
	}
	std::shared_ptr<UIImage> imageComponent = GetComponent<UIImage>();
	if (imageComponent == nullptr)
	{
		return result;
	}
	result.objectPosition = glm::vec4(transform->GetWorldPosition(), 1.0f);
	result.scale = glm::vec4(transform->GetWorldScale(), 1.0f);
	result.color = glm::vec4(imageComponent->GetColor(), imageComponent->GetOpacity());
	result.displayProperties.x = (imageComponent->GetTextured()) ? 1 : 0;
	//whether the instance is a character in text
	result.displayProperties.z = 0;

	auto iterator = std::find(textureFilePaths.begin(), textureFilePaths.end(), imageComponent->GetTexturePath());

	result.displayProperties.y = std::distance(textureFilePaths.begin(), iterator);
	if (result.displayProperties.y > textureFilePaths.size())
	{
		result.displayProperties.y = 0;
	}

	return result;
}

std::shared_ptr<GraphicsBuffer> RenderObject::GetInstanceBuffer(const std::vector<std::filesystem::path>& textureFilePaths)
{
	if (m_instanceBuffer == nullptr)
	{
		return nullptr;
	}

	VulkanCommonFunctions::InstanceInfo info = GetInstanceInfo(textureFilePaths);

	std::array<VulkanCommonFunctions::InstanceInfo, 1> infoArray = { info };

	m_instanceBuffer->LoadData(infoArray.data(), sizeof(VulkanCommonFunctions::InstanceInfo));

	return m_instanceBuffer;
}

std::shared_ptr<GraphicsBuffer> RenderObject::GetUIInstanceBuffer(const std::vector<std::filesystem::path>& textureFilePaths)
{
	if (m_instanceBuffer == nullptr)
	{
		return nullptr;
	}

	VulkanCommonFunctions::UIInstanceInfo info = GetUIInstanceInfo(textureFilePaths);
	std::array<VulkanCommonFunctions::UIInstanceInfo, 1> infoArray = { info };

	m_instanceBuffer->LoadData(infoArray.data(), sizeof(VulkanCommonFunctions::UIInstanceInfo));

	return m_instanceBuffer;
}