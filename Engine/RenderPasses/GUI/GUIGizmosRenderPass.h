#ifndef GUIGIZMOSRENDERPASS_H
#define GUIGIZMOSRENDERPASS_H

#include "RenderPasses/Container/ContainerRenderSubPassI.h"

#include "Core/Resources/Resource.h"
#include "Renderer/RenderViews/ShaderView.h"

#include "Systems/GUI/Gizmo/Gizmo.h"

class ENGINE_API GUIGizmosRenderPass : public ContainerRenderSubPassI
{
	DECLARE_RENDER_PASS(GUIGizmosRenderPass)

protected:
	Resource<ShaderView> _shaderView;

public:
	virtual void Init (const RenderSettings& settings);
	virtual RenderVolumeCollection* Execute (const RenderScene* renderScene, const Camera* camera,
		const RenderSettings& settings, RenderVolumeCollection* rvc);
	virtual bool IsAvailable (const RenderScene* renderScene, const Camera* camera,
		const RenderSettings& settings, const RenderVolumeCollection* rvc) const;

	void Clear ();
protected:
	void StartGizmoDraw (RenderVolumeCollection* rvc);

	void DrawGizmos (const Camera* camera, const RenderSettings& settings);
	void DrawLines (const Camera* camera, const RenderSettings& settings);

	void AddLine (std::vector<float>&, std::size_t stride, const GizmoLine& line);

	void DrawData (const std::vector<float>& data, const std::vector<unsigned int>& indices,
		const Camera* camera, const RenderSettings& settings, bool depth = false);
};

#endif