#ifndef VOLTENGINE_GENERICUIOBJECTMATERIAL_H
#define VOLTENGINE_GENERICUIOBJECTMATERIAL_H

#include "Management/UIMaterial.h"

class GenericUIObjectMaterial : public UIMaterial
{
public:
	struct  UIInstanceInfo
	{
		glm::vec4 m_objectWorldPosition;
		glm::vec4 m_scale;
		glm::vec4 m_colorRGB;
		glm::vec4 m_textureOffsetInAtlas;
		//x, y is texture size, z, w is offset
		glm::vec4 m_characterTextureSizeAndPositionOffset;
		//x is textured boolean, y is texture index, z is isTextCharacter boolean
		glm::uvec4 m_displayProperties;
	};

	struct  UIVertex
	{
		glm::vec4 m_position;
		glm::vec4 m_textureCoordinate;
	};

	GenericUIObjectMaterial() : UIMaterial(kMaterialName)
	{

	}

	static const std::string& GetNameStatic() {return kMaterialName;}

	size_t GetInstanceInfoSize() override { return sizeof(UIInstanceInfo); };
	void GetInstanceInfo(RenderObject* object, const std::vector<std::filesystem::path>& textureFilePaths, std::vector<std::byte>& outData) override;

	size_t GetInstanceCount(RenderObject* object) override;

protected:
	std::filesystem::path GetVertexShaderPath() override { return "shaders/HLSL/UIVertexShader.spv"; }
	std::filesystem::path GetPixelShaderPath() override { return "shaders/HLSL/UIPixelShader.spv"; }

	bool UpdateInstanceInfoFromImage(RenderObject* object, const std::vector<std::filesystem::path>& textureFilePaths, std::vector<std::byte>& outData);
	bool UpdateInstanceInfoFromText(RenderObject* object, const std::vector<std::filesystem::path>& textureFilePaths, std::vector<std::byte>& outData);

	void CreateVertexFormat() override;

private:
	static MaterialAutoRegister<GenericUIObjectMaterial> s_register;

	inline static const std::string kMaterialName = "GenericUIObjectMaterial";
};

#endif //VOLTENGINE_GENERICUIOBJECTMATERIAL_H
