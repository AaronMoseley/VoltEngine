#pragma once

#include "Objects/ObjectComponent.h"
#include "Vulkan Interface/TextureImage.h"
#include "Vulkan Interface/VulkanCommonFunctions.h"
#include "Components/UIMeshRenderer.h"

class UIImage : public UIMeshRenderer {
public:
	UIImage();
	UIImage(const std::string& imageFilePath);

	void SetOpacity(float opacity) { m_opacity = opacity; }
	float GetOpacity() const { return m_opacity; }

	bool GetTextured() const { return m_textured; }
	std::string GetTexturePath() { return m_texturePath; }

	void SetTexture(const std::string& texturePath) {
		m_texturePath = texturePath;
		m_textured = true;
		m_textureDataDirty = true;
		CalculateMeshInfo();
	};
	void SetTextured(bool textured) { m_textured = textured; }

	void CalculateMeshInfo();

	glm::vec3 GetColor() const { return m_color; }
	void SetColor(glm::vec3 color) { m_color = color; }

	void SetTextureDataDirty(bool dirty) { m_textureDataDirty = dirty; }
	bool IsTextureDataDirty() const { return m_textureDataDirty; }

	const std::vector<VulkanCommonFunctions::UIVertex>& GetVertices() override { return kSquareVertices; };
	const std::vector<uint16_t>& GetIndices() override { return kSquareIndices; };

private:
	using UIMeshRenderer::SetVertices;
	using UIMeshRenderer::SetIndices;

	alignas(16) std::vector<VulkanCommonFunctions::UIVertex> kSquareVertices = {
		//positions              //texture coords
		{ {-1.0f,  1.0f, 0.0f, 0.0f},  {0.0f, 0.0f, 0.0f, 0.0f} }, //top left
		{ { 1.0f,  1.0f, 0.0f, 0.0f},  {1.0f, 0.0f, 0.0f, 0.0f} }, //top right
		{ { 1.0f, -1.0f, 0.0f, 0.0f},  {1.0f, 1.0f, 0.0f, 0.0f} }, //bottom right
		{ {-1.0f, -1.0f, 0.0f, 0.0f},  {0.0f, 1.0f, 0.0f, 0.0f} }  //bottom left
	};

	alignas(16) std::vector<uint16_t> kSquareIndices = { 0, 1, 2, 2, 3, 0 };

	float m_opacity = 1.0f;

	bool m_textured = false;
	std::string m_texturePath = "";

	bool m_textureDataDirty = false;

	int m_imageWidth = 0;
	int m_imageHeight = 0;

	alignas(16) glm::vec3 m_color = glm::vec3(1.0f);
};