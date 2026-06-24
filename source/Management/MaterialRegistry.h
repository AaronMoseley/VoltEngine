#ifndef VOLTENGINE_MATERIALREGISTRY_H
#define VOLTENGINE_MATERIALREGISTRY_H

#include <memory>
#include <map>

class Material;

class MaterialRegistry {
public:
	MaterialRegistry();

	static std::shared_ptr<MaterialRegistry> Get();

	void RegisterMaterial(Material* materialToRegister);
	void InitializeAllMaterials();

	const std::map<std::string, Material*>& GetMaterials();
	Material* GetMaterialByName(const std::string& materialName) const;

private:
	static std::shared_ptr<MaterialRegistry> s_materialRegistrySingleton;

	std::map<std::string, Material*> m_registeredMaterials;
};



#endif //VOLTENGINE_MATERIALREGISTRY_H
