#include "GenericObjectMaterial.h"
#include "Components/Transform.h"

void GenericObjectMaterial::GetInstanceInfo(RenderObject* object, const std::vector<std::filesystem::path>& textureFilePaths, std::vector<std::byte>& outData)
{
	VulkanCommonFunctions::InstanceInfo result = {};

	std::shared_ptr<Transform> transform = object->GetComponent<Transform>();

	if (transform == nullptr)
	{
		return;
	}

	std::shared_ptr<MeshRenderer> meshRenderer = object->GetComponent<MeshRenderer>();

	if (meshRenderer == nullptr)
	{
		return;
	}

	result.m_modelMatrix = glm::mat4(1.0f);
	result.m_modelMatrix = glm::translate(result.m_modelMatrix, transform->GetWorldPosition());

	result.m_modelMatrix = glm::scale(result.m_modelMatrix, transform->GetWorldScale());

	glm::vec3 rotation = transform->GetWorldRotation();

	result.m_modelMatrix = glm::rotate(result.m_modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	result.m_modelMatrix = glm::rotate(result.m_modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	result.m_modelMatrix = glm::rotate(result.m_modelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	//need to transpose the matrix because hlsl expects column major matrices
	result.m_modelMatrix = glm::transpose(result.m_modelMatrix);

	result.m_modelMatrixInverse = glm::inverse(result.m_modelMatrix);

	result.m_scale = glm::vec4(transform->GetWorldScale(), 1.0f);

	result.m_ambient = glm::vec4(meshRenderer->GetColor(), 1.0f);
	result.m_diffuse = glm::vec4(meshRenderer->GetColor(), 1.0f);
	result.m_specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	result.m_opacityAndShininess.y = std::pow(2.0f, meshRenderer->GetShininess());

	result.m_opacityAndShininess.x = meshRenderer->GetOpacity();

	result.m_displayProperties.x = (meshRenderer->GetLit()) ? 1 : 0;

	result.m_displayProperties.y = (meshRenderer->GetTextured()) ? 1 : 0;

	result.m_displayProperties.w = (meshRenderer->IsBillboarded()) ? 1 : 0;

	auto iterator = std::find(textureFilePaths.begin(), textureFilePaths.end(), meshRenderer->GetTexturePath());

	result.m_displayProperties.z = std::distance(textureFilePaths.begin(), iterator);
	if (result.m_displayProperties.z > textureFilePaths.size())
	{
		result.m_displayProperties.z = 0;
	}

	memcpy(outData.data(), &result, sizeof(VulkanCommonFunctions::InstanceInfo));
}