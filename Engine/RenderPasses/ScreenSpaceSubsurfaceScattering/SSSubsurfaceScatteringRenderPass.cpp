#include "SSSubsurfaceScatteringRenderPass.h"

#include "Debug/Statistics/StatisticsManager.h"
#include "SSSubsurfaceScatteringStatisticsObject.h"

bool SSSubsurfaceScatteringRenderPass::IsAvailable (const RenderScene* renderScene, const Camera* camera,
	const RenderSettings& settings, const RenderVolumeCollection* rvc) const
{
	/*
	 * Check if screen space reflection is enabled
	*/

	return settings.subsurface_scattering_enabled;
}

std::string SSSubsurfaceScatteringRenderPass::GetPostProcessFragmentShaderPath () const
{
	return "Assets/Shaders/ScreenSpaceSubsurfaceScattering/screenSpaceSubsurfaceScatteringFragment.glsl";
}

std::string SSSubsurfaceScatteringRenderPass::GetPostProcessVolumeName () const
{
	return "SubsurfaceScatteringMapVolume";
}

glm::ivec2 SSSubsurfaceScatteringRenderPass::GetPostProcessVolumeResolution (const RenderSettings& settings) const
{
	return glm::ivec2 (glm::vec2 (settings.resolution.width, settings.resolution.height) * settings.ssr_scale);
}

FramebufferRenderVolume* SSSubsurfaceScatteringRenderPass::CreatePostProcessVolume (const RenderSettings& settings) const
{
	Resource<Texture> texture = Resource<Texture> (new Texture ("subsurfaceScatteringMap"));

	glm::ivec2 size = GetPostProcessVolumeResolution (settings);

	texture->SetSize (Size (size.x, size.y));
	texture->SetMipmapGeneration (false);
	texture->SetSizedInternalFormat (TEXTURE_SIZED_INTERNAL_FORMAT::FORMAT_RGB16);
	texture->SetInternalFormat (TEXTURE_INTERNAL_FORMAT::FORMAT_RGB);
	texture->SetChannelType (TEXTURE_CHANNEL_TYPE::CHANNEL_FLOAT);
	texture->SetWrapMode (TEXTURE_WRAP_MODE::WRAP_CLAMP_EDGE);
	texture->SetMinFilter (TEXTURE_FILTER_MODE::FILTER_NEAREST);
	texture->SetMagFilter (TEXTURE_FILTER_MODE::FILTER_NEAREST);
	texture->SetAnisotropicFiltering (false);

	Resource<Framebuffer> framebuffer = Resource<Framebuffer> (new Framebuffer (texture));

	FramebufferRenderVolume* renderVolume = new FramebufferRenderVolume (framebuffer);

	/*
	 * Update statistics object
	*/

	auto ssSubsurfaceScatteringStatisticsObject = StatisticsManager::Instance ()->GetStatisticsObject <SSSubsurfaceScatteringStatisticsObject> ();

	ssSubsurfaceScatteringStatisticsObject->ssrMapVolume = renderVolume;

	return renderVolume;
}

std::vector<PipelineAttribute> SSSubsurfaceScatteringRenderPass::GetCustomAttributes (const Camera* camera,
	const RenderSettings& settings, RenderVolumeCollection* rvc)
{
	/*
	 * Attach post process volume attributes to pipeline
	*/

	std::vector<PipelineAttribute> attributes = PostProcessRenderPass::GetCustomAttributes (camera, settings, rvc);

	/*
	 * Attach screen space ambient occlusion attributes to pipeline
	*/

	PipelineAttribute sstResolution;
	PipelineAttribute sstIterations;
	PipelineAttribute sstRoughness;
	PipelineAttribute sstThickness;
	PipelineAttribute sstStride;
	PipelineAttribute sstIntensity;

	sstResolution.type = PipelineAttribute::AttrType::ATTR_2F;
	sstIterations.type = PipelineAttribute::AttrType::ATTR_1I;
	sstRoughness.type = PipelineAttribute::AttrType::ATTR_1F;
	sstThickness.type = PipelineAttribute::AttrType::ATTR_1F;
	sstStride.type = PipelineAttribute::AttrType::ATTR_1I;
	sstIntensity.type = PipelineAttribute::AttrType::ATTR_1F;

	sstResolution.name = "sstResolution";
	sstIterations.name = "sstIterations";
	sstRoughness.name = "sstRoughness";
	sstThickness.name = "sstThickness";
	sstStride.name = "sstStride";
	sstIntensity.name = "sstIntensity";

	glm::ivec2 resolution = glm::ivec2 (glm::vec2 (settings.resolution.width, settings.resolution.height) * settings.ssr_scale);

	sstResolution.value = glm::vec3 (resolution, 0.0f);
	sstIterations.value.x = settings.sst_iterations;
	sstRoughness.value.x = settings.ssr_roughness;
	sstThickness.value.x = settings.sst_thickness;
	sstStride.value.x = settings.ssr_stride;
	sstIntensity.value.x = settings.sst_intensity;

	attributes.push_back (sstResolution);
	attributes.push_back (sstIterations);
	attributes.push_back (sstRoughness);
	attributes.push_back (sstThickness);
	attributes.push_back (sstStride);
	attributes.push_back (sstIntensity);

	return attributes;
}
