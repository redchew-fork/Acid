﻿#pragma once

#include <array>
#include <string>

#include "../../platforms/glfw/GlfwVulkan.hpp"
#include "../../shaders/Shader.hpp"
#include "../buffers/UniformBuffer.hpp"

namespace Flounder
{
	enum PipelineType
	{
		PipelinePolygon,
		PipelineNoDepthTest,
		PipelineMrt,
		PipelineMultiTexture
	};

	struct VertexInputState
	{
		uint32_t bindingDescriptionCount;
		uint32_t attributeDescriptionCount;
		VkVertexInputBindingDescription *pVertexBindingDescriptions;
		VkVertexInputAttributeDescription *pVertexAttributeDescriptions;
	};

	/// <summary>
	/// Class that represents a Vulkan pipeline.
	/// </summary>
	class Pipeline
	{
	private:
		static const std::vector<VkDynamicState> DYNAMIC_STATES;

		std::string m_name;
		PipelineType m_pipelineType;
		VertexInputState m_vertexInputState;

		Shader *m_shader;
		std::vector<UniformBuffer*> m_uniformBuffers;
		VkDescriptorPool m_descriptorPool;
		VkDescriptorSet m_descriptorSet;

		VkPipeline m_pipeline;
		VkPipelineLayout m_pipelineLayout;

		VkPipelineInputAssemblyStateCreateInfo m_inputAssemblyState;
		VkPipelineRasterizationStateCreateInfo m_rasterizationState;
		std::array<VkPipelineColorBlendAttachmentState, 1> m_blendAttachmentStates;
		VkPipelineColorBlendStateCreateInfo m_colourBlendState;
		VkPipelineDepthStencilStateCreateInfo m_depthStencilState;
		VkPipelineViewportStateCreateInfo m_viewportState;
		VkPipelineMultisampleStateCreateInfo m_multisampleState;
		VkPipelineDynamicStateCreateInfo m_dynamicState;
	public:
		/// <summary>
		/// Creates a new pipeline.
		/// </summary>
		/// <param name="name"> The pipelines name. </param>
		Pipeline(const std::string &name, const PipelineType &pipelineType, Shader *shader, const std::vector<UniformBuffer*> &uniformBuffers);

		/// <summary>
		/// Deconstructor for the pipeline.
		/// </summary>
		~Pipeline();

		/// <summary>
		/// Creates the pipeline.
		/// </summary>
		void Create(const VkDevice &logicalDevice, const VkRenderPass &renderPass, const VertexInputState &vertexInputState);

		/// <summary>
		/// Cleans up the shapipelineder.
		/// </summary>
		void Cleanup(const VkDevice &logicalDevice);

		/// <summary>
		/// Gets the loaded name for the pipeline.
		/// </summary>
		/// <returns> The pipelines name. </returns>
		std::string GetName() const { return m_name; }

		VkDescriptorPool GetDescriptorPool() { return m_descriptorPool; }

		VkDescriptorSet GetDescriptorSet() { return m_descriptorSet; }

		VkPipeline GetPipeline() const { return m_pipeline; }

		VkPipelineLayout GetPipelineLayout() const { return m_pipelineLayout; }
	private:
		void CreateAttributes();

		void CreateDescriptorPool(const VkDevice &logicalDevice);

		void CreateDescriptorSet(const VkDevice &logicalDevice);

		void CreatePipelineLayout(const VkDevice &logicalDevice);

		void CreatePolygonPipeline(const VkDevice &logicalDevice, const VkRenderPass &renderPass);

		void CreateNoDepthTestPipeline(const VkDevice &logicalDevice, const VkRenderPass &renderPass);

		void CreateMrtPipeline(const VkDevice &logicalDevice, const VkRenderPass &renderPass);

		void CreateMultiTexturePipeline(const VkDevice &logicalDevice, const VkRenderPass &renderPass);
	};
}