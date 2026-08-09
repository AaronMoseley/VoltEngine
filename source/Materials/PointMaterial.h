#ifndef POINTCLOUDAPP_POINTMATERIAL_H
#define POINTCLOUDAPP_POINTMATERIAL_H

#include "Management/Material.h"

class PointMaterial : public Material {
public:
	struct PointInstanceInfo
	{
		glm::mat4 m_modelMatrix;
		glm::vec4 m_color;
	};

	struct PointVertex
	{
		glm::vec4 m_position;
	};

	PointMaterial() : Material(kMaterialName)
	{
		SetAllowTransparency(false);
	}

	static const std::string& GetNameStatic() { return kMaterialName; }

	size_t GetInstanceInfoSize() override { return sizeof(PointInstanceInfo); }
	void GetInstanceInfo(RenderObject* object, const std::vector<std::filesystem::path>& textureFilePaths, std::vector<std::byte>& outData) override;

	size_t GetInstanceCount(RenderObject* object) override { return 1; }

protected:
	std::filesystem::path GetVertexShaderPath() override { return "shaders/PointShaderVertex.spv"; }
	std::filesystem::path GetPixelShaderPath() override { return "shaders/PointShaderPixel.spv"; }

	void CreateVertexFormat() override;

private:
	static MaterialAutoRegister<PointMaterial> s_register;

	inline static const std::string kMaterialName = "PointMaterial";
};

#endif
