#pragma once

#include "Device/LogicalDevice.h"
#include "Device/Swapchain.h"

struct GLFWwindow;

class RendererContext
{
public:
	RendererContext(GLFWwindow* windowHandle);

	void Init();
	void Shutdown();
	void SwapBuffers();

	const std::shared_ptr<PhysicalDevice>& GetPhysicalDevice() { return m_physicalDevice; }
	const std::shared_ptr<LogicalDevice>& GetLogicalDevice() { return m_logicalDevice; }
	const std::shared_ptr<Swapchain>& GetSwapchain() { return m_swapchain; }

private:
	bool HasValidationSupport();
	std::vector<const char*> GetRequiredDeviceExtensions();

private:
	GLFWwindow* m_windowHandle;

	std::shared_ptr<PhysicalDevice> m_physicalDevice;
	std::shared_ptr<LogicalDevice> m_logicalDevice;
	std::shared_ptr<Swapchain> m_swapchain;

	inline static VkInstance s_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;
};