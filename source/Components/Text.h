#pragma once

#include "Components/UIMeshRenderer.h"
#include "Vulkan Interface/VulkanCommonFunctions.h"
#include "Text Rendering/Font.h"

class Text : public UIMeshRenderer {
public:
	Text();

	void SetTextString(const std::string& newText) { m_textString = newText; m_textDataDirty = true; }
	[[nodiscard]] std::string GetTextString() const { return m_textString; }

	void SetFontName(const std::string& fontName) { m_fontName = fontName; m_textDataDirty = true; }
	[[nodiscard]] std::string GetFontName() const { return m_fontName; }

	void SetFontSize(const float fontSize) { m_fontSize = fontSize; m_textDataDirty = true; }
	[[nodiscard]] float GetFontSize() const { return m_fontSize; }

	const std::vector<VulkanCommonFunctions::UIVertex>& GetVertices() override { return m_squareVertices; };
	const std::vector<uint16_t>& GetIndices() override { return m_squareIndices; };

	void UpdateInstanceBuffer(const std::pair<size_t, size_t>& screenSize, const std::shared_ptr<Font>& currentFont, size_t textureIndex, GraphicsBuffer::BufferCreateInfo bufferCreateInfo);
	std::shared_ptr<GraphicsBuffer> GetInstanceBuffer() { return m_instanceBuffer; }

	static float GetPixelToScreen(const std::pair<size_t, size_t>& screenSize)
	{
		return 1.0f / static_cast<float>(screenSize.second);
	}

	void SetTextDataDirty(const bool newValue) {m_textDataDirty = newValue; }

private:
	using UIMeshRenderer::SetVertices;
	using UIMeshRenderer::SetIndices;

	void GetCharacterInstanceInfo(std::pair<size_t, size_t> screenSize, const std::shared_ptr<Font>& currentFont, std::vector<VulkanCommonFunctions::UIInstanceInfo>& outCharacterInfo);

	std::vector<VulkanCommonFunctions::UIVertex> m_squareVertices = {
		//positions              //texture coords
		{ {-1.0f,  1.0f, 0.0f, 0.0f},  {-0.5f, -0.5f, 0.0f, 0.0f} }, //top left
		{ { 1.0f,  1.0f, 0.0f, 0.0f},  {0.5f, -0.5f, 0.0f, 0.0f} }, //top right
		{ { 1.0f, -1.0f, 0.0f, 0.0f},  {0.5f, 0.5f, 0.0f, 0.0f} }, //bottom right
		{ {-1.0f, -1.0f, 0.0f, 0.0f},  {-0.5f, 0.5f, 0.0f, 0.0f} }  //bottom left
	};

	std::vector<uint16_t> m_squareIndices = { 0, 1, 2, 2, 3, 0 };

	std::shared_ptr<GraphicsBuffer> m_instanceBuffer = nullptr;

	std::string m_textString;
	std::string m_fontName;

	glm::vec4 m_color = glm::vec4(1.0f);

	float m_fontSize = 20.0f;

	float m_additionalCharacterSpacing = 0.015f;
	float m_additionalLineSpacing = 0.0f;

	bool m_textDataDirty = false;
};