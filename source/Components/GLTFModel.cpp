#include "GLTFModel.h"

#include "Management/Scene.h"

GLTFModel::GLTFModel() : GenericObjectMeshRenderer()
{

}

void GLTFModel::Start()
{
	if (GetScene()->MeshAlreadyAdded(m_meshName))
	{
		return;
	}

	ReadModel();
	SetDirtyData(true);
}

void GLTFModel::SetSourcePath(const std::filesystem::path& sourceFilePath)
{
	m_meshName = sourceFilePath.string();
	m_sourcePath = sourceFilePath;
}

void GLTFModel::ReverseWindingOrder(bool changeFlag)
{
	if (changeFlag)
	{
		m_reverseWindingOrder = !m_reverseWindingOrder;
	}

	if (m_readFile == false)
	{
		return;
	}

	if (IsIndexed())
	{
		std::reverse(m_indices.begin(), m_indices.end());
	} else
	{
		std::reverse(m_vertices.begin(), m_vertices.end());
	}
}

void GLTFModel::ReadModel()
{
	tinygltf::TinyGLTF loader;
	std::string err, warn;

	bool success = loader.LoadASCIIFromFile(&m_model, &err, &warn, m_sourcePath);

	if (!warn.empty())
	{
		qDebug() << "Warning: " << warn << "\n";
	}

	if (!err.empty())
	{
		qDebug() << "Error: " << err  << "\n";
	}

	if (!success)
	{
		return;
	}

	//read position
	std::vector<uint8_t> positionBytes;
	ReadAttribute<glm::vec3>(kPositionAttributeName, positionBytes);
	std::vector<glm::vec3> positions(positionBytes.size() / sizeof(glm::vec3));
	memcpy(positions.data(), positionBytes.data(), positionBytes.size());

	if (positions.size() > 0)
	{
		m_modelOrigin = positions[0];
	}

	//read normal
	std::vector<uint8_t> normalBytes;
	bool hasNormal = ReadAttribute<glm::vec3>(kNormalAttributeName, normalBytes);
	std::vector<glm::vec3> normals(normalBytes.size() / sizeof(glm::vec3));
	if (hasNormal)
	{
		memcpy(normals.data(), normalBytes.data(), normalBytes.size());
	}

	//read texture coordinate
	std::vector<uint8_t> textureCoordinateBytes;
	bool hasTextureCoordinate = ReadAttribute<glm::vec2>(kTextureCoordinateAttributeName, textureCoordinateBytes);
	std::vector<glm::vec2> textureCoordinates(textureCoordinateBytes.size() / sizeof(glm::vec2));
	if (hasTextureCoordinate)
	{
		memcpy(textureCoordinates.data(), textureCoordinateBytes.data(), textureCoordinateBytes.size());
	}

	std::vector<uint8_t> colorBytes;
	bool hasColor = ReadAttribute<glm::vec3>(kColorAttributeName, colorBytes);
	std::vector<glm::vec3> vertexColor(colorBytes.size() / sizeof(glm::vec3));
	if (hasColor)
	{
		memcpy(vertexColor.data(), colorBytes.data(), colorBytes.size());
	}

	//create vector of vertices
	size_t vertexCount = std::max(std::max(positions.size(), normals.size()), textureCoordinates.size());
	m_vertices.resize(vertexCount);
	for(size_t i = 0; i < vertexCount; i++)
	{
		if (i < positions.size())
		{
			m_vertices[i].m_position = glm::vec4(positions[i] - m_modelOrigin, 0.0f);
		}

		if (i < normals.size())
		{
			m_vertices[i].m_normalVector = glm::vec4(normals[i], 0.0f);
		}

		if (i < textureCoordinates.size())
		{
			m_vertices[i].m_textureCoordinate = glm::vec4(textureCoordinates[i], 0.0f, 0.0f);
		}

		if (i < vertexColor.size())
		{
			m_vertices[i].m_color = glm::vec4(vertexColor[i], 1.0f);
		}
	}

	//read indices
	ReadIndices();

	m_readFile = true;

	if (m_reverseWindingOrder)
	{
		ReverseWindingOrder(false);
	}

	m_vertexCount = m_vertices.size();
}

