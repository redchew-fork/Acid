#include "RendererShadows.hpp"

#include "../Devices/Display.hpp"
#include "../Meshes/Mesh.hpp"
#include "../Scenes/Scenes.hpp"
#include "ShadowRender.hpp"
#include "UbosShadows.hpp"

namespace Flounder
{
	const PipelineCreate PIPELINE_CREATE =
		{
			PIPELINE_POLYGON_NO_DEPTH, // pipelineModeFlags
			VK_POLYGON_MODE_FILL, // polygonMode
			VK_CULL_MODE_FRONT_BIT, // cullModeFlags

			VertexModel::GetBindingDescriptions(), // vertexBindingDescriptions
			VertexModel::GetAttributeDescriptions(0), // vertexAttributeDescriptions

			{
				UniformBuffer::CreateDescriptor(0, VK_SHADER_STAGE_ALL), // uboScene
				UniformBuffer::CreateDescriptor(1, VK_SHADER_STAGE_ALL) // uboObject
			}, // descriptors

			{"Resources/Shaders/Shadows/Shadow.vert", "Resources/Shaders/Shadows/Shadow.frag"} // shaderStages
		};

	RendererShadows::RendererShadows(const GraphicsStage &graphicsStage) :
		IRenderer(),
		m_uniformScene(new UniformBuffer(sizeof(UbosShadows::UboScene))),
		m_pipeline(new Pipeline(graphicsStage, PIPELINE_CREATE))
	{
	}

	RendererShadows::~RendererShadows()
	{
		delete m_uniformScene;
		delete m_pipeline;
	}

	void RendererShadows::Render(const VkCommandBuffer &commandBuffer, const Vector4 &clipPlane, const ICamera &camera)
	{
		UbosShadows::UboScene uboScene = {};
		uboScene.projectionView = *Shadows::Get()->GetShadowBox()->GetProjectionViewMatrix();
		uboScene.cameraPosition = *Scenes::Get()->GetCamera()->GetPosition();
		m_uniformScene->Update(&uboScene);

		m_pipeline->BindPipeline(commandBuffer);

		std::vector<ShadowRender *> renderList = std::vector<ShadowRender *>();
		Scenes::Get()->GetStructure()->QueryComponents<ShadowRender>(&renderList);

		for (auto shadowRender : renderList)
		{
			shadowRender->CmdRender(commandBuffer, *m_pipeline, m_uniformScene);
		}
	}
}
