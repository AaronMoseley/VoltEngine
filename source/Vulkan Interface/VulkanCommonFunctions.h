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
    static constexpr VulkanCommonFunctions::ObjectHandle INVALID_OBJECT_HANDLE = 0;
    static constexpr size_t MAX_OBJECTS = 10000;

    static constexpr size_t MAX_FRAMES_IN_FLIGHT = 3;
    
    struct GlobalInfo {
         glm::mat4 m_viewMatrix;
         glm::mat4 m_projectionMatrix;
         glm::vec4 m_cameraPosition;
        //only x is used
         glm::uvec4 m_lightCount;
    };

    struct  LightInfo {
         glm::vec4 m_lightPosition;
         glm::vec4 m_lightColor;

         glm::vec4 m_lightAmbient;
         glm::vec4 m_lightDiffuse;
         glm::vec4 m_lightSpecular;

        //only x is used
         glm::vec4 m_maxLightDistance;
    };

    struct UIGlobalInfo {
        //x is width, y is height
         glm::uvec4 m_screenSize;
    };

    struct QueueFamilyIndices {
        std::optional<uint32_t> m_graphicsFamily;
        std::optional<uint32_t> m_presentFamily;

        bool IsComplete() const
        {
            return m_graphicsFamily.has_value() && m_presentFamily.has_value();
        }
    };

    VkCommandBuffer BeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue);
    bool HasStencilComponent(VkFormat format);
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
}

#endif