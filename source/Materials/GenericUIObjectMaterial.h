#ifndef VOLTENGINE_GENERICUIOBJECTMATERIAL_H
#define VOLTENGINE_GENERICUIOBJECTMATERIAL_H

#include "Management/UIMaterial.h"

class GenericUIObjectMaterial : public UIMaterial {
public:
	GenericUIObjectMaterial() : UIMaterial("GenericUIObjectMaterial")
	{

	}

	size_t GetInstanceInfoSize() override { return sizeof(VulkanCommonFunctions::UIInstanceInfo); };
	void GetInstanceInfo(RenderObject* object, const std::vector<std::filesystem::path>& textureFilePaths, std::vector<std::byte>& outData) override;

	size_t GetInstanceCount(RenderObject* object) override;

protected:
	std::filesystem::path GetVertexShaderPath() override { return "shaders/HLSL/UIVertexShader.spv"; }
	std::filesystem::path GetPixelShaderPath() override { return "shaders/HLSL/UIPixelShader.spv"; }

	bool UpdateInstanceInfoFromImage(RenderObject* object, const std::vector<std::filesystem::path>& textureFilePaths, std::vector<std::byte>& outData);
	bool UpdateInstanceInfoFromText(RenderObject* object, const std::vector<std::filesystem::path>& textureFilePaths, std::vector<std::byte>& outData);

	bool IsUIBased() override { return true; }

private:
	static MaterialAutoRegister<GenericUIObjectMaterial> s_register;
};

MaterialAutoRegister<GenericUIObjectMaterial> GenericUIObjectMaterial::s_register;

#endif //VOLTENGINE_GENERICUIOBJECTMATERIAL_H
