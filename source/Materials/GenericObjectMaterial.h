#ifndef VOLTENGINE_GENERICOBJECTMATERIAL_H
#define VOLTENGINE_GENERICOBJECTMATERIAL_H

#include "Management/MaterialRegistry.h"
#include "Management/Material.h"

class GenericObjectMaterial : public Material {
public:
	struct  InstanceInfo {
		glm::mat4 m_modelMatrix;
		glm::mat4 m_modelMatrixInverse;

		glm::vec4 m_scale;

		glm::vec4 m_ambient;
		glm::vec4 m_diffuse;
		glm::vec4 m_specular;
		//x is opacity, y is shininess
		glm::vec4 m_opacityAndShininess;

		//x is lit boolean, y is textured boolean, z is texture index, w is billboarded boolean
		glm::uvec4 m_displayProperties;
	};

	struct Vertex {
		glm::vec4 m_position;
		glm::vec4 m_normalVector;
		glm::vec4 m_textureCoordinate;
	};

	GenericObjectMaterial() : Material(kMaterialName)
	{

	}

	static const std::string& GetNameStatic() { return kMaterialName; }

	size_t GetInstanceInfoSize() override { return sizeof(InstanceInfo); };
	void GetInstanceInfo(RenderObject* object, const std::vector<std::filesystem::path>& textureFilePaths, std::vector<std::byte>& outData) override;

	size_t GetInstanceCount(RenderObject* object) override { return 1; };

protected:
	std::filesystem::path GetVertexShaderPath() override { return "shaders/HLSL/VertexShader.spv"; }
	std::filesystem::path GetPixelShaderPath() override { return "shaders/HLSL/PixelShader.spv"; }

	void CreateVertexFormat() override;

private:
	static MaterialAutoRegister<GenericObjectMaterial> s_register;

	inline static const std::string kMaterialName = "GenericObjectMaterial";
};

#endif //VOLTENGINE_GENERICOBJECTMATERIAL_H
