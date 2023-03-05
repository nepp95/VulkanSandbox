#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "LogicalDevice.h"
#include "PhysicalDevice.h"
#include "Pipeline.h"
#include "Swapchain.h"
#include "UseAlot.h"
#include "Vulkan.h"

class Application
{
public:
	Application();

	static Application& Get() { return *s_instance; }
	VkInstance GetInstance() { return m_instance; }
	GLFWwindow* GetWindow() { return m_window; }

	const std::shared_ptr<Swapchain>& GetSwapchain() const { return m_swapchain; }

	void Run();
	void Shutdown();
	
private:
	void DrawFrame();

	bool HasValidationLayerSupport();
	std::vector<const char*> GetRequiredExtensions();

	// Command buffers
	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	// Vertex buffer
	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void CreateVertexBuffer();
	void CreateIndexBuffer();
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

private:
	GLFWwindow* m_window{ nullptr };
	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;

	static Application* s_instance;

	std::shared_ptr<PhysicalDevice> m_physicalDevice;
	std::shared_ptr<LogicalDevice> m_logicalDevice;
	std::shared_ptr<Swapchain> m_swapchain;
	std::shared_ptr<Pipeline> m_pipeline;

	bool m_framebufferResized{ false };

	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexBufferMemory;
	VkBuffer m_indexBuffer;
	VkDeviceMemory m_indexBufferMemory;
};