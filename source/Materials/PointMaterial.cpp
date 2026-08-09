#include "PointMaterial.h"

#include "Components/PointMeshRenderer.h"
#include "Components/Transform.h"

MaterialAutoRegister<PointMaterial> PointMaterial::s_register;

void PointMaterial::GetInstanceInfo(RenderObject* object, const std::vector<std::filesystem::path>& textureFilePaths, std::vector<std::byte>& outData)
{
	PointInstanceInfo instanceInfo = {};

	std::shared_ptr<Transform> transform = object->GetComponent<Transform>();
	std::shared_ptr<PointMeshRenderer> meshRenderer = object->GetComponent<PointMeshRenderer>();

	if (meshRenderer == nullptr)
	{
		return;
	}

	instanceInfo.m_modelMatrix = glm::mat4(1.0f);

	instanceInfo.m_modelMatrix = glm::translate(instanceInfo.m_modelMatrix, transform->GetPosition());

	glm::quat rotation = transform->GetWorldRotationQuaternion();
	instanceInfo.m_modelMatrix *= glm::mat4_cast(rotation);

	instanceInfo.m_modelMatrix = glm::scale(instanceInfo.m_modelMatrix, transform->GetScale());

	//need to transpose the matrix because hlsl expects column major matrices
	instanceInfo.m_modelMatrix = glm::transpose(instanceInfo.m_modelMatrix);

	instanceInfo.m_color = glm::vec4(meshRenderer->GetColor(), 1.0f);

	memcpy(outData.data(), &instanceInfo, sizeof(PointInstanceInfo));
}

void PointMaterial::CreateVertexFormat()
{
	m_vertexFormat = std::make_shared<VertexFormat>();

	m_vertexFormat->AddNewBinding(0, VK_VERTEX_INPUT_RATE_VERTEX);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);

	m_vertexFormat->AddNewBinding(1, VK_VERTEX_INPUT_RATE_INSTANCE);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);

	m_vertexFormat->Finalize();
}