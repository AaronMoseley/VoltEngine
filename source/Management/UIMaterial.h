#ifndef VOLTENGINE_UIMATERIAL_H
#define VOLTENGINE_UIMATERIAL_H

#include "Material.h"

class UIMaterial : public Material {
public:
	UIMaterial(const std::string& materialName) : Material(materialName)
	{}

protected:
	void CreateDescriptorSetLayout(const MaterialRegistry::MaterialCreationData& creationData) override;
	void CreateDescriptorSets(const MaterialRegistry::MaterialCreationData& creationData) override;
	void CreateDescriptorPool(const MaterialRegistry::MaterialCreationData& creationData) override;
};



#endif //VOLTENGINE_UIMATERIAL_H
