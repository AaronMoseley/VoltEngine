#include "FontManager.h"

std::shared_ptr<FontManager> FontManager::s_fontManagerSingleton = nullptr;

std::shared_ptr<FontManager> FontManager::Get()
{
	if (s_fontManagerSingleton == nullptr)
	{
		s_fontManagerSingleton = std::make_shared<FontManager>();
	}

	return s_fontManagerSingleton;
}

std::shared_ptr<Font> FontManager::AddFont(std::string atlasFilePath, std::string descriptionFilePath)
{
	std::shared_ptr<Font> newFont = std::make_shared<Font>(atlasFilePath, descriptionFilePath);
	m_fonts[newFont->GetFontName()] = newFont;

	return newFont;
}

std::shared_ptr<Font> FontManager::GetFontByName(const std::string& fontName)
{
	auto it = m_fonts.find(fontName);
	if (it != m_fonts.end())
	{
		return it->second;
	}
	return nullptr;
}