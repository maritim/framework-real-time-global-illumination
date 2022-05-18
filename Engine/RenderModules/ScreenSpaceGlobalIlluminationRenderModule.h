#ifndef SCREENSPACEGLOBALILLUMINATIONRENDERMODULE_H
#define SCREENSPACEGLOBALILLUMINATIONRENDERMODULE_H

#include "Renderer/RenderModule.h"

#include "Renderer/RenderModuleManager.h"

class ScreenSpaceGlobalIlluminationRenderModule : public RenderModule
{
protected:
	void Init ();
};

REGISTER_RENDER_MODULE(ScreenSpaceGlobalIlluminationRenderModule)

#endif