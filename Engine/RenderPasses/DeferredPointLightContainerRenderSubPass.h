#ifndef DEFERREDPOINTLIGHTCONTAINERRENDERSUBPASS_H
#define DEFERREDPOINTLIGHTCONTAINERRENDERSUBPASS_H

#include "RenderPasses/PointVolumetricLightContainerRenderSubPass.h"

#include <string>

class DeferredPointLightContainerRenderSubPass : public PointVolumetricLightContainerRenderSubPass
{
protected:
	std::string _shaderName;
	std::string _shadowShaderName;

public:
	DeferredPointLightContainerRenderSubPass ();

	void Init (const RenderSettings& settings);

	void Clear ();
protected:
	void LockShader (const VolumetricLight*);
};

#endif