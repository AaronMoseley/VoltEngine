#pragma once

#include "Objects/ObjectComponent.h"
#include "Vulkan Interface/VulkanCommonFunctions.h"
#include "Components/Transform.h"

class DemoBehavior : public ObjectComponent {
public:
    DemoBehavior() {};

    void Start() override;
    void Update(float deltaTime) override;

    void WriteDebugText();

private:
    alignas(16) std::vector<glm::vec3> kObjectPositions = {
        glm::vec3(0.0f,  0.0f,  0.0f),
        glm::vec3(2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f,  2.0f, -2.5f),
        glm::vec3(1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    std::vector<VulkanCommonFunctions::Vertex> kSquareVertices = {
        //positions                                     //normals                                       //texture coords
        {{-0.5f,  0.5f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}}, //top left
        {{ 0.5f,  0.5f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 0.0f}}, //top right
        {{ 0.5f, -0.5f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}}, //bottom right
        {{-0.5f, -0.5f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}}  //bottom left
    };

    std::vector<uint16_t> kSquareIndices = {
        0, 1, 2, 2, 3, 0
    };

	float m_lightOrbitRadius = 5.0f;
    VulkanCommonFunctions::ObjectHandle m_lightObjectHandle = 0;
	std::set<VulkanCommonFunctions::ObjectHandle> m_objectHandles;
	float m_currentTime = 0.0f;
};