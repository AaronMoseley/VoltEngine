#pragma once

#include "Objects/ObjectComponent.h"
#include "Vulkan Interface/VulkanCommonFunctions.h"
#include "Components/Transform.h"
#include "Objects/RenderObject.h"

#include "glm.hpp"

class LightSource : public ObjectComponent {
public:
	LightSource() {};

	glm::vec3 GetColor() const { return m_color; }
	void SetColor(glm::vec3 color) { m_color = glm::vec4(color, 1.0f); }

	glm::vec3 GetSpecularColor() const { return m_specularColor; }
	void SetSpecularColor(glm::vec3 color) { m_specularColor = glm::vec4(color, 1.0f); }

	float GetMaxDistance() const { return m_maxDistance; }
	void SetMaxDistance(float distance) { m_maxDistance = distance; }

	VulkanCommonFunctions::LightInfo GetLightInfo() const
	{
		VulkanCommonFunctions::LightInfo lightInfo;
		lightInfo.m_lightColor = m_color;
		lightInfo.m_lightAmbient = m_color * 0.2f;
		lightInfo.m_lightDiffuse = m_color * 0.5f;
		lightInfo.m_lightSpecular = m_specularColor;
		lightInfo.m_lightPosition = glm::vec4(GetOwner()->GetComponent<Transform>()->GetPosition(), 1.0);
		lightInfo.m_maxLightDistance.x = m_maxDistance;
		return lightInfo;
	}

private:
	glm::vec4 m_color = glm::vec4(1.0f);
	glm::vec4 m_specularColor = glm::vec4(1.0f);
	float m_maxDistance = 40.0f;
};