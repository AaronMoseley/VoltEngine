#pragma once

#include "Objects/ObjectComponent.h"
#include "Vulkan Interface/TextureImage.h"
#include "Vulkan Interface/VulkanCommonFunctions.h"
#include "Components/GenericUIObjectMeshRenderer.h"

class UIImage : public GenericUIObjectMeshRenderer {
public:
	UIImage();
	UIImage(const std::string& imageFilePath);

	void SetOpacity(float opacity) { m_opacity = opacity; }
	float GetOpacity() const { return m_opacity; }

	void TextureSetCallback() override;

	glm::vec3 GetColor() const { return m_color; }
	void SetColor(glm::vec3 color) { m_color = color; }

private:
	using GenericUIObjectMeshRenderer::SetVertices;
	using GenericUIObjectMeshRenderer::SetIndices;

	inline static const std::vector<GenericUIObjectMaterial::UIVertex> kSquareVertices = {
		//positions              //texture coords
		{ {-1.0f,  1.0f, 0.0f, 0.0f},  {0.0f, 0.0f, 0.0f, 0.0f} }, //top left
		{ { 1.0f,  1.0f, 0.0f, 0.0f},  {1.0f, 0.0f, 0.0f, 0.0f} }, //top right
		{ { 1.0f, -1.0f, 0.0f, 0.0f},  {1.0f, 1.0f, 0.0f, 0.0f} }, //bottom right
		{ {-1.0f, -1.0f, 0.0f, 0.0f},  {0.0f, 1.0f, 0.0f, 0.0f} }  //bottom left
	};

	inline static const std::vector<uint32_t> kSquareIndices = { 0, 1, 2, 2, 3, 0 };

	float m_opacity = 1.0f;

	int m_imageWidth = 0;
	int m_imageHeight = 0;

	glm::vec3 m_color = glm::vec3(1.0f);
};