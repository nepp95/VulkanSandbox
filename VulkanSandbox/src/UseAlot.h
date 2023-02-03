#pragma once

#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>

// Macros
#define LOG(x) std::cout << x << std::endl
#define VK_CHECK(func, msg) { if (func != VK_SUCCESS) LOG(msg); }

// Debugging enabled
#ifdef VDEBUG
	const bool ENABLE_VALIDATION_LAYERS = true;
#else
	const bool ENABLE_VALIDATION_LAYERS = false;
#endif

// Global width/height
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

// Validation layers
const std::vector<const char*> VALIDATION_LAYERS = {
	"VK_LAYER_KHRONOS_validation"
};

// Device extensions
const std::vector<const char*> DEVICE_EXTENSIONS = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// Debug callback
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	/*
		MessageSeverity:
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: Diagnostic message
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: Informational message like the creation of a resource
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: Message about behavior that is not necessarily an error, but very likely a bug in your application
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: Message about behavior that is invalid and may cause crashes

		MessageType:
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: Some event has happened that is unrelated to the specification or performance
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: Something has happened that violates the specification or indicates a possible mistake
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: Potential non-optimal use of Vulkan
	*/
	std::stringstream ss;
	ss << "Validation layer: " << pCallbackData->pMessage;
	LOG(ss.str());

	return VK_FALSE;
}

// File reading
static std::vector<char> ReadBytes(const std::string& filepath)
{
	std::ifstream stream(filepath, std::ios::binary | std::ios::ate);
	if (!stream)
		return {};

	std::streampos end = stream.tellg();
	stream.seekg(0, std::ios::beg);
	uint32_t fileSize = (uint32_t)(end - stream.tellg());

	if (fileSize == 0)
		return {};

	std::vector<char> fileData(fileSize);
	stream.read(fileData.data(), fileSize);
	stream.close();

	return fileData;
}