#include "GenericObjectMaterial.h"
#include "Components/Transform.h"
#include "Components/GenericObjectMeshRenderer.h"

MaterialAutoRegister<GenericObjectMaterial> GenericObjectMaterial::s_register;

void GenericObjectMaterial::GetInstanceInfo(RenderObject* object, const std::vector<std::filesystem::path>& textureFilePaths, std::vector<std::byte>& outData)
{
	InstanceInfo result = {};

	std::shared_ptr<Transform> transform = object->GetComponent<Transform>();

	if (transform == nullptr)
	{
		return;
	}

	std::shared_ptr<GenericObjectMeshRenderer> meshRenderer = object->GetComponent<GenericObjectMeshRenderer>();

	if (meshRenderer == nullptr)
	{
		return;
	}

	result.m_modelMatrix = glm::mat4(1.0f);

	result.m_modelMatrix = glm::translate(result.m_modelMatrix, transform->GetWorldPosition());

	glm::quat rotation = transform->GetWorldRotationQuaternion();
	result.m_modelMatrix *= glm::mat4_cast(rotation);

	result.m_modelMatrix = glm::scale(result.m_modelMatrix, transform->GetWorldScale());

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

	memcpy(outData.data(), &result, sizeof(InstanceInfo));
}

void GenericObjectMaterial::CreateVertexFormat()
{
	m_vertexFormat = std::make_shared<VertexFormat>();

	m_vertexFormat->AddNewBinding(0, VK_VERTEX_INPUT_RATE_VERTEX);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);

	m_vertexFormat->AddNewBinding(1, VK_VERTEX_INPUT_RATE_INSTANCE);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_UINT);

	m_vertexFormat->Finalize();
}