#include "UIImage.h"
#include "stb_image.h"
#include "Objects/RenderObject.h"

UIImage::UIImage() : GenericUIObjectMeshRenderer(kSquareVertices, kSquareIndices)
{

}

UIImage::UIImage(const std::string& imageFilePath) : GenericUIObjectMeshRenderer(kSquareVertices, kSquareIndices)
{
	SetTexture(imageFilePath);
}

void UIImage::TextureSetCallback()
{
	int channels;
	if (!stbi_info(m_texturePath.c_str(), &m_imageWidth, &m_imageHeight, &channels))
	{
		throw std::runtime_error("Failed to load image info: " + m_texturePath);
	}

	if (m_imageWidth > m_imageHeight)
	{
		float ratio = static_cast<float>(m_imageHeight) / static_cast<float>(m_imageWidth);
		for (size_t i = 0; i < m_vertices.size(); i++)
		{
			m_vertices[i].m_position.y *= ratio;
		}
	}
	else if (m_imageHeight > m_imageWidth)
	{
		float ratio = static_cast<float>(m_imageWidth) / static_cast<float>(m_imageHeight);
		for (size_t i = 0; i < m_vertices.size(); i++)
		{
			m_vertices[i].m_position.x *= ratio;
		}
	}

	m_meshDataDirty = true;
}

void UIImage::SetOpacity(float opacity)
{
	m_opacity = opacity;
	GetOwner()->RequestInstanceBufferUpdate();
}

void UIImage::SetColor(glm::vec3 color)
{
	m_color = color;
	GetOwner()->RequestInstanceBufferUpdate();
}