#ifndef SPOTLIGHTCOMPONENT_H
#define SPOTLIGHTCOMPONENT_H

#include "PointLightComponent.h"

class ENGINE_API SpotLightComponent : public PointLightComponent
{
	DECLARE_COMPONENT(SpotLightComponent)

protected:
	ATTRIBUTE(EditAnywhere, Meta)	
	float _cutoff;
	ATTRIBUTE(EditAnywhere, Meta)	
	float _outerCutoff;

public:
	SpotLightComponent ();

	void Awake ();

	void Update ();

	void OnAttachedToScene ();
	void OnDetachedFromScene ();

	float GetSpotCutoff () const;
	float GetSpotOuterCutoff () const;

	void SetCutoff (float spotCutoff);
	void SetOuterCutoff (float spotOuterCutoff);
protected:
	void UpdateTransform ();
};

#endif