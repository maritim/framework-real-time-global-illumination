#version 330

layout(location = 0) out vec3 out_color;

uniform sampler2D gPositionMap;
uniform sampler2D gNormalMap;
uniform sampler2D gDiffuseMap;
uniform sampler2D gSpecularMap;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 viewProjectionMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform mat3 normalMatrix;
uniform mat3 normalWorldMatrix;
uniform mat4 inverseViewMatrix;
uniform mat3 inverseNormalWorldMatrix;

uniform vec3 cameraPosition;

uniform vec3 lightPosition;
uniform vec3 lightColor;

uniform vec2 screenSize;

uniform sampler2D rsmShadowMap;
uniform sampler2D rsmPositionMap;
uniform sampler2D rsmNormalMap;
uniform sampler2D rsmFluxMap;

uniform mat4 rsmLightSpaceMatrix;

uniform int rsmSamplesCount;

uniform vec2 rsmSample[200];

uniform float rsmRadius;
uniform float rsmIntensity;

#include "ShadowMap/cascadedShadowMap.glsl"

vec2 CalcTexCoord()
{
	return gl_FragCoord.xy / screenSize;
}

vec3 CalcDirectDiffuseLight (vec3 in_position, vec3 in_normal)
{
	// The position is also a direction for Directional Lights
	vec3 lightDirection = normalize (lightPosition);

	// Diffuse contribution
	float dCont = max (dot (in_normal, lightDirection), 0.0);

	// Attenuation is 1.0 for Directional Lights
	vec3 diffuseColor = lightColor * dCont;

	return diffuseColor;
}

//vec3 CalcDirectSpecularLight (vec3 in_position, vec3 in_normal, vec3 in_diffuse, vec3 in_specular, float in_shininess)
//{
//	vec3 surface2view = normalize (cameraPosition - in_position);
//	vec3 reflection = reflect (-lightDirection, in_normal);

//	// Specular contribution
//	float sCont = pow (max (dot (surface2view, reflection), 0.0), 1.0);

//	vec3 specularColor = lightColor * sCont;

//	return specularColor;
//}

/*
 * Indirect Illumination Calculation
 * Thanks to: http://ericpolman.com/reflective-shadow-maps-part-2-the-implementation/
*/

vec3 CalcIndirectDiffuseLight (vec3 worldSpacePos, vec3 worldSpaceNormal)
{
	vec3 indirectColor = vec3 (0.0);

	vec4 lightSpacePos = rsmLightSpaceMatrix * vec4 (worldSpacePos, 1.0);
	vec3 rsmProjCoords = lightSpacePos.xyz / lightSpacePos.w;

	rsmProjCoords = rsmProjCoords * 0.5 + 0.5;

	for (int index = 0; index < rsmSamplesCount; index ++) {
		vec2 rnd = rsmSample [index];

		vec2 coords = rsmProjCoords.xy + rnd * rsmRadius;

		vec3 rsmWorldSpacePos = texture2D (rsmPositionMap, coords).xyz;
		vec3 rsmWorldSpaceNormal = texture2D (rsmNormalMap, coords).xyz;
		vec3 rsmFlux = texture2D (rsmFluxMap, coords).xyz;

		vec3 result = rsmFlux *
			((max (0.0, dot (rsmWorldSpaceNormal, worldSpacePos - rsmWorldSpacePos))
				* max (0.0, dot (worldSpaceNormal, rsmWorldSpacePos - worldSpacePos)))
			/ pow (length (worldSpacePos - rsmWorldSpacePos), 4.0));

		float dist = length (rnd);

		result = result * dist * dist;

		indirectColor += result;
	}

	return indirectColor * rsmIntensity;
}

vec3 CalcDirectionalLight (vec3 in_position, vec3 in_normal, vec3 in_diffuse, vec3 in_specular, float in_shininess)
{
	// Compute fragment world position and world normal
	vec3 worldPosition = vec3 (inverseViewMatrix * vec4 (in_position, 1.0));
	vec3 worldNormal = normalMatrix * inverseNormalWorldMatrix * in_normal;

	vec3 directDiffuseColor = CalcDirectDiffuseLight (worldPosition, worldNormal);

	float shadow = CalcDirectionalShadowContribution (in_position);

	directDiffuseColor = (1.0 - shadow) * (directDiffuseColor);

	vec3 indirectDiffuseColor = CalcIndirectDiffuseLight (worldPosition, worldNormal);

	return (directDiffuseColor + indirectDiffuseColor) * in_diffuse;
}

void main()
{
	vec2 texCoord = CalcTexCoord();
	vec3 in_position = texture2D (gPositionMap, texCoord).xyz;
	vec3 in_diffuse = texture2D (gDiffuseMap, texCoord).xyz;
	vec3 in_normal = texture2D (gNormalMap, texCoord).xyz;
	vec3 in_specular = texture2D (gSpecularMap, texCoord).xyz;
	float in_shininess = texture2D (gSpecularMap, texCoord).w;

	in_normal = normalize(in_normal);

	out_color = CalcDirectionalLight(in_position, in_normal, in_diffuse, in_specular, in_shininess);
} 