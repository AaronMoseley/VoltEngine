#include "MaterialRegistry.h"
#include "Management/Material.h"

std::shared_ptr<MaterialRegistry> MaterialRegistry::s_materialRegistrySingleton = nullptr;

MaterialRegistry::MaterialRegistry()
{

}

std::shared_ptr<MaterialRegistry> MaterialRegistry::Get()
{
	if (s_materialRegistrySingleton == nullptr)
	{
		s_materialRegistrySingleton = std::make_shared<MaterialRegistry>();
	}

	return s_materialRegistrySingleton;
}

Material* MaterialRegistry::GetMaterialByName(const std::string& materialName) const
{
	if (m_registeredMaterials.contains(materialName) == false)
	{
		return nullptr;
	}

	return m_registeredMaterials.at(materialName);
}

const std::map<std::string, Material*>& MaterialRegistry::GetMaterials()
{
	return m_registeredMaterials;
}

void MaterialRegistry::InitializeAllMaterials()
{
	//need to add parameters that are used to initialize graphics buffers
	for (auto it = m_registeredMaterials.begin(); it != m_registeredMaterials.end(); it++)
	{
		it->second->InitializeMaterial();
	}
}

void MaterialRegistry::RegisterMaterial(Material* materialToRegister)
{
	m_registeredMaterials[materialToRegister->GetMaterialName()] = materialToRegister;
}