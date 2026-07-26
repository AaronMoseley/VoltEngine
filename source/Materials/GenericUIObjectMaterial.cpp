#include "GenericUIObjectMaterial.h"
#include "Components/Transform.h"
#include "Vulkan Interface/VulkanWindow.h"

MaterialAutoRegister<GenericUIObjectMaterial> GenericUIObjectMaterial::s_register;

void GenericUIObjectMaterial::GetInstanceInfo(RenderObject* object, const std::vector<std::filesystem::path>& textureFilePaths, std::vector<std::byte>& outData)
{
	bool handled = UpdateInstanceInfoFromImage(object, textureFilePaths, outData);
	if (!handled)
	{
		handled = UpdateInstanceInfoFromText(object, textureFilePaths, outData);
	}
}

size_t GenericUIObjectMaterial::GetInstanceCount(RenderObject* object)
{
	std::shared_ptr<Text> textComponent = object->GetComponent<Text>();

	if (textComponent == nullptr)
	{
		return 1;
	}

	return textComponent->GetTextString().size();
}

bool GenericUIObjectMaterial::UpdateInstanceInfoFromImage(RenderObject* object, const std::vector<std::filesystem::path>& textureFilePaths, std::vector<std::byte>& outData)
{
	UIInstanceInfo result {};
	std::shared_ptr<Transform> transform = object->GetComponent<Transform>();
	if (transform == nullptr)
	{
		return false;
	}
	std::shared_ptr<UIImage> imageComponent = object->GetComponent<UIImage>();
	if (imageComponent == nullptr)
	{
		return false;
	}
	result.m_objectWorldPosition = glm::vec4(transform->GetWorldPosition(), 1.0f);
	result.m_scale = glm::vec4(transform->GetWorldScale(), 1.0f);

	result.m_colorRGB = glm::vec4(imageComponent->GetColor(), imageComponent->GetOpacity());
	result.m_displayProperties.x = (imageComponent->GetTextured()) ? 1 : 0;
	//whether the instance is a character in text
	result.m_displayProperties.z = 0;

	auto iterator = std::find(textureFilePaths.begin(), textureFilePaths.end(), imageComponent->GetTexturePath());

	result.m_displayProperties.y = std::distance(textureFilePaths.begin(), iterator);
	if (result.m_displayProperties.y > textureFilePaths.size())
	{
		result.m_displayProperties.y = 0;
	}

	memcpy(outData.data(), &result, sizeof(UIInstanceInfo));

	return true;
}

void GenericUIObjectMaterial::CreateVertexFormat()
{
	m_vertexFormat = std::make_shared<VertexFormat>();

	m_vertexFormat->AddNewBinding(0, VK_VERTEX_INPUT_RATE_VERTEX);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);

	m_vertexFormat->AddNewBinding(1, VK_VERTEX_INPUT_RATE_INSTANCE);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_SFLOAT);
	m_vertexFormat->AddAttributeToBinding(VK_FORMAT_R32G32B32A32_UINT);

	m_vertexFormat->Finalize();
}

bool GenericUIObjectMaterial::UpdateInstanceInfoFromText(RenderObject* object, const std::vector<std::filesystem::path>& textureFilePaths, std::vector<std::byte>& outData)
{
	std::shared_ptr<Text> textComponent = object->GetComponent<Text>();

	if (textComponent == nullptr)
	{
		return false;
	}

	std::pair<size_t, size_t> screenSize = {m_vulkanWindow->swapChainImageSize().width(), m_vulkanWindow->swapChainImageSize().height() };

	std::string fontName = textComponent->GetFontName();
	std::shared_ptr<Font> font = FontManager::Get()->GetFontByName(fontName);

	std::string atlasFilePath = font->GetAtlasFilePath();

	auto filePathIterator = std::find(textureFilePaths.begin(), textureFilePaths.end(), atlasFilePath);

	if (filePathIterator == textureFilePaths.end())
	{
		return false;
	}

	size_t textureIndex = std::distance(textureFilePaths.begin(), filePathIterator);

	textComponent->GetInstanceInfo(screenSize, font, textureIndex, outData);

	return true;
}