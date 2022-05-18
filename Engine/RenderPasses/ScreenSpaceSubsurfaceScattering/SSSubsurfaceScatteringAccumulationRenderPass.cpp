#include "SSSubsurfaceScatteringAccumulationRenderPass.h"

bool SSSubsurfaceScatteringAccumulationRenderPass::IsAvailable (const RenderScene* renderScene, const Camera* camera,
	const RenderSettings& settings, const RenderVolumeCollection* rvc) const
{
	/*
	 * Check if screen space ambient occlusion is enabled
	*/

	return settings.subsurface_scattering_enabled;
}

std::string SSSubsurfaceScatteringAccumulationRenderPass::GetPostProcessFragmentShaderPath () const
{
	return "Assets/Shaders/ScreenSpaceSubsurfaceScattering/screenSpaceSubsurfaceScatteringAccumulationFragment.glsl";
}

std::string SSSubsurfaceScatteringAccumulationRenderPass::GetPostProcessVolumeName () const
{
	return "PostProcessMapVolume";
}

glm::ivec2 SSSubsurfaceScatteringAccumulationRenderPass::GetPostProcessVolumeResolution (const RenderSettings& settings) const
{
	return glm::ivec2 (settings.resolution.width, settings.resolution.height);
}

FramebufferRenderVolume* SSSubsurfaceScatteringAccumulationRenderPass::CreatePostProcessVolume (const RenderSettings& settings) const
{
	Resource<Texture> texture = Resource<Texture> (new Texture ("postProcessMap"));

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

	return renderVolume;
}

std::vector<PipelineAttribute> SSSubsurfaceScatteringAccumulationRenderPass::GetCustomAttributes (const Camera* camera,
	const RenderSettings& settings, RenderVolumeCollection* rvc)
{
	/*
	 * Attach post process volume attributes to pipeline
	*/

	std::vector<PipelineAttribute> attributes = PostProcessRenderPass::GetCustomAttributes (camera, settings, rvc);

	/*
	 * Attach screen space ambient occlusion attributes to pipeline
	*/

	PipelineAttribute sstIntensity;

	sstIntensity.type = PipelineAttribute::AttrType::ATTR_1F;

	sstIntensity.name = "sstIntensity";

	sstIntensity.value.x = settings.sst_intensity;

	attributes.push_back (sstIntensity);

	return attributes;
}
