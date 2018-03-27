﻿#include "Pipeline.hpp"

#include <cassert>
#include "../../Devices/Display.hpp"
#include "Helpers/FileSystem.hpp"
#include "Helpers/FormatString.hpp"
#include "../Renderer.hpp"

namespace Flounder
{
	const std::vector<VkDynamicState> DYNAMIC_STATES = {
		VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
	};

	const TBuiltInResource DefaultTBuiltInResource =
	{
		/* .MaxLights = */ 32,
		/* .MaxClipPlanes = */ 6,
		/* .MaxTextureUnits = */ 32,
		/* .MaxTextureCoords = */ 32,
		/* .MaxVertexAttribs = */ 64,
		/* .MaxVertexUniformComponents = */ 4096,
		/* .MaxVaryingFloats = */ 64,
		/* .MaxVertexTextureImageUnits = */ 32,
		/* .MaxCombinedTextureImageUnits = */ 80,
		/* .MaxTextureImageUnits = */ 32,
		/* .MaxFragmentUniformComponents = */ 4096,
		/* .MaxDrawBuffers = */ 32,
		/* .MaxVertexUniformVectors = */ 128,
		/* .MaxVaryingVectors = */ 8,
		/* .MaxFragmentUniformVectors = */ 16,
		/* .MaxVertexOutputVectors = */ 16,
		/* .MaxFragmentInputVectors = */ 15,
		/* .MinProgramTexelOffset = */ -8,
		/* .MaxProgramTexelOffset = */ 7,
		/* .MaxClipDistances = */ 8,
		/* .MaxComputeWorkGroupCountX = */ 65535,
		/* .MaxComputeWorkGroupCountY = */ 65535,
		/* .MaxComputeWorkGroupCountZ = */ 65535,
		/* .MaxComputeWorkGroupSizeX = */ 1024,
		/* .MaxComputeWorkGroupSizeY = */ 1024,
		/* .MaxComputeWorkGroupSizeZ = */ 64,
		/* .MaxComputeUniformComponents = */ 1024,
		/* .MaxComputeTextureImageUnits = */ 16,
		/* .MaxComputeImageUniforms = */ 8,
		/* .MaxComputeAtomicCounters = */ 8,
		/* .MaxComputeAtomicCounterBuffers = */ 1,
		/* .MaxVaryingComponents = */ 60,
		/* .MaxVertexOutputComponents = */ 64,
		/* .MaxGeometryInputComponents = */ 64,
		/* .MaxGeometryOutputComponents = */ 128,
		/* .MaxFragmentInputComponents = */ 128,
		/* .MaxImageUnits = */ 8,
		/* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
		/* .MaxCombinedShaderOutputResources = */ 8,
		/* .MaxImageSamples = */ 0,
		/* .MaxVertexImageUniforms = */ 0,
		/* .MaxTessControlImageUniforms = */ 0,
		/* .MaxTessEvaluationImageUniforms = */ 0,
		/* .MaxGeometryImageUniforms = */ 0,
		/* .MaxFragmentImageUniforms = */ 8,
		/* .MaxCombinedImageUniforms = */ 8,
		/* .MaxGeometryTextureImageUnits = */ 16,
		/* .MaxGeometryOutputVertices = */ 256,
		/* .MaxGeometryTotalOutputComponents = */ 1024,
		/* .MaxGeometryUniformComponents = */ 1024,
		/* .MaxGeometryVaryingComponents = */ 64,
		/* .MaxTessControlInputComponents = */ 128,
		/* .MaxTessControlOutputComponents = */ 128,
		/* .MaxTessControlTextureImageUnits = */ 16,
		/* .MaxTessControlUniformComponents = */ 1024,
		/* .MaxTessControlTotalOutputComponents = */ 4096,
		/* .MaxTessEvaluationInputComponents = */ 128,
		/* .MaxTessEvaluationOutputComponents = */ 128,
		/* .MaxTessEvaluationTextureImageUnits = */ 16,
		/* .MaxTessEvaluationUniformComponents = */ 1024,
		/* .MaxTessPatchComponents = */ 120,
		/* .MaxPatchVertices = */ 32,
		/* .MaxTessGenLevel = */ 64,
		/* .MaxViewports = */ 16,
		/* .MaxVertexAtomicCounters = */ 0,
		/* .MaxTessControlAtomicCounters = */ 0,
		/* .MaxTessEvaluationAtomicCounters = */ 0,
		/* .MaxGeometryAtomicCounters = */ 0,
		/* .MaxFragmentAtomicCounters = */ 8,
		/* .MaxCombinedAtomicCounters = */ 8,
		/* .MaxAtomicCounterBindings = */ 1,
		/* .MaxVertexAtomicCounterBuffers = */ 0,
		/* .MaxTessControlAtomicCounterBuffers = */ 0,
		/* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
		/* .MaxGeometryAtomicCounterBuffers = */ 0,
		/* .MaxFragmentAtomicCounterBuffers = */ 1,
		/* .MaxCombinedAtomicCounterBuffers = */ 1,
		/* .MaxAtomicCounterBufferSize = */ 16384,
		/* .MaxTransformFeedbackBuffers = */ 4,
		/* .MaxTransformFeedbackInterleavedComponents = */ 64,
		/* .MaxCullDistances = */ 8,
		/* .MaxCombinedClipAndCullDistances = */ 8,
		/* .MaxSamples = */ 4,
		/* .limits = */ {
			   /* .nonInductiveForLoops = */ true,
			   /* .whileLoops = */ true,
			   /* .doWhileLoops = */ true,
			   /* .generalUniformIndexing = */ true,
			   /* .generalAttributeMatrixVectorIndexing = */ true,
			   /* .generalVaryingIndexing = */ true,
			   /* .generalSamplerIndexing = */ true,
			   /* .generalVariableIndexing = */ true,
			   /* .generalConstantMatrixVectorIndexing = */ true,
		}
	};

