#ifndef LPVRADIANCEINJECTIONRENDERPASS_H
#define LPVRADIANCEINJECTIONRENDERPASS_H

#include "RenderPasses/VolumetricLightRenderPassI.h"

#include "Core/Resources/Resource.h"
#include "Renderer/RenderViews/ShaderView.h"

#include "LPVVolume.h"

class ENGINE_API LPVRadianceInjectionRenderPass : public VolumetricLightRenderPassI
{
	DECLARE_RENDER_PASS(LPVRadianceInjectionRenderPass)

protected:
	Resource<ShaderView> _shaderView;
	LPVVolume* _lpvVolume;

public:
	LPVRadianceInjectionRenderPass ();
	~LPVRadianceInjectionRenderPass ();

	void Init (const RenderSettings& settings);
	RenderVolumeCollection* Execute (const RenderScene* renderScene, const Camera* camera,
		const RenderSettings& settings, RenderVolumeCollection* rvc);

	void Clear ();
protected:
	bool IsAvailable (const RenderLightObject*) const;

	void StartPostProcessPass ();
	void PostProcessPass (const RenderScene* renderScene, const Camera* camera,
		const RenderSettings& settings, const RenderLightObject* renderLightObject,
		RenderVolumeCollection* rvc);
	void EndPostProcessPass ();

	std::vector<PipelineAttribute> GetCustomAttributes (const RenderLightObject* renderLightObject) const;

	void UpdateLPVVolumeBoundingBox (const RenderScene* renderScene);
	void InitLPVVolume (const RenderSettings& settings);
	void UpdateLPVVolume (const RenderSettings& settings);
};

#endif