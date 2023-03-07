#pragma once

#include "LogicalDevice.h"

class Pipeline
{
public:
	Pipeline(const std::shared_ptr<LogicalDevice>& device);

	void Destroy();

	VkPipeline GetPipeline() { return m_pipeline; }
	VkPipelineLayout GetPipelineLayout() { return m_pipelineLayout; }
	VkDescriptorSetLayout GetDescriptorLayout() { return m_descriptorLayout; }

private:
	VkShaderModule CreateShaderModule(const std::vector<char>& byteCode);

private:
	std::shared_ptr<LogicalDevice> m_logicalDevice;

	VkPipeline m_pipeline;
	VkPipelineLayout m_pipelineLayout;

	VkDescriptorSetLayout m_descriptorLayout;
};
