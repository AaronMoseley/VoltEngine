#pragma once

#ifndef VULKAN_COMMON_FUNCTIONS
#define VULKAN_COMMON_FUNCTIONS

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_STD340

#include "ThirdParty/vk_mem_alloc.h"

#include "glm.hpp"

#include <optional>
#include <vector>
#include <array>

namespace VulkanCommonFunctions {
    using ObjectHandle = size_t;
    static const VulkanCommonFunctions::ObjectHandle INVALID_OBJECT_HANDLE = 0;
    static const size_t MAX_OBJECTS = 10000;
    
    struct alignas(256) GlobalInfo {
         glm::mat4 view;
         glm::mat4 proj;
         glm::vec4 cameraPosition;
        //only x is used
         glm::uvec4 lightCount;
    };

    static_assert(sizeof(GlobalInfo) % 16 == 0);

    struct  LightInfo {
         glm::vec4 lightPosition;
         glm::vec4 lightColor;

         glm::vec4 lightAmbient;
         glm::vec4 lightDiffuse;
         glm::vec4 lightSpecular;

        //only x is used
         glm::vec4 maxLightDistance;
    };

    static_assert(sizeof(LightInfo) % 16 == 0);

    struct alignas(256) UIGlobalInfo {
        //x is width, y is height
         glm::uvec4 screenSize;
    };

    static_assert(sizeof(UIGlobalInfo) % 16 == 0);

    struct  InstanceInfo {
         glm::mat4 modelMatrix;
         glm::mat4 modelMatrixInverse;

         glm::vec4 scale;

         glm::vec4 ambient;
         glm::vec4 diffuse;
         glm::vec4 specular;
        //x is opacity, y is shininess
         glm::vec4 opacityAndShininess;

        //x is lit boolean, y is textured boolean, z is texture index, w is billboarded boolean
         glm::uvec4 displayProperties;
    };

    static_assert(sizeof(InstanceInfo) % 16 == 0);

    struct  Vertex {
         glm::vec4 pos;
         glm::vec4 normal;
         glm::vec4 texCoord;

        static std::array<VkVertexInputBindingDescription, 2> GetBindingDescriptions() {
            std::array<VkVertexInputBindingDescription, 2> result;

            result[0].binding = 0;
            result[0].stride = sizeof(Vertex);
            result[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            result[1].binding = 1;
            result[1].stride = sizeof(InstanceInfo);
            result[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

            return result;
        }

        static std::array<VkVertexInputAttributeDescription, 17> GetAttributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 17> attributeDescriptions{};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, normal);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

            for (uint32_t i = 3; i < 7; i++) {
                attributeDescriptions[i].binding = 1; // instance buffer binding index
                attributeDescriptions[i].location = i; // consecutive locations (e.g., 2,3,4,5)
                attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT; // vec4 per row
                attributeDescriptions[i].offset = sizeof(glm::vec4) * (i - 3);
            }

            for (uint32_t i = 7; i < 11; i++) {
                attributeDescriptions[i].binding = 1; // instance buffer binding index
                attributeDescriptions[i].location = i; // consecutive locations (e.g., 2,3,4,5)
                attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT; // vec4 per row
                attributeDescriptions[i].offset = sizeof(glm::mat4) + sizeof(glm::vec4) * (i - 7);
            }

            for (uint32_t i = 11; i < 16; i++)
            {
                attributeDescriptions[i].binding = 1;
                attributeDescriptions[i].location = i;
                attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
                attributeDescriptions[i].offset = (2 * sizeof(glm::mat4)) + (sizeof(glm::vec4) * (i - 11));
            }

            attributeDescriptions[16].binding = 1;
            attributeDescriptions[16].location = 16;
            attributeDescriptions[16].format = VK_FORMAT_R32G32B32A32_UINT;
            attributeDescriptions[16].offset = offsetof(InstanceInfo, displayProperties);

            return attributeDescriptions;
        }
    };

    static_assert(sizeof(Vertex) % 16 == 0);

    struct  UIInstanceInfo {
         glm::vec4 objectPosition;
         glm::vec4 scale;
         glm::vec4 color;
         glm::vec4 textureOffset;
        //x, y is texture size, z, w is offset
         glm::vec4 characterTextureSizeAndOffset;
        //x is textured boolean, y is texture index, z is isTextCharacter boolean
         glm::uvec4 displayProperties;
    };

    static_assert(sizeof(UIInstanceInfo) % 16 == 0);

    struct  UIVertex {
         glm::vec4 position;
         glm::vec4 texCoord;

        static std::array<VkVertexInputBindingDescription, 2> GetBindingDescriptions() {
            std::array<VkVertexInputBindingDescription, 2> result;

            result[0].binding = 0;
            result[0].stride = sizeof(UIVertex);
            result[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            result[1].binding = 1;
            result[1].stride = sizeof(UIInstanceInfo);
            result[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

            return result;
        }

        static std::array<VkVertexInputAttributeDescription, 8> GetAttributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 8> attributeDescriptions{};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(UIVertex, position);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(UIVertex, texCoord);

            attributeDescriptions[2].binding = 1;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(UIInstanceInfo, objectPosition);

            attributeDescriptions[3].binding = 1;
            attributeDescriptions[3].location = 3;
            attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            attributeDescriptions[3].offset = offsetof(UIInstanceInfo, scale);

            attributeDescriptions[4].binding = 1;
            attributeDescriptions[4].location = 4;
            attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            attributeDescriptions[4].offset = offsetof(UIInstanceInfo, color);

            attributeDescriptions[5].binding = 1;
            attributeDescriptions[5].location = 5;
            attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            attributeDescriptions[5].offset = offsetof(UIInstanceInfo, textureOffset);

            attributeDescriptions[6].binding = 1;
            attributeDescriptions[6].location = 6;
            attributeDescriptions[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            attributeDescriptions[6].offset = offsetof(UIInstanceInfo, characterTextureSizeAndOffset);

            attributeDescriptions[7].binding = 1;
            attributeDescriptions[7].location = 7;
            attributeDescriptions[7].format = VK_FORMAT_R32G32B32A32_UINT;
            attributeDescriptions[7].offset = offsetof(UIInstanceInfo, displayProperties);

            return attributeDescriptions;
        }
    };

    static_assert(sizeof(UIVertex) % 16 == 0);

    struct QueueFamilyIndices {
        std::optional<uint32_t> m_graphicsFamily;
        std::optional<uint32_t> m_presentFamily;

        bool IsComplete() {
            return m_graphicsFamily.has_value() && m_presentFamily.has_value();
        }
    };

    VkCommandBuffer BeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue);
    bool HasStencilComponent(VkFormat format);
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
}

#endif