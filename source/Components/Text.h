#pragma once

#include "Components/GenericUIObjectMeshRenderer.h"
#include "Vulkan Interface/VulkanCommonFunctions.h"
#include "Text Rendering/Font.h"

class Text : public GenericUIObjectMeshRenderer {
public:
	enum AnchorPosition
	{
		Center = 0,
		TopLeft,
		TopRight,
		BottomLeft,
		BottomRight,
		LeftMiddle,
		RightMiddle,
		TopCenter,
		BottomCenter
	};

	Text() : GenericUIObjectMeshRenderer(kSquareVertices, kSquareIndices)
	{

	}

	void SetTextString(const std::string& newText) { m_textString = newText; m_textDataDirty = true; }
	std::string GetTextString() const { return m_textString; }

	void SetFontName(const std::string& fontName) { m_fontName = fontName; m_textDataDirty = true; }
	std::string GetFontName() const { return m_fontName; }

	void SetFontSize(const float fontSize) { m_fontSize = fontSize; m_textDataDirty = true; }
	float GetFontSize() const { return m_fontSize; }

	void GetInstanceInfo(const std::pair<size_t, size_t>& screenSize, const std::shared_ptr<Font>& currentFont, size_t textureIndex, std::vector<std::byte>& outData);

	static float GetPixelToScreen(const std::pair<size_t, size_t>& screenSize)
	{
		return 1.0f / static_cast<float>(screenSize.second);
	}

	void SetTextDataDirty(const bool newValue) {m_textDataDirty = newValue; }

	void SetAnchorPoint(AnchorPosition newAnchorPosition)
	{
		m_anchorPoint = newAnchorPosition;
		SetTextDataDirty(true);
	}

private:
	using MeshRenderer::SetVertices;
	using MeshRenderer::SetIndices;

	void GetCharacterInstanceInfo(
		std::pair<size_t, size_t> screenSize,
		const std::shared_ptr<Font>& currentFont,
		std::vector<GenericUIObjectMaterial::UIInstanceInfo>& outCharacterInfo) ;

	inline static const std::vector<GenericUIObjectMaterial::UIVertex> kSquareVertices = {
		//positions              //texture coords
		{ {-1.0f,  1.0f, 0.0f, 0.0f},  {-0.5f, -0.5f, 0.0f, 0.0f} }, //top left
		{ { 1.0f,  1.0f, 0.0f, 0.0f},  {0.5f, -0.5f, 0.0f, 0.0f} }, //top right
		{ { 1.0f, -1.0f, 0.0f, 0.0f},  {0.5f, 0.5f, 0.0f, 0.0f} }, //bottom right
		{ {-1.0f, -1.0f, 0.0f, 0.0f},  {-0.5f, 0.5f, 0.0f, 0.0f} }  //bottom left
	};

	inline static const std::vector<uint32_t> kSquareIndices = { 0, 1, 2, 2, 3, 0 };

	std::map<AnchorPosition, glm::vec2> kAnchorPositionScreenCoordinates = {
		{AnchorPosition::Center, {0.0f, 0.0f}},
		{AnchorPosition::TopLeft, {-1.0f, 1.0f}},
		{AnchorPosition::TopRight, {1.0f, 1.0f}},
		{AnchorPosition::BottomLeft, {-1.0f, -1.0f}},
		{AnchorPosition::BottomRight, {1.0f, -1.0f}},
		{AnchorPosition::LeftMiddle, {-1.0f, 0.0f}},
		{AnchorPosition::RightMiddle, {1.0f, 0.0f}},
		{AnchorPosition::TopCenter, {0.0f, 1.0f}},
		{AnchorPosition::BottomCenter, {0.0f, -1.0f}}
	};

	AnchorPosition m_anchorPoint = AnchorPosition::Center;

	std::string m_textString;
	std::string m_fontName;

	glm::vec4 m_color = glm::vec4(1.0f);

	float m_fontSize = 20.0f;

	float m_additionalCharacterSpacing = 0.015f;
	float m_additionalLineSpacing = 0.0f;

	bool m_textDataDirty = false;
};