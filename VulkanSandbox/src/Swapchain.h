#pragma once

#include "LogicalDevice.h"

struct SwapchainSupportDetails
{
	VkSurfaceCapabilitiesKHR Capabilities;
	std::vector<VkSurfaceFormatKHR> Formats;
	std::vector<VkPresentModeKHR> PresentModes;
};

class Swapchain
{
public:
	Swapchain(const std::shared_ptr<LogicalDevice>& device);

	void Create();
	void Recreate();
	void Cleanup();
	void Destroy();
	void DestroySurface();

	void BeginFrame();
	void Present();

	void OnResize();

	uint32_t GetWidth() { return m_width; }
	uint32_t GetHeight() { return m_height; }

	VkRenderPass GetRenderPass() { return m_renderPass; }

private:
	uint32_t GetNextImage();
	void RecordCommandBuffer();

	SwapchainSupportDetails QuerySwapchainSupport();
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

private:
	std::shared_ptr<LogicalDevice> m_logicalDevice;

	VkSurfaceKHR m_surface;
	VkSwapchainKHR m_swapchain;
	VkExtent2D m_extent;
	VkFormat m_format;

	VkRenderPass m_renderPass;

	VkSemaphore m_presentSemaphore;
	VkSemaphore m_renderSemaphore;
	std::vector<VkFence> m_fences;

	VkCommandPool m_commandPool;
	std::vector<VkCommandBuffer> m_commandBuffers;

	uint32_t m_currentFrameIndex = 0;
	uint32_t m_currentIndex = 0;
	uint32_t m_width, m_height;

	bool m_recreateNeeded = false;
	bool m_isCleanedUp = false;

	std::vector<VkImage> m_images;
	std::vector<VkImageView> m_imageViews;
	std::vector<VkFramebuffer> m_framebuffers;
};