	Pipeline::Pipeline(const GraphicsStage &graphicsStage, const PipelineCreate &pipelineCreateInfo) :
		m_graphicsStage(graphicsStage),
		m_pipelineCreateInfo(pipelineCreateInfo),
		m_modules(std::vector<VkShaderModule>()),
		m_stages(std::vector<VkPipelineShaderStageCreateInfo>()),
		m_descriptorSetLayout(VK_NULL_HANDLE),
		m_descriptorPool(VK_NULL_HANDLE),
		m_pipeline(VK_NULL_HANDLE),
		m_pipelineLayout(VK_NULL_HANDLE),
		m_inputAssemblyState({}),
		m_rasterizationState({}),
		m_blendAttachmentStates({}),
		m_colourBlendState({}),
		m_depthStencilState({}),
		m_viewportState({}),
		m_multisampleState({}),
		m_dynamicState({})
	{
#if FLOUNDER_VERBOSE
		const auto debugStart = Engine::Get()->GetTimeMs();
#endif

		CreateDescriptorLayout();
		CreateDescriptorPool();
		CreatePipelineLayout();
		CreateShaderProgram();
		CreateAttributes();

		switch (pipelineCreateInfo.pipelineModeFlags)
		{
		case PIPELINE_POLYGON:
			CreatePipelinePolygon();
			break;
		case PIPELINE_POLYGON_NO_DEPTH:
			CreatePipelinePolygonNoDepth();
			break;
		case PIPELINE_MRT:
			CreatePipelineMrt();
			break;
		case PIPELINE_MRT_NO_DEPTH:
			CreatePipelineMrtNoDepth();
			break;
		default:
			assert(false);
			break;
		}

#if FLOUNDER_VERBOSE
		const auto debugEnd = Engine::Get()->GetTimeMs();
		printf("Pipeline '%s' created in %fms\n", m_pipelineCreateInfo.shaderStages.at(0).c_str(), debugEnd - debugStart);
#endif
	}

	Pipeline::~Pipeline()
	{
		const auto logicalDevice = Display::Get()->GetLogicalDevice();

		for (auto shaderModule : m_modules)
		{
			vkDestroyShaderModule(logicalDevice, shaderModule, nullptr);
		}

		vkDestroyDescriptorSetLayout(logicalDevice, m_descriptorSetLayout, nullptr);
		vkDestroyDescriptorPool(logicalDevice, m_descriptorPool, nullptr);
		vkDestroyPipeline(logicalDevice, m_pipeline, nullptr);
		vkDestroyPipelineLayout(logicalDevice, m_pipelineLayout, nullptr);
	}

