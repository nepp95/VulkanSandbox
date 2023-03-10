VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["glfw"] = "%{wks.location}/VulkanSandbox/vendor/glfw/include"
IncludeDir["glm"] = "%{wks.location}/VulkanSandbox/vendor/glm"
IncludeDir["stb"] = "%{wks.location}/VulkanSandbox/vendor/stb"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["Vma"] = "%{wks.location}/VulkanSandbox/vendor/vulkan-memory-allocator/include"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"