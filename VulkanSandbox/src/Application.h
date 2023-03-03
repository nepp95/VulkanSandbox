#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "LogicalDevice.h"
#include "PhysicalDevice.h"
#include "UseAlot.h"
#include "Vulkan.h"

#include <optional>

struct SwapchainSupportDetails
{
	VkSurfaceCapabilitiesKHR Capabilities;
	std::vector<VkSurfaceFormatKHR> Formats;
	std::vector<VkPresentModeKHR> PresentModes;
};

class Application
{
public:
	Application();

	static Application& Get() { return *s_instance; }
	VkInstance GetInstance() { return m_instance; }

	void Run();
	void Shutdown();
	
private:
	void DrawFrame();

	bool HasValidationLayerSupport();
	std::vector<const char*> GetRequiredExtensions();

	// Physical device
	bool IsDeviceSuitable(VkPhysicalDevice device);

	// Surface
	void CreateSurface();

	// Swapchain
	SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void CreateSwapchain();
	void RecreateSwapchain();
	void CleanupSwapchain();
	void CreateFramebuffers();
	void OnResize();

	// ImageView
	void CreateImageViews();

	// Pipeline
	void CreateGraphicsPipeline();
	VkShaderModule CreateShaderModule(const std::vector<char>& byteCode);

	// Render pass
	void CreateRenderPass();

	// Command buffers
	void CreateCommandPool();
	void CreateCommandBuffers();
	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	// Syncing
	void CreateSyncObjects();

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

	VkSurfaceKHR m_surface;

	VkSwapchainKHR m_swapchain;
	std::vector<VkImage> m_swapchainImages;
	std::vector<VkImageView> m_swapchainImageViews;
	VkFormat m_swapchainImageFormat;
	VkExtent2D m_swapchainExtent;
	std::vector<VkFramebuffer> m_swapchainFramebuffers;
	bool m_framebufferResized{ false };

	VkPipeline m_graphicsPipeline;
	VkPipelineLayout m_pipelineLayout;
	VkRenderPass m_renderPass;

	VkCommandPool m_commandPool;
	std::vector<VkCommandBuffer> m_commandBuffers;

	std::vector<VkSemaphore> m_imageAvailableSemaphores;
	std::vector<VkSemaphore> m_renderFinishedSemaphores;
	std::vector<VkFence> m_inFlightFences;
	uint32_t m_currentFrame{ 0 };

	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexBufferMemory;
	VkBuffer m_indexBuffer;
	VkDeviceMemory m_indexBufferMemory;
};