	void Pipeline::BindPipeline(const VkCommandBuffer &commandBuffer) const
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
	}

	DepthStencil *Pipeline::GetDepthStencil(const int &stage) const
	{
		return Renderer::Get()->GetRenderStage(stage == -1 ? m_graphicsStage.renderpass : stage)->m_depthStencil;
	}

	Texture *Pipeline::GetTexture(const unsigned int &i, const int &stage) const
	{
		return Renderer::Get()->GetRenderStage(stage == -1 ? m_graphicsStage.renderpass : stage)->m_framebuffers->GetTexture(i);
	}

	void Pipeline::CreateDescriptorLayout()
	{
		const auto logicalDevice = Display::Get()->GetLogicalDevice();

		std::vector<VkDescriptorSetLayoutBinding> bindings = std::vector<VkDescriptorSetLayoutBinding>();

		for (auto type : m_pipelineCreateInfo.descriptors)
		{
			bindings.push_back(type.m_descriptorSetLayoutBinding);
		}

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
		descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		descriptorSetLayoutCreateInfo.pBindings = bindings.data();

		vkDeviceWaitIdle(logicalDevice);
		Platform::ErrorVk(vkCreateDescriptorSetLayout(logicalDevice, &descriptorSetLayoutCreateInfo, nullptr, &m_descriptorSetLayout));
	}

	void Pipeline::CreateDescriptorPool()
	{
		const auto logicalDevice = Display::Get()->GetLogicalDevice();

		std::vector<VkDescriptorPoolSize> poolSizes = std::vector<VkDescriptorPoolSize>();

		for (auto type : m_pipelineCreateInfo.descriptors)
		{
			poolSizes.push_back(type.m_descriptorPoolSize);
		}

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
		descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();
		descriptorPoolCreateInfo.maxSets = 16384;

		vkDeviceWaitIdle(logicalDevice);
		Platform::ErrorVk(vkCreateDescriptorPool(logicalDevice, &descriptorPoolCreateInfo, nullptr, &m_descriptorPool));
	}

	void Pipeline::CreatePipelineLayout()
	{
		const auto logicalDevice = Display::Get()->GetLogicalDevice();

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &m_descriptorSetLayout;

		vkDeviceWaitIdle(logicalDevice);
		Platform::ErrorVk(vkCreatePipelineLayout(logicalDevice, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout));
	}

	void Pipeline::CreateShaderProgram()
	{
		const auto logicalDevice = Display::Get()->GetLogicalDevice();

		std::vector<glslang::TShader> shaders;
		glslang::TProgram program;
		EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

		for (auto &type : m_pipelineCreateInfo.shaderStages)
		{
			auto shaderCode = FileSystem::ReadTextFile(type);

			VkShaderStageFlagBits stageFlag = GetShaderStage(type);
			EShLanguage language = GetEshLanguage(stageFlag);

			glslang::TShader shader = glslang::TShader(language);
			const char *shaderStrings[1];
			shaderStrings[0] = shaderCode.c_str();
			shader.setStrings(shaderStrings, 1);

			printf("%i\n", stageFlag);

			if (!shader.parse(&DefaultTBuiltInResource, 100, false, messages))
			{
				printf("SPRIV shader compile failed!\n");
				printf("%s\n", shader.getInfoLog());
				printf("%s\n", shader.getInfoDebugLog());
			}

			shaders.push_back(shader);
			program.addShader(&shader);

			if (!program.link(messages))
			{
				printf("Error while linking shader program.\n");
			}
		}

		for (auto &type : m_pipelineCreateInfo.shaderStages)
		{
			VkShaderStageFlagBits stageFlag = GetShaderStage(type);

			EShLanguage stage = EShLanguage::EShLangVertex;
			std::vector<uint32_t> spirv = {};
			glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);

			printf("Size: %i\n", spirv.size());

			VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
			shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shaderModuleCreateInfo.codeSize = spirv.size();
			shaderModuleCreateInfo.pCode = spirv.data();

			VkShaderModule shaderModule = VK_NULL_HANDLE;
			Platform::ErrorVk(vkCreateShaderModule(logicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule));

			VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo = {};
			pipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			pipelineShaderStageCreateInfo.stage = stageFlag;
			pipelineShaderStageCreateInfo.module = shaderModule;
			pipelineShaderStageCreateInfo.pName = "main";

			m_modules.push_back(shaderModule);
			m_stages.push_back(pipelineShaderStageCreateInfo);
		}
	}

	VkShaderStageFlagBits Pipeline::GetShaderStage(const std::string &filename)
	{
		if (FormatString::Contains(filename, ".comp"))
		{
			return VK_SHADER_STAGE_COMPUTE_BIT;
		}
		else if (FormatString::Contains(filename, ".vert"))
		{
			return VK_SHADER_STAGE_VERTEX_BIT;
		}
		else if (FormatString::Contains(filename, ".tesc"))
		{
			return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		}
		else if (FormatString::Contains(filename, ".tese"))
		{
			return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		}
		else if (FormatString::Contains(filename, ".geom"))
		{
			return VK_SHADER_STAGE_GEOMETRY_BIT;
		}
		else if (FormatString::Contains(filename, ".frag"))
		{
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		}

		return VK_SHADER_STAGE_ALL;
	}

	EShLanguage Pipeline::GetEshLanguage(const VkShaderStageFlagBits &stageFlag)
	{
		switch(stageFlag)
		{
		case VK_SHADER_STAGE_COMPUTE_BIT:
			return EShLangCompute;
		case VK_SHADER_STAGE_VERTEX_BIT:
			return EShLangVertex;
		case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
			return EShLangTessControl;
		case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
			return EShLangTessEvaluation;
		case VK_SHADER_STAGE_GEOMETRY_BIT:
			return EShLangGeometry;
		case VK_SHADER_STAGE_FRAGMENT_BIT:
			return EShLangFragment;
		default:
			return EShLangCount;
		}
	}

	void Pipeline::CreateAttributes()
	{
		m_inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		m_inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		m_inputAssemblyState.primitiveRestartEnable = VK_FALSE;

		m_rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		m_rasterizationState.depthClampEnable = VK_FALSE;
		m_rasterizationState.rasterizerDiscardEnable = VK_FALSE;
		m_rasterizationState.polygonMode = m_pipelineCreateInfo.polygonMode;
		m_rasterizationState.cullMode = m_pipelineCreateInfo.cullModeFlags;
		m_rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		m_rasterizationState.depthBiasEnable = VK_FALSE;
		m_rasterizationState.depthBiasConstantFactor = 0.0f;
		m_rasterizationState.depthBiasClamp = 0.0f;
		m_rasterizationState.depthBiasSlopeFactor = 0.0f;
		m_rasterizationState.lineWidth = 1.0f;

		m_blendAttachmentStates[0].blendEnable = VK_TRUE;
		m_blendAttachmentStates[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		m_blendAttachmentStates[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		m_blendAttachmentStates[0].colorBlendOp = VK_BLEND_OP_ADD;
		m_blendAttachmentStates[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		m_blendAttachmentStates[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		m_blendAttachmentStates[0].alphaBlendOp = VK_BLEND_OP_ADD;
		m_blendAttachmentStates[0].colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		m_colourBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		m_colourBlendState.logicOpEnable = VK_FALSE;
		m_colourBlendState.logicOp = VK_LOGIC_OP_COPY;
		m_colourBlendState.attachmentCount = static_cast<uint32_t>(m_blendAttachmentStates.size());
		m_colourBlendState.pAttachments = m_blendAttachmentStates.data();
		m_colourBlendState.blendConstants[0] = 0.0f;
		m_colourBlendState.blendConstants[1] = 0.0f;
		m_colourBlendState.blendConstants[2] = 0.0f;
		m_colourBlendState.blendConstants[3] = 0.0f;

		VkStencilOpState stencilOpState = {};
		stencilOpState.failOp = VK_STENCIL_OP_KEEP;
		stencilOpState.passOp = VK_STENCIL_OP_KEEP;
		stencilOpState.depthFailOp = VK_STENCIL_OP_KEEP;
		stencilOpState.compareOp = VK_COMPARE_OP_ALWAYS;
		stencilOpState.compareMask = 0b00000000;
		stencilOpState.writeMask = 0b11111111;
		stencilOpState.reference = 0b00000000;

		m_depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		m_depthStencilState.depthTestEnable = VK_TRUE;
		m_depthStencilState.depthWriteEnable = VK_TRUE;
		m_depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
		m_depthStencilState.depthBoundsTestEnable = VK_FALSE;
		m_depthStencilState.stencilTestEnable = VK_FALSE;
		m_depthStencilState.front = stencilOpState;
		m_depthStencilState.back = stencilOpState;
		m_depthStencilState.minDepthBounds = 0.0f;
		m_depthStencilState.maxDepthBounds = 1.0f;

		m_viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		m_viewportState.viewportCount = 1;
		m_viewportState.scissorCount = 1;

		m_multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		m_multisampleState.sampleShadingEnable = VK_FALSE;
		m_multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		m_multisampleState.minSampleShading = 0.0f;

		m_dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		m_dynamicState.pDynamicStates = DYNAMIC_STATES.data();
		m_dynamicState.dynamicStateCount = static_cast<uint32_t>(DYNAMIC_STATES.size());
	}

	void Pipeline::CreatePipelinePolygon()
	{
		const auto logicalDevice = Display::Get()->GetLogicalDevice();
		const auto pipelineCache = Renderer::Get()->GetPipelineCache();
		const auto renderStage = Renderer::Get()->GetRenderStage(m_graphicsStage.renderpass);

		VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
		vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputStateCreateInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(m_pipelineCreateInfo.vertexBindingDescriptions.size());
		vertexInputStateCreateInfo.pVertexBindingDescriptions = m_pipelineCreateInfo.vertexBindingDescriptions.data();
		vertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_pipelineCreateInfo.vertexAttributeDescriptions.size());
		vertexInputStateCreateInfo.pVertexAttributeDescriptions = m_pipelineCreateInfo.vertexAttributeDescriptions.data();

		VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.layout = m_pipelineLayout;
		pipelineCreateInfo.renderPass = renderStage->m_renderpass->GetRenderpass();
		pipelineCreateInfo.subpass = m_graphicsStage.subpass;
		pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineCreateInfo.basePipelineIndex = -1;

		pipelineCreateInfo.pInputAssemblyState = &m_inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &m_rasterizationState;
		pipelineCreateInfo.pColorBlendState = &m_colourBlendState;
		pipelineCreateInfo.pMultisampleState = &m_multisampleState;
		pipelineCreateInfo.pViewportState = &m_viewportState;
		pipelineCreateInfo.pDepthStencilState = &m_depthStencilState;
		pipelineCreateInfo.pDynamicState = &m_dynamicState;

		pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
		pipelineCreateInfo.stageCount = static_cast<uint32_t>(m_stages.size());
		pipelineCreateInfo.pStages = m_stages.data();

		// Create the graphics pipeline.
		Platform::ErrorVk(vkCreateGraphicsPipelines(logicalDevice, pipelineCache, 1, &pipelineCreateInfo, nullptr, &m_pipeline));
	}

	void Pipeline::CreatePipelinePolygonNoDepth()
	{
		m_depthStencilState.depthTestEnable = VK_FALSE;
		m_depthStencilState.depthWriteEnable = VK_FALSE;

		CreatePipelinePolygon();
	}

	void Pipeline::CreatePipelineMrt()
	{
		std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates = {};

		for (uint32_t i = 0; i < Renderer::Get()->GetRenderStage(m_graphicsStage.renderpass)->m_imageAttachments; i++)
		{
			VkPipelineColorBlendAttachmentState blendAttachmentState = {};
			blendAttachmentState.blendEnable = VK_FALSE;
			blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
			blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
			blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
				VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			blendAttachmentStates.push_back(blendAttachmentState);
		}

		m_colourBlendState.attachmentCount = static_cast<uint32_t>(blendAttachmentStates.size());
		m_colourBlendState.pAttachments = blendAttachmentStates.data();

		CreatePipelinePolygon();
	}

	void Pipeline::CreatePipelineMrtNoDepth()
	{
		std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates = {};

		for (uint32_t i = 0; i < Renderer::Get()->GetRenderStage(m_graphicsStage.renderpass)->m_imageAttachments; i++)
		{
			VkPipelineColorBlendAttachmentState blendAttachmentState = {};
			blendAttachmentState.blendEnable = VK_FALSE;
			blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
			blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
			blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
				VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			blendAttachmentStates.push_back(blendAttachmentState);
		}

		m_colourBlendState.attachmentCount = static_cast<uint32_t>(blendAttachmentStates.size());
		m_colourBlendState.pAttachments = blendAttachmentStates.data();

		m_depthStencilState.depthTestEnable = VK_FALSE;
		m_depthStencilState.depthWriteEnable = VK_FALSE;

		CreatePipelinePolygon();
	}
}
