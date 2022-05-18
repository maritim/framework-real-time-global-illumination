#include "EditorRenderingSettings.h"

#include <filesystem>
#include <glm/vec2.hpp>
#include <ImGui/imgui.h>

#include "Systems/Window/Window.h"
#include "Systems/Input/Input.h"
#include "Systems/Settings/SettingsManager.h"

#include "Managers/RenderSettingsManager.h"

#include "Resources/Resources.h"

#include "Renderer/RenderSystem.h"
#include "Renderer/RenderManager.h"

#include "RenderPasses/FramebufferRenderVolume.h"

#include "Utils/Files/FileSystem.h"

#include "Utils/Extensions/MathExtend.h"

#include "Debug/Statistics/StatisticsManager.h"
#include "RenderPasses/DeferredStatisticsObject.h"
#include "RenderPasses/ReflectiveShadowMapping/RSMStatisticsObject.h"
#include "RenderPasses/TemporalReflectiveShadowMapping/TRSMStatisticsObject.h"
#include "RenderPasses/LightPropagationVolumes/LPVStatisticsObject.h"
#include "RenderPasses/VoxelConeTracing/VCTStatisticsObject.h"
#include "RenderPasses/ScreenSpaceAmbientOcclusion/SSAOStatisticsObject.h"
#include "RenderPasses/ScreenSpaceDirectionalOcclusion/SSDOStatisticsObject.h"
#include "RenderPasses/ScreenSpaceReflections/SSRStatisticsObject.h"
#include "RenderPasses/ScreenSpaceSubsurfaceScattering/SSSubsurfaceScatteringStatisticsObject.h"
#include "RenderPasses/HybridGlobalIllumination/HGIStatisticsObject.h"
#include "RenderPasses/TemporalAntialiasing/TAAStatisticsObject.h"
#include "RenderPasses/VolumetricLighting/VolLightingStatisticsObject.h"
#include "RenderPasses/LightShafts/LightShaftsStatisticsObject.h"

namespace fs = std::filesystem;

EditorRenderingSettings::EditorRenderingSettings () :
	_lutTexture (nullptr),
	_lutTextureView (nullptr),
	_continuousVoxelizationReset (false),
	_dialog (ImGuiFs::Dialog ())
{

}

void EditorRenderingSettings::Show ()
{
	bool visible = SettingsManager::Instance ()->GetValue<bool> ("Menu", "show_rendering_settings", false);

	if (visible) {
		std::string path = SettingsManager::Instance ()->GetValue<std::string> ("Menu", "rendering_settings_path", "");

		if (path == std::string ()) {
			return;
		}

		_settings = RenderSettingsManager::Instance ()->GetRenderSettings (path);

		if (_settings == nullptr) {
			_settings = Resources::LoadRenderSettings (path);
			RenderSettingsManager::Instance ()->AddRenderSettings (_settings);
		}

		if (_settings == nullptr) {
			return;
		}

		ShowRenderingSettingsWindow ();
	}
}

