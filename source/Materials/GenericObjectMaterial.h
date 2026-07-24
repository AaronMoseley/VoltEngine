#ifndef VOLTENGINE_GENERICOBJECTMATERIAL_H
#define VOLTENGINE_GENERICOBJECTMATERIAL_H

#include "Management/MaterialRegistry.h"
#include "Management/Material.h"

class GenericObjectMaterial : public Material {
public:
	GenericObjectMaterial() : Material("GenericObjectMaterial")
	{

	}

	size_t GetInstanceInfoSize() override { return sizeof(VulkanCommonFunctions::InstanceInfo); };
	void GetInstanceInfo(RenderObject* object, const std::vector<std::filesystem::path>& textureFilePaths, std::vector<std::byte>& outData) override;

	size_t GetInstanceCount(RenderObject* object) override { return 1; };

protected:
	std::filesystem::path GetVertexShaderPath() override { return "shaders/HLSL/VertexShader.spv"; }
	std::filesystem::path GetPixelShaderPath() override { return "shaders/HLSL/PixelShader.spv"; }

	bool IsUIBased() override { return false; }

private:
	static MaterialAutoRegister<GenericObjectMaterial> s_register;
};

MaterialAutoRegister<GenericObjectMaterial> GenericObjectMaterial::s_register;

#endif //VOLTENGINE_GENERICOBJECTMATERIAL_H
