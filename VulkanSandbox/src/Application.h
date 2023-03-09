#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Image.h"
#include "IndexBuffer.h"
#include "LogicalDevice.h"
#include "PhysicalDevice.h"
#include "Pipeline.h"
#include "Swapchain.h"
#include "UniformBuffer.h"
#include "VertexBuffer.h"
#include "Vulkan.h"

class Application
{
public:
	Application();

	static Application& Get() { return *s_instance; }
	VkInstance GetInstance() { return m_instance; }
	GLFWwindow* GetWindow() { return m_window; }

	const std::shared_ptr<LogicalDevice>& GetDevice() const { return m_logicalDevice; }
	const std::shared_ptr<Swapchain>& GetSwapchain() const { return m_swapchain; }
	const std::shared_ptr<UniformBuffer>& GetUniformBuffer() const { return m_uniformBuffer; }

	void Run();
	void Shutdown();
	
private:
	void BeginFrame();

	bool HasValidationLayerSupport();
	std::vector<const char*> GetRequiredExtensions();

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

	std::shared_ptr<VertexBuffer> m_vertexBuffer;
	std::shared_ptr<IndexBuffer> m_indexBuffer;
	std::shared_ptr<UniformBuffer> m_uniformBuffer;

	std::shared_ptr<Image> m_image;
	VkSampler m_sampler;

	// Shader? Renderer?
	VkDescriptorPool m_descriptorPool;
	std::vector<VkDescriptorSet> m_descriptorSets;
};