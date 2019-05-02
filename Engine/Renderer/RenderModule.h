#ifndef RENDERMODULE_H
#define RENDERMODULE_H

#include "Core/Interfaces/Object.h"

#include <vector>

#include "RenderPassI.h"

#include "RenderProduct.h"

#include "SceneGraph/Scene.h"
#include "Systems/Camera/Camera.h"
#include "RenderSettings.h"

class RenderModule : public Object
{
protected:
	RenderVolumeCollection* _rvc;
	std::vector<RenderPassI*> _renderPasses;

public:
	RenderModule ();
	virtual ~RenderModule ();

	virtual void InitModule (const RenderSettings&);
	virtual RenderProduct RenderScene (const Scene*, const Camera*, const RenderSettings&);

	virtual void ClearModule ();
protected:
	virtual void Init () = 0;
};

#endif