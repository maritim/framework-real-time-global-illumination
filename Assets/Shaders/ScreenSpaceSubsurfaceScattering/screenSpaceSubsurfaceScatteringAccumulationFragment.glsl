#version 330 core

layout(location = 0) out vec3 out_color;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform mat3 normalMatrix;
uniform mat3 normalWorldMatrix;
uniform mat4 inverseViewMatrix;

uniform vec3 cameraPosition;
uniform vec2 cameraZLimits;

uniform float sstIntensity;

uniform sampler2D postProcessMap;
uniform sampler2D subsurfaceScatteringMap;

#include "deferred.glsl"

void main()
{
	vec2 texCoord = CalcTexCoord();
	vec3 in_position = texture (gPositionMap, texCoord).xyz;
	vec3 in_tr_position = texture (gTrPositionMap, texCoord).xyz;
	vec3 in_diffuse = texture (postProcessMap, texCoord).xyz;
	vec3 in_refraction = texture (subsurfaceScatteringMap, texCoord).xyz;
	float in_transparency = texture (gTrDiffuseMap, texCoord).w;

	if (in_transparency == 0.0f || in_tr_position.z < in_position.z) {
		out_color = in_diffuse;
	} else {
		out_color = mix (in_diffuse, in_refraction * sstIntensity, in_transparency);
	}
}