void EditorRenderingSettings::ShowRenderingSettingsWindow ()
{
	std::size_t windowHeight = Window::GetHeight ();

	ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(350, windowHeight - 30), ImGuiCond_FirstUseEver);

	ImGuiWindowFlags windowFlags = 0;

	if (!ImGui::Begin("Rendering Settings", NULL, windowFlags)) {
		ImGui::End();

		return;
	}

	ImGui::PushItemWidth(ImGui::GetFontSize() * -12);

	std::map<std::string, int> renderModes;
	renderModes ["DirectLightingRenderModule"] = 0;
	renderModes ["ReflectiveShadowMappingRenderModule"] = 1;
	renderModes ["LightPropagationVolumesRenderModule"] = 2;
	renderModes ["VoxelConeTracingRenderModule"] = 3;
	renderModes ["ScreenSpaceGlobalIlluminationRenderModule"] = 4;

	int lastRenderMode = renderModes [_settings->renderMode];
	int renderMode = lastRenderMode;

	const char* items[] = { "Direct Illumiantion",
		"Reflective Shadow Maps",
		"Light Propagation Volumes",
		"Voxel Cone Tracing",
		"Screen Space Global Illumination"
	};

	ImGui::Combo("Render Module", &renderMode, items, 5);

	const char* srenderModes[] = {
		"DirectLightingRenderModule",
		"ReflectiveShadowMappingRenderModule",
		"LightPropagationVolumesRenderModule",
		"VoxelConeTracingRenderModule",
		"ScreenSpaceGlobalIlluminationRenderModule"
	};

	if (lastRenderMode != renderMode) {
		_settings->renderMode = srenderModes [renderMode];
	}

	ImGui::Separator ();

	ImGui::Checkbox ("Enable Indirect Diffuse", &_settings->indirect_diffuse_enabled);
	ImGui::Checkbox ("Enable Glossy Reflections", &_settings->indirect_specular_enabled);
	ImGui::Checkbox ("Enable Subsurface Scattering", &_settings->subsurface_scattering_enabled);

	ImGui::PushID ("DeferredDebug");

	if (ImGui::TreeNode ("Debug")) {

		auto deferredStat = StatisticsManager::Instance ()->GetStatisticsObject <DeferredStatisticsObject> ();

		if (deferredStat->geometryBuffer != nullptr) {

			FramebufferRenderVolume* geometryBuffer = deferredStat->geometryBuffer;

			auto geometryBufferSize = geometryBuffer->GetFramebuffer ()->GetTexture (0)->GetSize ();

			int windowWidth = ImGui::GetContentRegionAvail().x * 0.95f;

			int geometryBufferWidth = windowWidth;
			int geometryBufferHeight = ((float) geometryBufferSize.height / geometryBufferSize.width) * geometryBufferWidth;

			glm::ivec2 geometryBufferWindowSize = glm::ivec2 (geometryBufferWidth, geometryBufferHeight);

			ImGui::Text ("Depth Map");
			ShowImage (geometryBuffer->GetFramebufferView ()->GetDepthTextureView ()->GetGPUIndex (), geometryBufferWindowSize);

			ImGui::Text ("Position Map");
			ShowImage (geometryBuffer->GetFramebufferView ()->GetTextureView (0)->GetGPUIndex (), geometryBufferWindowSize);

			ImGui::Text ("Normal Map");
			ShowImage (geometryBuffer->GetFramebufferView ()->GetTextureView (1)->GetGPUIndex (), geometryBufferWindowSize);

			ImGui::Text ("Diffuse Map");
			ShowImage (geometryBuffer->GetFramebufferView ()->GetTextureView (2)->GetGPUIndex (), geometryBufferWindowSize);

			ImGui::Text ("Specular Map");
			ShowImage (geometryBuffer->GetFramebufferView ()->GetTextureView (3)->GetGPUIndex (), geometryBufferWindowSize);

			ImGui::Text ("Emissive Map");
			ShowImage (geometryBuffer->GetFramebufferView ()->GetTextureView (4)->GetGPUIndex (), geometryBufferWindowSize);
		}

		ImGui::TreePop();
	}

	ImGui::PopID ();

	ImGui::Separator ();

    // ImGui::Spacing();

	if (ImGui::CollapsingHeader ("Reflective Shadow Maps")) {

		auto rsmStat = StatisticsManager::Instance ()->GetStatisticsObject <RSMStatisticsObject> ();

		if (ImGui::TreeNode ("Diffuse Indirect Illumination")) {

			std::size_t limit1 = 1, limit2 = 500;
			ImGui::SliderScalar ("Sample Count", ImGuiDataType_U32, &_settings->rsm_samples, &limit1, &limit2);

			ImGui::SliderFloat ("Sampling Radius", &_settings->rsm_radius, 0.001, 1.0);

			ImGui::InputFloat ("Intensity", &_settings->rsm_indirect_diffuse_intensity, 0.1);

			ImGui::Separator();

			ImGui::Text ("Screen Interpolation");

			ImGui::Checkbox ("Shown Non Interpolated Pixels", &_settings->rsm_debug_interpolation);

			float interpolationScale = _settings->rsm_interpolation_scale;
			ImGui::InputFloat ("Interpolation Scale", &interpolationScale);
			if (interpolationScale > 0) {
				_settings->rsm_interpolation_scale = interpolationScale;
			}

			ImGui::InputFloat ("Min Interpolation Distance", &_settings->rsm_min_interpolation_distance, 0.1);
			ImGui::InputFloat ("Min Interpolation Angle (deg)", &_settings->rsm_min_interpolation_angle, 0.1);

			ImGui::Separator ();

			if (ImGui::TreeNode ("Debug")) {

				ShowImage ("Screen Space Interpolation Diffuse Indirect Illumination Map", rsmStat->rsmInterpolatedIndirectDiffuseMapVolume);

				ShowImage ("Diffuse Indirect Illumination Map", rsmStat->rsmIndirectDiffuseMapVolume);

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode ("Glossy Reflections")) {

			ImGui::InputFloat ("Intensity", &_settings->rsm_indirect_specular_intensity, 0.1);

			ImGui::Separator();

			std::size_t step = 1;
			ImGui::InputScalar ("Iteration Count", ImGuiDataType_U32, &_settings->rsm_reflection_iterations, &step);

			ImGui::InputFloat ("Sample Thickness", &_settings->rsm_reflection_thickness, 0.1);

			ImGui::Separator();

			if (ImGui::TreeNode ("Debug")) {

				ShowImage ("Glossy Reflections Map", rsmStat->rsmIndirectSpecularMapVolume);

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode ("Refraction")) {

			ImGui::InputFloat ("Intensity", &_settings->rsm_indirect_refractive_intensity, 0.1);

			ImGui::Separator();

			std::size_t step = 1;
			ImGui::InputScalar ("Iteration Count", ImGuiDataType_U32, &_settings->rsm_refraction_iterations, &step);

			ImGui::InputFloat ("Sample Thickness", &_settings->rsm_refraction_thickness, 0.1);

			ImGui::Separator();

			if (ImGui::TreeNode ("Debug")) {

				ShowImage ("Subsurface Scattering Map", rsmStat->rsmSubsurfaceScatteringMapVolume);

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode ("Ambient Occlusion")) {

			ImGui::Checkbox ("Enable Ambient Occlusion", &_settings->rsm_ao_enabled);

			ImGui::Separator();

			if (ImGui::TreeNode ("Debug")) {

				ShowImage ("Ambient Occlusion Map", rsmStat->rsmAmbientOcclusionMapVolume);

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		ImGui::Separator();

		ImGui::PushID ("RSMDebug");

		if (ImGui::TreeNode ("Debug")) {

			if (rsmStat->rsmVolume != nullptr) {

				FramebufferRenderVolume* rsmVolume = rsmStat->rsmVolume;

				int windowWidth = ImGui::GetContentRegionAvail().x * 0.95f;

				ShowImage ("Depth Map", rsmVolume->GetFramebufferView ()->GetDepthTextureView ()->GetGPUIndex (), glm::ivec2 (windowWidth, windowWidth));

				ShowImage ("Position Map", rsmVolume->GetFramebufferView ()->GetTextureView (0)->GetGPUIndex (), glm::ivec2 (windowWidth, windowWidth));

				ShowImage ("Normal Map", rsmVolume->GetFramebufferView ()->GetTextureView (1)->GetGPUIndex (), glm::ivec2 (windowWidth, windowWidth));

				ShowImage ("Flux Map", rsmVolume->GetFramebufferView ()->GetTextureView (2)->GetGPUIndex (), glm::ivec2 (windowWidth, windowWidth));
			}

			ImGui::TreePop();
		}

		ImGui::PopID ();
	}

    ImGui::Spacing();

	if (ImGui::CollapsingHeader ("Light Propagation Volumes")) {

		auto lpvStat = StatisticsManager::Instance ()->GetStatisticsObject <LPVStatisticsObject> ();

		ImGui::InputScalar ("LPV Resolution (^3)", ImGuiDataType_U32, &_settings->lpv_volume_size);
		ImGui::InputScalar ("Iteration Count", ImGuiDataType_U32, &_settings->lpv_iterations);

		ImGui::Checkbox ("Use Geometry Occlusion", &_settings->lpv_geometry_occlusion);

		ImGui::Separator ();

		if (ImGui::TreeNode ("Diffuse Indirect Illumination")) {

			ImGui::InputFloat ("Intensity", &_settings->lpv_indirect_diffuse_intensity, 0.1);

			ImGui::Separator();

			if (ImGui::TreeNode ("Debug")) {

				ShowImage ("Diffuse Indirect Illumination Map", lpvStat->lpvIndirectDiffuseMapVolume);

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode ("Glossy Reflections")) {

			ImGui::InputFloat ("Intensity", &_settings->lpv_indirect_specular_intensity, 0.1);

			ImGui::InputScalar ("Iteration Count", ImGuiDataType_U32, &_settings->lpv_reflection_iterations);

			ImGui::Separator();

			if (ImGui::TreeNode ("Debug")) {

				ShowImage ("Glossy Reflections Map", lpvStat->lpvIndirectSpecularMapVolume);

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode ("Refraction")) {

			ImGui::InputFloat ("Intensity", &_settings->lpv_indirect_refractive_intensity, 0.1);

			ImGui::InputScalar ("Iteration Count", ImGuiDataType_U32, &_settings->lpv_refraction_iterations);

			ImGui::Separator();

			if (ImGui::TreeNode ("Debug")) {

				ShowImage ("Subsurface Scattering Map", lpvStat->lpvSubsurfaceScatteringMapVolume);

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode ("Area Lights Direct Illumination")) {

			ImGui::InputScalar ("Sample Count/Triangle", ImGuiDataType_U32, &_settings->lpv_emissive_vpls);

			ImGui::TreePop();
		}
	}

	ImGui::Spacing ();

	if (ImGui::CollapsingHeader ("Voxel Cone Tracing")) {
		if (_continuousVoxelizationReset == true) {
			_settings->vct_continuous_voxelization = _lastContinuousVoxelization;
			_continuousVoxelizationReset = false;
		}

		auto vctStat = StatisticsManager::Instance ()->GetStatisticsObject <VCTStatisticsObject> ();

		std::size_t lastVoxelVolumeSize = _settings->vct_voxels_size;
		bool lastVoxelBordering = _settings->vct_bordering;
		std::size_t lastVolumeMipmapLevels = _settings->vct_mipmap_levels;

		ImGui::InputScalar ("Voxels Resolution (^3)", ImGuiDataType_U32, &_settings->vct_voxels_size);
		ImGui::Checkbox ("Use Continuous Voxelization", &_settings->vct_continuous_voxelization);
		// ImGui::Checkbox ("Voxel Volume Bordering", &_settings->vct_bordering);

		std::size_t speed = 1;
		ImGui::InputScalar ("Volume Mipmap Levels", ImGuiDataType_U32, &_settings->vct_mipmap_levels, &speed);
		_settings->vct_mipmap_levels = Extensions::MathExtend::Clamp (
			_settings->vct_mipmap_levels, (std::size_t) 1,
			(std::size_t) std::log2 (_settings->vct_voxels_size));

		ImGui::Separator ();

		if (ImGui::TreeNode ("Diffuse Indirect Illumination")) {

			ImGui::InputFloat ("Intensity", &_settings->vct_indirect_diffuse_intensity, 0.1f);

	        ImGui::Separator();

			ImGui::SliderFloat ("Cone Distance (Voxels Space)", &_settings->vct_diffuse_cone_distance, 0.0f, 1.0f);

			ImGui::Separator();

			if (ImGui::TreeNode ("Debug")) {

				ShowImage ("Diffuse Indirect Illumination Map", vctStat->vctIndirectDiffuseMapVolume);

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode ("Glossy Reflections")) {

			ImGui::InputFloat ("Intensity", &_settings->vct_indirect_specular_intensity, 0.1f);

			ImGui::Separator();

			ImGui::SliderFloat ("Cone Radius/Height", &_settings->vct_specular_cone_ratio, 0.0f, 1.0f, "%3f", 10.0f);
			ImGui::SliderFloat ("Cone Distance (Voxels Space)", &_settings->vct_specular_cone_distance, 0.0f, 1.0f);

	        ImGui::Separator();

			if (ImGui::TreeNode ("Debug")) {

				ShowImage ("Glossy Reflections Map", vctStat->vctIndirectSpecularMapVolume);

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode ("Subsurface Scattering")) {

			ImGui::InputFloat ("Intensity", &_settings->vct_indirect_refractive_intensity, 0.1f);

			ImGui::Separator();

			ImGui::SliderFloat ("Cone Radius/Height", &_settings->vct_refractive_cone_ratio, 0.0f, 1.0f, "%3f", 10.0f);
			ImGui::SliderFloat ("Cone Distance (Voxels Space)", &_settings->vct_refractive_cone_distance, 0.0f, 1.0f);

	        ImGui::Separator();

			if (ImGui::TreeNode ("Debug")) {

				ShowImage ("Subsurface Scattering Map", vctStat->vctSubsurfaceScatteringMapVolume);

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode ("Ambient Occlusion")) {

	        ImGui::Checkbox ("Enable Ambient Occlusion", &_settings->vct_ao_enabled);
			ImGui::SliderFloat ("Cone Radius/Height", &_settings->vct_ao_cone_ratio, 0.0f, 1.0f);
			ImGui::SliderFloat ("Cone Distance (Voxels Space)", &_settings->vct_ao_cone_distance, 0.0f, 1.0f);

	        ImGui::Separator();

			if (ImGui::TreeNode ("Debug")) {

				ShowImage ("Ambient Occlusion Map", vctStat->vctAmbientOcclusionMapVolume);

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode ("Shadows")) {

	        ImGui::Checkbox ("Shadow Cone Cast", &_settings->vct_shadow_cone_enabled);
			ImGui::SliderFloat ("Cone Radius/Height", &_settings->vct_shadow_cone_ratio, 0.0f, 1.0f, "%3f", 10.0f);
			ImGui::SliderFloat ("Cone Distance (Voxels Space)", &_settings->vct_shadow_cone_distance, 0.0f, 1.0f);

	        ImGui::Separator();

			if (ImGui::TreeNode ("Debug")) {

				ShowImage ("Shadow", vctStat->vctShadowVolume);

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		if (lastVoxelVolumeSize != _settings->vct_voxels_size ||
			lastVoxelBordering != _settings->vct_bordering ||
			lastVolumeMipmapLevels != _settings->vct_mipmap_levels) {
			_lastContinuousVoxelization = _settings->vct_continuous_voxelization;
			_continuousVoxelizationReset = true;

			_settings->vct_continuous_voxelization = true;
		}

        ImGui::Separator();

		if (ImGui::TreeNode ("Debug")) {

			ImGui::Checkbox ("Show Voxels", &_settings->vct_debug_show_voxels);

			if (_settings->vct_debug_show_voxels && _settings->renderMode == "VoxelConeTracingRenderModule") {
				_settings->renderMode = "VoxelRayTracingRenderModule";
			}
			else if (!_settings->vct_debug_show_voxels && _settings->renderMode == "VoxelRayTracingRenderModule") {
				_settings->renderMode = "VoxelConeTracingRenderModule";
			}

			std::size_t speed = 1;
			ImGui::InputScalar ("Mipmap Level", ImGuiDataType_U32, &_settings->vct_debug_volume_mipmap_level, &speed);
			_settings->vct_debug_volume_mipmap_level = Extensions::MathExtend::Clamp (
				_settings->vct_debug_volume_mipmap_level, (std::size_t) 0,
				(std::size_t) _settings->vct_mipmap_levels - 1);

			ImGui::TreePop();
		}
	}

    ImGui::Spacing();

	if (ImGui::CollapsingHeader ("Screen Space Global Illumination")) {

		if (ImGui::TreeNode ("Diffuse Indirect Illumination")) {

			ImGui::InputFloat ("Intensity", &_settings->ssdo_indirect_intensity, 0.1f);

			std::size_t limit1 = 1, limit2 = 500;
			ImGui::SliderScalar ("Sample Count", ImGuiDataType_U32, &_settings->ssdo_samples, &limit1, &limit2);

			ImGui::InputFloat ("Sampling Radius", &_settings->ssdo_radius, 0.1f);

			ImGui::Separator ();

			ImGui::Text ("Screen Interpolation");

			ImGui::Checkbox ("Enable Interpolation", &_settings->ssdo_interpolation_enabled);
			ImGui::Checkbox ("Shown Non Interpolated Pixels", &_settings->ssdo_debug_interpolation);

			float interpolationScale = _settings->ssdo_interpolation_scale;
			ImGui::InputFloat ("Interpolation Scale", &interpolationScale);
			if (interpolationScale > 0) {
				_settings->ssdo_interpolation_scale = interpolationScale;
			}

			ImGui::InputFloat ("Min Interpolation Distance", &_settings->ssdo_min_interpolation_distance, 0.1);
			ImGui::InputFloat ("Min Interpolation Angle (deg)", &_settings->ssdo_min_interpolation_angle, 0.1);

			ImGui::Separator ();

			if (ImGui::TreeNode ("Debug")) {
				auto ssdoStat = StatisticsManager::Instance ()->GetStatisticsObject <SSDOStatisticsObject> ();

				ShowImage ("Diifuse Indirect Interpolated Map", ssdoStat->ssdoInterpolatedMapVolume);

				ShowImage ("Diffuse Indirect Illumination Map", ssdoStat->ssdoMapVolume);

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode ("Glossy Reflections")) {

			ImGui::InputFloat ("Intensity", &_settings->ssr_intensity, 0.1f);

			// ImGui::SliderFloat ("Roughness", &_settings->ssr_roughness, 0.0f, 1.0f);

			std::size_t step = 1;
			ImGui::InputScalar ("Iteration Count", ImGuiDataType_U32, &_settings->ssr_iterations, &step);

			ImGui::SliderFloat ("Sample Thickness", &_settings->ssr_thickness, 0.0f, 10.0f);

			// std::size_t strideStep = 1;
			// ImGui::InputScalar ("Sample Stride", ImGuiDataType_U32, &_settings->ssr_stride, &strideStep);

			ImGui::Separator ();

			if (ImGui::TreeNode ("Debug")) {
				auto ssrStat = StatisticsManager::Instance ()->GetStatisticsObject <SSRStatisticsObject> ();

				ShowImage ("SSR Position Map", ssrStat->ssrPositionMapVolume);

				ShowImage ("SSR Map", ssrStat->ssrMapVolume);

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode ("Refraction")) {

			ImGui::InputFloat ("Intensity", &_settings->sst_intensity, 0.1f);

			std::size_t step = 1;
			ImGui::InputScalar ("Iteration Count", ImGuiDataType_U32, &_settings->sst_iterations, &step);

			ImGui::SliderFloat ("Sample Thickness", &_settings->sst_thickness, 0.0f, 10.0f);

			ImGui::Separator ();

			if (ImGui::TreeNode ("Debug")) {
				auto ssrStat = StatisticsManager::Instance ()->GetStatisticsObject <SSSubsurfaceScatteringStatisticsObject> ();

				ShowImage ("SSR Map", ssrStat->ssrMapVolume);

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode ("Screen Space Ambient Occlusion")) {

			ImGui::Checkbox ("Ambient Occlusion Enabled", &_settings->ssao_enabled);

			float scale = _settings->ssao_scale;
			ImGui::InputFloat ("Scale", &scale);
			if (scale > 0) {
				_settings->ssao_scale = scale;
			}

			std::size_t limit1 = 0, limit2 = 200;
			ImGui::SliderScalar ("Samples Size", ImGuiDataType_U32, &_settings->ssao_samples, &limit1, &limit2);
			
			ImGui::InputScalar ("Noise Size", ImGuiDataType_U32, &_settings->ssao_noise_size);
			ImGui::InputFloat ("Radius", &_settings->ssao_radius, 0.1f);
			ImGui::InputFloat ("Bias", &_settings->ssao_bias, 0.1f);

			ImGui::Separator ();

			ImGui::Checkbox ("Blur Enabled", &_settings->ssao_blur_enabled);
			// ImGui::Checkbox ("Temporal Filter Enabled", &_settings->ssao_temporal_filter_enabled);

			ImGui::Separator ();

			if (ImGui::TreeNode ("Debug")) {

				auto ssaoStat = StatisticsManager::Instance ()->GetStatisticsObject <SSAOStatisticsObject> ();

				ShowImage ("SSAO Map", ssaoStat->ssaoMapVolume);

				ShowImage ("SSAO Blur Map", ssaoStat->ssaoBlurMapVolume);

				ShowImage ("SSAO Temporal Filter Map", ssaoStat->ssaoTemporalFilterMapVolume);

				int mapWidth = ImGui::GetContentRegionAvail().x * 0.95f;

				ImGui::Text ("SSAO Noise Map");
				ShowImage (ssaoStat->ssaoNoiseMapVolume->GetTextureView (0)->GetGPUIndex (), glm::ivec2 (mapWidth));

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}
	}

	ImGui::Spacing ();

	if (ImGui::CollapsingHeader ("Post Processing")) {

		if (ImGui::TreeNode ("Temporal Anti-aliasing")) {

			ImGui::Checkbox ("Enabled", &_settings->taa_enabled);

			ImGui::Separator ();

			if (ImGui::TreeNode ("Debug")) {
				auto taaStat = StatisticsManager::Instance ()->GetStatisticsObject <TAAStatisticsObject> ();

				ShowImage ("TAA Map", taaStat->taaMapVolume);

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode ("Volumetric Lighting")) {

			ImGui::Checkbox ("Enabled", &_settings->vol_lighting_enabled);

			ImGui::Separator ();

			std::size_t step = 1;
			ImGui::InputScalar ("Iterations", ImGuiDataType_U32, &_settings->vol_lighting_iterations, &step);

			ImGui::SliderFloat ("Scattering", &_settings->vol_lighting_scattering, 0.0f, 1.0f);
			ImGui::InputFloat ("Intensity", &_settings->vol_lighting_intensity, 0.1f);

			ImGui::Separator ();			

			if (ImGui::TreeNode ("Debug")) {
				auto volLightingStat = StatisticsManager::Instance ()->GetStatisticsObject <VolLightingStatisticsObject> ();

				ShowImage ("Volumetric Lighting Map",volLightingStat->volumetricLightMapVolume);

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode ("Light Shafts")) {

			ImGui::Checkbox ("Enabled", &_settings->light_shafts_enabled);

			ImGui::Separator ();

			std::size_t step = 1;
			ImGui::InputScalar ("Iterations", ImGuiDataType_U32, &_settings->light_shafts_iterations, &step);

			ImGui::SliderFloat ("Density", &_settings->light_shafts_density, 0.0f, 1.0f);
			ImGui::SliderFloat ("Weight", &_settings->light_shafts_weight, 0.0f, 1.0f);
			ImGui::SliderFloat ("Decay", &_settings->light_shafts_decay, 0.0f, 1.0f);
			ImGui::InputFloat ("Intensity", &_settings->light_shafts_intensity, 0.1f);

			ImGui::Separator ();

			if (ImGui::TreeNode ("Debug")) {
				auto lightShaftsStat = StatisticsManager::Instance ()->GetStatisticsObject <LightShaftsStatisticsObject> ();

				ShowImage ("Light Shafts Map",lightShaftsStat->lightShaftsMapVolume);

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode ("Bloom")) {

			ImGui::Checkbox ("Enabled", &_settings->bloom_enabled);
			ImGui::SliderFloat ("Threshold", &_settings->bloom_threshold, 0.0f, 10.0f);
			ImGui::InputFloat ("Intensity", &_settings->bloom_intensity, 0.1f);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode ("High Dynamic Range")) {

			ImGui::Checkbox ("Enabled", &_settings->hdr_enabled);
			ImGui::InputFloat ("Exposure", &_settings->hdr_exposure, 0.1f);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode ("LUT Texture")) {

			ImGui::Checkbox ("Enabled", &_settings->lut_enabled);

			ImGui::Spacing ();
			ImGui::Spacing ();

			std::string lutTexturePath = _settings->lut_texture_path;

			ImGui::Text ("Path: %s", lutTexturePath.c_str ());

			if (_lutTexture == nullptr || _lastLUTTexturePath != lutTexturePath) {
				_lutTexture = Resources::LoadTexture (lutTexturePath);
				_lutTextureView = RenderSystem::LoadTexture (_lutTexture);
				_lastLUTTexturePath = lutTexturePath;
			}

			int windowWidth = ImGui::GetWindowWidth() * 0.6f;

			auto imageSize = _lutTexture->GetSize ();
			imageSize.height = windowWidth * ((float) imageSize.height / imageSize.width);
			imageSize.width = windowWidth;
			ImGui::Image((void*)(intptr_t) _lutTextureView->GetGPUIndex (), ImVec2(imageSize.width, imageSize.height));

			ImGui::SameLine ();

			bool lastLoadTexture = ImGui::Button ("Load", ImVec2 (48, 18));

			const char* path = _dialog.chooseFileDialog(lastLoadTexture);

			if (strlen (path) > 0) {
				_settings->lut_texture_path = path;				
			}

			ImGui::SliderFloat ("Intensity", &_settings->lut_intensity, 0.0f, 1.0f);

			ImGui::TreePop ();
		}

		if (ImGui::TreeNode ("Gamma Correction")) {

			ImGui::Checkbox ("Enabled", &_settings->gamma_enabled);

			ImGui::TreePop();
		}
	}

	ImGui::End();
}

void EditorRenderingSettings::ShowImage (const std::string& name,
	unsigned int textureID, const glm::ivec2& size)
{
	if (ImGui::TreeNodeEx (name.c_str (), ImGuiTreeNodeFlags_DefaultOpen)) {
		ShowImage (textureID, size);

		ImGui::TreePop();
	}
}

void EditorRenderingSettings::ShowImage (unsigned int textureID, const glm::ivec2& size)
{
	ImGui::Image((void*)(intptr_t) textureID, ImVec2(size.x, size.y), ImVec2 (0, 1), ImVec2 (1, 0));

    bool saveVolume = false;

    ImGui::PushID (textureID);

    if (ImGui::BeginPopupContextItem(std::to_string (textureID).c_str ())) {
		saveVolume = ImGui::MenuItem ("Save");

	    ImGui::EndPopup();
    }

	std::string volumePath = _dialog.saveFileDialog(saveVolume, "", "image.png", ".png");

	if (volumePath != std::string ()) {

		volumePath = FileSystem::Relative (volumePath, fs::current_path ().string ());

		Resource<TextureView> textureView (new TextureView (), "temp");
		textureView->SetGPUIndex (textureID);

		Resource<Texture> texture = RenderSystem::SaveTexture (textureView);

		textureView->SetGPUIndex (0);

		Resources::SaveTexture (texture, volumePath);
	}

	ImGui::PopID ();
}


void EditorRenderingSettings::ShowImage (const std::string& name, FramebufferRenderVolume* renderVolume)
{
	if (renderVolume == nullptr
		|| renderVolume->GetFramebuffer () == nullptr
		|| renderVolume->GetFramebuffer ()->GetTexture (0) == nullptr) {

		/*
		 *
		*/

		if (ImGui::TreeNodeEx (name.c_str (), ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text ("Unavailable");

			ImGui::TreePop();
		}

		return;
	}

	auto mapSize = renderVolume->GetFramebuffer ()->GetTexture (0)->GetSize ();

	int mapWidth = ImGui::GetContentRegionAvail().x * 0.95f;
	int mapHeight = ((float) mapSize.height / mapSize.width) * mapWidth;

	glm::ivec2 imageSize = glm::ivec2 (mapWidth, mapHeight);

	ShowImage (name, renderVolume->GetFramebufferView ()->GetTextureView (0)->GetGPUIndex (), imageSize);		
}

