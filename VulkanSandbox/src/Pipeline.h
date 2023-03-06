#pragma once

#include "LogicalDevice.h"

class Pipeline
{
public:
	Pipeline(const std::shared_ptr<LogicalDevice>& device);

	void Destroy();

	VkPipeline GetPipeline() { return m_pipeline; }

private:
	VkShaderModule CreateShaderModule(const std::vector<char>& byteCode);

private:
	std::shared_ptr<LogicalDevice> m_logicalDevice;

	VkPipeline m_pipeline;
	VkPipelineLayout m_pipelineLayout;

	VkDescriptorSetLayout m_descriptorLayout;
};
