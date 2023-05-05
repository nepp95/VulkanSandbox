#pragma once

#include "../Memory/Allocator.h"

#include <filesystem>

class Image
{
public:
	Image(uint32_t width, uint32_t height);
	Image(const std::filesystem::path& filepath);
	~Image();

	VkImageView GetImageView() { return m_imageView; }

	uint32_t GetWidth() const { return m_width; }
	uint32_t GetHeight() const { return m_height; }

private:
	uint32_t m_width;
	uint32_t m_height;

	VkImage m_image;
	VkImageView m_imageView;
	VmaAllocation m_allocation;
};