void GLTFModel::ReadIndices()
{
	for (const tinygltf::Node& node : m_model.nodes)
	{
		if (node.mesh < 0)
		{
			continue;
		}

		const tinygltf::Mesh& mesh = m_model.meshes[node.mesh];

		for (const auto& primitive : mesh.primitives)
		{
			const tinygltf::Accessor&   accessor  = m_model.accessors[primitive.indices];
			const tinygltf::BufferView& view = m_model.bufferViews[accessor.bufferView];
			const tinygltf::Buffer&     buffer  = m_model.buffers[view.buffer];

			const uint8_t* rawData = buffer.data.data() + view.byteOffset + accessor.byteOffset;

			for (size_t i = 0; i < accessor.count; i++)
			{
				uint32_t idx = 0;
				switch (accessor.componentType)
				{
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
					idx = reinterpret_cast<const uint8_t*>(rawData)[i];
					break;
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
					idx = reinterpret_cast<const uint16_t*>(rawData)[i];
					break;
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
					idx = reinterpret_cast<const uint32_t*>(rawData)[i];
					break;
				}

				m_indices.push_back(idx);
			}
		}
	}

	SetIndexed(m_indices.size() >= 3);
	m_indexCount = m_indices.size();
}

template<typename T>
bool GLTFModel::AddAttributeFromNode(int nodeIndex, const std::string& attributeName, std::vector<uint8_t>& outBytes) const
{
	bool result = false;
	const tinygltf::Node node = m_model.nodes[nodeIndex];

	if (node.mesh >= 0)
	{
		const tinygltf::Mesh& mesh = m_model.meshes[node.mesh];

		for (const auto& primitive : mesh.primitives)
		{
			if (primitive.attributes.contains(attributeName) == false)
			{
				continue;
			}

			result = true;

			const tinygltf::Accessor&   accessor  = m_model.accessors[primitive.attributes.at(attributeName)];
			const tinygltf::BufferView& view = m_model.bufferViews[accessor.bufferView];
			const tinygltf::Buffer&     buffer  = m_model.buffers[view.buffer];

			const uint8_t* data = buffer.data.data() + view.byteOffset + accessor.byteOffset;

			size_t byteCount = sizeof(T) * accessor.count;
			outBytes.resize(byteCount);
			memcpy(outBytes.data(), data, byteCount);
		}
	}

	for (int childNodeIndex : node.children)
	{
		result = result || AddAttributeFromNode<T>(childNodeIndex, attributeName, outBytes);
	}

	return result;
}

template <typename T>
bool GLTFModel::ReadAttribute(const std::string& attributeName, std::vector<uint8_t>& outBytes) const
{
	bool result = false;

	for (const tinygltf::Scene& scene : m_model.scenes)
	{
		for (int nodeIndex : scene.nodes)
		{
			result = result || AddAttributeFromNode<T>(nodeIndex, attributeName, outBytes);
		}
	}

	return result;
}

template <typename T>
bool GLTFModel::GetAttribute(const std::string& attributeName, std::vector<T>& outAttributeData)
{
	if (m_customAttributes.contains(attributeName) == false)
	{
		std::vector<uint8_t> attributeData;
		bool hasAttribute = ReadAttribute<T>(attributeName, attributeData);

		if (hasAttribute == false)
		{
			return false;
		}

		m_customAttributes[attributeName] = attributeData;
	}

	outAttributeData.resize(m_customAttributes.at(attributeName).size() / sizeof(T));

	memcpy(outAttributeData.data(), m_customAttributes.at(attributeName).data(), m_customAttributes.at(attributeName).data());

	return true;
}