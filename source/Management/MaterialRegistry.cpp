#include "MaterialRegistry.h"
#include "Management/Material.h"
#include "Vulkan Interface/GraphicsBuffer.h"
#include "Vulkan Interface/TextureImage.h"
#include "Vulkan Interface/VulkanWindow.h"

std::shared_ptr<MaterialRegistry> MaterialRegistry::s_materialRegistrySingleton = nullptr;

MaterialRegistry::MaterialRegistry()
{

}

void MaterialRegistry::CleanupMaterials()
{
	for (auto it = m_registeredMaterials.begin(); it != m_registeredMaterials.end(); it++)
	{
		it->second->CleanupMaterial();
	}
}

std::shared_ptr<MaterialRegistry> MaterialRegistry::Get()
{
	if (s_materialRegistrySingleton == nullptr)
	{
		s_materialRegistrySingleton = std::make_shared<MaterialRegistry>();
	}

	return s_materialRegistrySingleton;
}

std::shared_ptr<Material> MaterialRegistry::GetMaterialByName(const std::string& materialName) const
{
	if (m_registeredMaterials.contains(materialName) == false)
	{
		return nullptr;
	}

	return m_registeredMaterials.at(materialName);
}

const std::map<std::string, std::shared_ptr<Material>>& MaterialRegistry::GetMaterials()
{
	return m_registeredMaterials;
}

void MaterialRegistry::InitializeAllMaterials(const MaterialCreationData& creationData)
{
	//need to add parameters that are used to initialize graphics buffers
	for (auto it = m_registeredMaterials.begin(); it != m_registeredMaterials.end(); it++)
	{
		it->second->InitializeMaterial(creationData);
	}
}

void MaterialRegistry::RegisterMaterial(std::shared_ptr<Material> materialToRegister)
{
	m_registeredMaterials[materialToRegister->GetMaterialName()] = materialToRegister;
}