#include "LPVEmissiveRadianceInjectionRenderPass.h"

#include "LPVVolume.h"
#include "LPVGeometryVolume.h"
#include "LPVSampleCountVolume.h"

#include "Resources/Resources.h"
#include "Renderer/RenderSystem.h"

#include "Renderer/Pipeline.h"

#include "Wrappers/OpenGL/GL.h"

#include "Core/Console/Console.h"

#include "SceneNodes/SceneLayer.h"

void LPVEmissiveRadianceInjectionRenderPass::Init (const RenderSettings& settings)
{
	/*
	 * Shader for static objects
	*/

	Resource<Shader> staticShader = Resources::LoadShader ({
		"Assets/Shaders/Voxelize/voxelizeVertex.glsl",
		"Assets/Shaders/LightPropagationVolumes/lightPropagationVolumesEmissiveRadianceInjectionFragment.glsl",
		"Assets/Shaders/LightPropagationVolumes/lightPropagationVolumesEmissiveRadianceInjectionGeometry.glsl"
	});

	_staticShaderView = RenderSystem::LoadShader (staticShader);

	/*
	 * Shader for animated objects
	*/

	Resource<Shader> animationShader = Resources::LoadShader ({
		"Assets/Shaders/Voxelize/voxelizeAnimationVertex.glsl",
		"Assets/Shaders/LightPropagationVolumes/lightPropagationVolumesEmissiveRadianceInjectionFragment.glsl",
		"Assets/Shaders/LightPropagationVolumes/lightPropagationVolumesEmissiveRadianceInjectionGeometry.glsl"
	});

	_animationShaderView = RenderSystem::LoadShader (animationShader);
}

RenderVolumeCollection* LPVEmissiveRadianceInjectionRenderPass::Execute (const RenderScene* renderScene, const Camera* camera,
	const RenderSettings& settings, RenderVolumeCollection* rvc)
{
	/*
	* Voxelization start
	*/

	StartVoxelization ();

	/*
	* Voxelization: voxelize geomtry
	*/

	GeometryVoxelizationPass (renderScene, settings, rvc);

	/*
	* Clear opengl state after voxelization
	*/

	EndVoxelization ();

	return rvc;
}

void LPVEmissiveRadianceInjectionRenderPass::Clear ()
{
	/*
	 * Nothing
	*/
}

bool LPVEmissiveRadianceInjectionRenderPass::IsAvailable (const RenderScene* renderScene, const Camera* camera,
	const RenderSettings& settings, const RenderVolumeCollection* rvc) const
{
	/*
	 * Always execute light propagation volumes emissive radiance injection render pass
	*/

	return settings.lpv_emissive_voxelization;
}

void LPVEmissiveRadianceInjectionRenderPass::StartVoxelization ()
{
	/*
	* Render to window but mask out all color.
	*/

	GL::Disable (GL_CULL_FACE);

	GL::ColorMask (GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	GL::DepthMask (GL_FALSE);
}

void LPVEmissiveRadianceInjectionRenderPass::GeometryVoxelizationPass (const RenderScene* renderScene,
	const RenderSettings& settings, RenderVolumeCollection* rvc)
{
	auto lpvVolume = (LPVVolume*) rvc->GetRenderVolume ("LightPropagationVolume");
	auto lpvGeometryVolume = (LPVGeometryVolume*) rvc->GetRenderVolume ("LPVGeometryVolume");
	auto lpvSampleCountVolume = (LPVSampleCountVolume*) rvc->GetRenderVolume ("LPVSampleCountVolume");

	/*
	 * Set viewport
	*/

	std::size_t volumeSize = ((LPVVolume*) lpvVolume)->GetVolumeSize ();

	GL::Viewport (0, 0, volumeSize, volumeSize);

	/*
	* Bind voxel volume to geometry render pass
	*/

	lpvVolume->BindForWriting ();
	lpvGeometryVolume->BindForWriting ();
	lpvSampleCountVolume->BindForWriting ();

	/*
	* Render geometry
	*/

	for_each_type (RenderObject*, renderObject, *renderScene) {

		/*
		 * Check if it's active
		*/

		if (renderObject->IsActive () == false) {
			continue;
		}

		if (renderObject->GetRenderStage () != RenderStage::RENDER_STAGE_DEFERRED) {
			continue;
		}

		/*
		 * Check if the object is emissive
		*/

		auto modelView = renderObject->GetModelView ();

		bool isEmissive = false;

		for (auto& groupObject : *modelView) {

			// TODO: Fix this
			glm::vec3 emissiveColor = groupObject.materialView == nullptr ? glm::vec3 (0) :
				groupObject.materialView->emissiveColor;
			if (glm::dot (emissiveColor, emissiveColor) > 0.0f) {
				isEmissive = true;
				break;
			}
		}

		if (isEmissive == false) {
			continue;
		}

		/*
		 * Lock voxelization shader for geomtry rendering
		*/

		LockShader (renderObject->GetSceneLayers ());

		/*
		 * Send voxel volume attributes to pipeline
		*/

		Pipeline::SendCustomAttributes (nullptr, lpvVolume->GetCustomAttributes ());

		/*
		 * Send custom attributes to pipeline
		*/

		Pipeline::SendCustomAttributes (nullptr, GetCustomAttributes (settings));

		/*
		 * Voxelize object
		*/

		renderObject->Draw ();
	}
}

void LPVEmissiveRadianceInjectionRenderPass::EndVoxelization ()
{
	GL::MemoryBarrier(GL_ALL_BARRIER_BITS);

	/*
	* Clear settings
	*/

	GL::ColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	GL::DepthMask (GL_TRUE);

	/*
	* Unlock voxelization shader
	*/

	Pipeline::UnlockShader ();
}

void LPVEmissiveRadianceInjectionRenderPass::LockShader (int sceneLayers)
{
	/*
	 * Unlock last shader
	*/

	Pipeline::UnlockShader ();

	/*
	 * Lock the shader for animations
	*/

	if (sceneLayers & SceneLayer::ANIMATION) {
		Pipeline::LockShader (_animationShaderView);
	}

	/*
	 * Lock general shader for not animated objects
	*/

	if (sceneLayers & (SceneLayer::STATIC | SceneLayer::DYNAMIC)) {
		Pipeline::LockShader (_staticShaderView);
	}
}

std::vector<PipelineAttribute> LPVEmissiveRadianceInjectionRenderPass::GetCustomAttributes (const RenderSettings& settings)
{
	std::vector<PipelineAttribute> attributes;

	/*
	 * Attach bloom attributes to pipeline
	*/

	PipelineAttribute normalAngleStep;
	PipelineAttribute emissiveTextured;

	normalAngleStep.type = PipelineAttribute::AttrType::ATTR_1F;
	emissiveTextured.type = PipelineAttribute::AttrType::ATTR_1I;

	normalAngleStep.name = "normalAngleStep";
	emissiveTextured.name = "emissiveTextured";

	normalAngleStep.value.x = settings.lpv_emissive_normal_angle_step;
	emissiveTextured.value.x = settings.lpv_emissive_textured;

	attributes.push_back (normalAngleStep);
	attributes.push_back (emissiveTextured);

	return attributes;
}
