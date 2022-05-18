#include "DirectLightingRenderModule.h"

#include "RenderPasses/ResultFrameBufferGenerationRenderPass.h"
#include "RenderPasses/DeferredGeometryRenderPass.h"
#include "RenderPasses/DeferredSkyboxRenderPass.h"
#include "RenderPasses/DeferredBlitRenderPass.h"
#include "RenderPasses/ForwardRenderPass.h"
#include "RenderPasses/GUI/GUIGizmosRenderPass.h"
#include "RenderPasses/GUI/GUIRenderPass.h"
#include "RenderPasses/WindowBlitRenderPass.h"

#include "RenderPasses/Container/ContainerRenderPass.h"
#include "RenderPasses/IterateOverRenderVolumeCollection.h"

#include "RenderPasses/ScreenSpaceAmbientOcclusion/SSAOSamplesGenerationRenderPass.h"
#include "RenderPasses/ScreenSpaceAmbientOcclusion/SSAONoiseGenerationRenderPass.h"
#include "RenderPasses/ScreenSpaceAmbientOcclusion/SSAORenderPass.h"
#include "RenderPasses/ScreenSpaceAmbientOcclusion/SSAOBlurRenderPass.h"
#include "RenderPasses/ScreenSpaceAmbientOcclusion/SSAOTemporalFilterRenderPass.h"

#include "RenderPasses/AmbientLight/AmbientLightRenderPass.h"

#include "RenderPasses/FramebufferMipmapsGenerationRenderPass.h"

#include "RenderPasses/ShadowMap/DirectionalLightShadowMapRenderPass.h"
#include "RenderPasses/DeferredDirectionalLightRenderPass.h"
#include "RenderPasses/DirectionalLightContainerRenderVolumeCollection.h"

#include "RenderPasses/FramebufferGenerationRenderPass.h"

#include "RenderPasses/IdleRenderPass.h"
#include "RenderPasses/TemporalAntialiasing/TAARenderPass.h"
#include "RenderPasses/VolumetricLighting/VolumetricLightingDirectionalRenderPass.h"
#include "RenderPasses/VolumetricLighting/VolumetricLightingRenderPass.h"
#include "RenderPasses/LightShafts/DirectionalLightSourceRenderPass.h"
#include "RenderPasses/LightShafts/LightShaftsRenderPass.h"
#include "RenderPasses/LightShafts/LightShaftsAccumulationRenderPass.h"
#include "RenderPasses/Bloom/BrightExtractionRenderPass.h"
#include "RenderPasses/Bloom/BloomHorizontalBlurRenderPass.h"
#include "RenderPasses/Bloom/BloomVerticalBlurRenderPass.h"
#include "RenderPasses/Bloom/BloomAccumulationRenderPass.h"
#include "RenderPasses/HighDynamicRange/HDRRenderPass.h"
#include "RenderPasses/TextureLUT/TextureLUTRenderPass.h"
#include "RenderPasses/GammaCorrection/GammaCorrectionRenderPass.h"

void DirectLightingRenderModule::Init ()
{
	_renderPasses.push_back (new ResultFrameBufferGenerationRenderPass ());
	_renderPasses.push_back (new DeferredGeometryRenderPass ());
	_renderPasses.push_back (new FramebufferGenerationRenderPass ("volumetricLightMap"));
	_renderPasses.push_back (new FramebufferGenerationRenderPass ("lightSourceMap"));
	_renderPasses.push_back (new FramebufferGenerationRenderPass ("lightShaftsMap"));
	_renderPasses.push_back (ContainerRenderPass::Builder ()
		.Volume (new IterateOverRenderVolumeCollection (1))
		.Attach (new SSAOSamplesGenerationRenderPass ())
		.Attach (new SSAONoiseGenerationRenderPass ())
		.Attach (new SSAORenderPass ())
		.Attach (new SSAOBlurRenderPass ())
		.Attach (new SSAOTemporalFilterRenderPass ())
		.Build ());
	_renderPasses.push_back (new AmbientLightRenderPass ());
	_renderPasses.push_back (ContainerRenderPass::Builder ()
		.Volume (new DirectionalLightContainerRenderVolumeCollection ())
		.Attach (new DirectionalLightShadowMapRenderPass ())
		.Attach (new DeferredDirectionalLightRenderPass ())
		.Attach (new VolumetricLightingDirectionalRenderPass ())
		.Attach (new DirectionalLightSourceRenderPass ())
		.Attach (new LightShaftsRenderPass ())
		.Build ());
	_renderPasses.push_back (new DeferredSkyboxRenderPass ());

	/*
	 * Post processing passes
	*/

	_renderPasses.push_back (ContainerRenderPass::Builder ()
		.Volume (new IterateOverRenderVolumeCollection (1))
		.Attach (new IdleRenderPass ())
		.Attach	(new TAARenderPass ())
		.Attach (new VolumetricLightingRenderPass ())
		.Attach (new LightShaftsAccumulationRenderPass ())
		.Attach (ContainerRenderPass::Builder ()
			.Volume (new IterateOverRenderVolumeCollection (1))
			.Attach (new BrightExtractionRenderPass ())
			.Attach (ContainerRenderPass::Builder ()
				.Volume (new IterateOverRenderVolumeCollection (5))
				.Attach (new BloomHorizontalBlurRenderPass ())
				.Attach (new BloomVerticalBlurRenderPass ())
				.Build ())
			.Attach (new BloomAccumulationRenderPass ())
			.Build ())
		.Attach (new HDRRenderPass ())
		.Attach (new TextureLUTRenderPass ())
		.Attach (new GammaCorrectionRenderPass ())
		.Attach (new DeferredBlitRenderPass())
		.Build ());
	_renderPasses.push_back (new ForwardRenderPass ());
	_renderPasses.push_back (new GUIGizmosRenderPass ());
}
