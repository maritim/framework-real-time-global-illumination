#version 420 core

// uniform layout (binding = 0, r32ui) coherent volatile uimage3D voxelVolume;
uniform layout (binding = 0, rgba8) coherent volatile image3D voxelVolume;

layout (location = 0) out vec4 fragColor;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform mat3 normalMatrix;
uniform mat3 normalWorldMatrix;

uniform vec3 MaterialDiffuse;
uniform vec3 MaterialEmissive;

uniform float MaterialTransparency;

uniform sampler2D DiffuseMap;
uniform sampler2D EmissiveMap;

uniform vec3 minPosition;
uniform vec3 maxPosition;

uniform ivec3 volumeSize;

in vec3 geom_worldPosition;
in vec3 geom_worldNormal;
in vec2 geom_texcoord;
in mat3 geom_swizzleMatrixInv;
in vec4 geom_BBox;

void main()
{
	/*
	 *
	*/

	vec2 bboxMin = floor((geom_BBox.xy * 0.5 + 0.5) * volumeSize.xy);
	vec2 bboxMax = ceil((geom_BBox.zw * 0.5 + 0.5) * volumeSize.xy);
	if (!all(greaterThanEqual(gl_FragCoord.xy, bboxMin)) || !all(lessThanEqual(gl_FragCoord.xy, bboxMax))) {
		discard;
	}

	/*
	 * Get color of all used texture maps
	*/

	vec3 diffuseMap = MaterialDiffuse * vec3 (texture2D (DiffuseMap, geom_texcoord.xy));
	vec3 emissiveMap = MaterialEmissive * vec3 (texture2D (EmissiveMap, geom_texcoord.xy));

	vec3 fragmentColor = emissiveMap;// + diffuseMap;

	/*
	 * Calculate the position in texture 3D
	*/
	
	vec3 coords = geom_swizzleMatrixInv * vec3(gl_FragCoord.xy, gl_FragCoord.z * volumeSize.z);

	/*
	 * Save in texture
	*/

	if (dot (emissiveMap, emissiveMap) > 0) {
		imageStore (voxelVolume, ivec3 (coords), vec4 (fragmentColor, 0.99));
	} else {
		imageStore (voxelVolume, ivec3 (coords), vec4 (fragmentColor, 1.0 - MaterialTransparency));
	}
}