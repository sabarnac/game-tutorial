#version 330 core

// The reason for suffixing structures and uniform variables with
//   the shader component name, is so that they don't collide with
//   definitions in other shaders.
// GPU shader compilers optimize and remove any unused variables,
//   and if there are different unused variables in the same structure
//   definition in different shader components, with both being used
//   through the same variable, then the shader first deletes the unused
//   variables in the initial shader component compilation step, then
//   fails to link the two shader components together because their
//   structures are now different.
// Note that for primitive uniform variables this cannot be an issue,
//   because there is no structure to change. Either the entire
//   variable is kept as is, or completely removed.

#define MAX_SIMPLE_LIGHTS 2
#define MAX_CUBE_LIGHTS 8

// The UV coordinates of the diffuse color of the fragment in the standard object texture.
// This is interpolated by the GPU for the fragment when passed from the vertex shader.
in vec2 fragmentUv;
// The coordinates of the fragment in the standard object model in world-space.
// This is interpolated by the GPU for the fragment when passed from the vertex shader.
in vec4 fragmentPosition_worldSpace;
// The coordinates of the fragment in the standard object model in view-space.
// This is interpolated by the GPU for the fragment when passed from the vertex shader.
in vec4 fragmentPosition_viewSpace;
// The normal vector of the fragment in the standard object model in view-space.
// This is interpolated by the GPU for the fragment when passed from the vertex shader.
in vec3 fragmentNormal_viewSpace;

// The shadow map coordinates of the current fragment w.r.t all the active simple lights.
in vec4 simpleLightShadowMapCoord[MAX_SIMPLE_LIGHTS];
// The direction of the light going towards the current fragment from of all the active
//   simple light sources.
in vec3 simpleLightDirection_viewSpace[MAX_SIMPLE_LIGHTS];
// The direction of the light going towards the current fragment from of all the active
//   cube light sources.
in vec3 cubeLightDirection_viewSpace[MAX_CUBE_LIGHTS];

// The final color of the fragment.
out vec3 color;

// The structure defining the details regarding the active lights.
struct LightDetails_Fragment
{
	vec3 lightPosition;
	mat4 lightVpMatrix;
	vec3 lightColor;
	float lightIntensity;
	float nearPlane;
	float farPlane;
};

// The standard object texture sampler.
uniform sampler2D diffuseTexture;

// The texture samplers of the shadow maps of simple lights (2D texture lights).
uniform sampler2D simpleLightTextures[MAX_SIMPLE_LIGHTS];
// The texture samplers of the shadow maps of cube lights (cubemap texture lights).
uniform samplerCube cubeLightTextures[MAX_CUBE_LIGHTS];

// The number of active simple lights (2D texture lights).
uniform int simpleLightsCount;
// The number of active cube lights (cubemap texture lights).
uniform int cubeLightsCount;

// The details of the active simple lights (2D texture lights).
uniform LightDetails_Fragment simpleLightDetails_fragment[MAX_SIMPLE_LIGHTS];
// The details of the active cube lights (cubemap texture lights).
uniform LightDetails_Fragment cubeLightDetails_fragment[MAX_CUBE_LIGHTS];

// The ambient light factor to use.
// This defines how much of the surface color is visible from ambient lighting.
uniform float ambientFactor;

// An integer mask defining what features should be disabled (lighting, shadows, etc.)
uniform int disableFeatureMask;

// The bias values to use to combat acne bias with the various light source types.
float simpleLightAcneBias = 0.0001;
float cubeLightAcneBias = 0.05;

// The specular values to use that define specular reflectivity and the lobe size.
// This could also be passed using a specular map, which would also allow to define
//   these values at a per-fragment level.
float specularReflectivity = 1.25;
float specularLobeFactor = 3.5;

// The mask flags for disabling shadows and lighting.
int DISABLE_SHADOW = 1;
int DISABLE_LIGHT = 2;

float isEqual(int required, int val) {
	return step(required, val) - step(required + 1, val);
}

/**
 * Function that returns the size of a single texel (texture-pixel) of the given
 *   simple light shadow map texture.
 *
 * @param lightIndex  The index of the simple light shadow map texture to use.
 *
 * @return The texel size of the texture.
 */
vec2 getSimpleLightShadowMapTexelValue(int lightIndex)
{
	// Grab the shadow map texture size value depending on which index is provided.
	vec2 shadowMapSize = 
			(isEqual(0, lightIndex) * textureSize(simpleLightTextures[0], 0)) +
			(isEqual(1, lightIndex) * textureSize(simpleLightTextures[1], 0));
	// Calculate the size of a single texel by taking the inverse of the texture size,
	//   and return it.
	return 1.0 / shadowMapSize;
}

/**
 * Function that returns the closest depth value recorded in the given simple
 *   light shadow map texture at the given UV coordinates.
 *
 * @param lightIndex  The index of the simple light shadow map texture to use.
 * @param coords      The UV coordinates from where to get the closest depth value.
 *
 * @return The closest depth value at the given UV coordinates.
 */
float getSimpleLightShadowMapCoordValue(vec2 coords, int lightIndex)
{
	// Grab the shadow map closest depth value depending on which index is provided.
	// We grab the value from the red channel because that is where the depth value
	//   is recorded.
	float closestDepth = 
			(isEqual(0, lightIndex) * texture(simpleLightTextures[0], coords).r) +
			(isEqual(1, lightIndex) * texture(simpleLightTextures[1], coords).r);
	// Return the captured closest depth value from the shadow map.
	return closestDepth;
}

/**
 * Function that returns the closest depth value recorded in the given cube
 *   light shadow map at the given UV coordinates.
 *
 * @param lightIndex  The index of the cube light shadow map texture to use.
 * @param coords      The UV coordinates from where to get the closest depth value.
 *
 * @return The closest depth value at the given UV coordinates.
 */
float getCubeLightShadowMapCoordValue(vec3 coords, int lightIndex)
{
	// Grab the shadow map closest depth value depending on which index is provided.
	// We grab the value from the red channel because that is where the depth value
	//   is recorded.
	float closestDepth = 
			(isEqual(0, lightIndex) * texture(cubeLightTextures[0], coords).r) +
			(isEqual(1, lightIndex) * texture(cubeLightTextures[1], coords).r) +
			(isEqual(2, lightIndex) * texture(cubeLightTextures[2], coords).r) +
			(isEqual(3, lightIndex) * texture(cubeLightTextures[3], coords).r) +
			(isEqual(4, lightIndex) * texture(cubeLightTextures[4], coords).r) +
			(isEqual(5, lightIndex) * texture(cubeLightTextures[5], coords).r) +
			(isEqual(6, lightIndex) * texture(cubeLightTextures[6], coords).r) +
			(isEqual(7, lightIndex) * texture(cubeLightTextures[7], coords).r);
	// Since for cube lights (point lights), we divided the actual depth against
	//   the max distance the light could reach till (the far plane), multiply
	//   by the same value again to the actual value back and return it.
	return closestDepth * cubeLightDetails_fragment[lightIndex].farPlane;
}

float getSimpleLightVisibility(vec2 shadowMapCoords, float currentDepth, int lightIndex)
{
	// Grab the depth of the fragment that was closest to the light source at the given coordinates.
	float closestDepth = getSimpleLightShadowMapCoordValue(shadowMapCoords, lightIndex);
	// If the depth of the current fragment w.r.t. the light source is farther away then the depth of the closest
	//   recorded fragment (accounting for some bias), that means the current fragment is not visible to the light
	//   source, so add visibility as 0. Otherwise, add visibility of the current fragment as 1.
	return currentDepth - simpleLightAcneBias > closestDepth ? 0.0 : 1.0;
}

/**
 * Function that returns the average visibility of the fragment from the given
 *   simple light source.
 *
 * @param shadowMapCoords  The shadow map coordinates of the current fragment.
 * @param currentDepth     The depth of the current fragment w.r.t. the light source.
 * @param lightIndex       The index of the simple light shadow map texture to use.
 *
 * @return The average visibility of the fragment.
 */
float getSimpleLightAverageVisibility(vec2 shadowMapCoords, float currentDepth, int lightIndex)
{
	// Define the variable where we'll store the average visibility.
  float visibility = 0.0;
	vec2 texelSize = getSimpleLightShadowMapTexelValue(lightIndex);
	// Get the texel size of the shadow map texture.
	// We'll sample the closest depth values from the given coordinate and the
	//   immediately surrounding coordinates as well to get a better average
	//   visibility value.
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			// Get the visibility of the fragment at the given shadow map coordinates.
			visibility += getSimpleLightVisibility(shadowMapCoords + (vec2(x, y) * texelSize), currentDepth, lightIndex);
		}
	}
	// Return the average visibility across the number of shadow map samples taken (5 * 5 = 25).
  return visibility / 9.0;
}

/**
 * Function that returns the average visibility of the fragment from the given
 *   cube light source.
 *
 * @param shadowMapCoords  The shadow map coordinates of the current fragment.
 * @param currentDepth     The depth of the current fragment w.r.t. the light source.
 * @param lightIndex       The index of the cube light shadow map texture to use.
 *
 * @return The average visibility of the fragment.
 */
float getCubeLightVisibility(vec3 shadowMapCoords, float currentDepth, int lightIndex)
{
	// Grab the depth of the fragment that was closest to the light source at the given coordinates.
	float closestDepth = getCubeLightShadowMapCoordValue(shadowMapCoords, lightIndex);
	// If the depth of the current fragment w.r.t. the light source is farther away then the depth of the closest
	//   recorded fragment (accounting for some bias), that means the current fragment is not visible to the light
	//   source, so add visibility as 0. Otherwise, add visibility of the current fragment as 1. Return the result.
	return currentDepth - cubeLightAcneBias > closestDepth ? 0.0 : 1.0;
}


/**
 * Function that calculates the diffuse lighting value from the given simple light source.
 *
 * @param lightIndex  The index of the simple light source to calculate against.
 *
 * @return The diffuse lighting value from the given simple light source.
 */
vec3 getSimpleLightDiffuseLighting(int lightIndex, vec3 lightColorIntensity, float distanceFromLight)
{
	// Calculate the strength of the diffuse lighting based on the angle of the light against the normal vector of the
	//   fragment of the surface.
  float diffuseStrength = clamp(dot(fragmentNormal_viewSpace, simpleLightDirection_viewSpace[lightIndex]), 0.0, 1.0);
	// Calculate the final diffuse lighting value using the light's color and intensity, the diffuse strength, and the
	//   distance of the light source from the fragment.
  return (lightColorIntensity * diffuseStrength) / (distanceFromLight * distanceFromLight);
}

/**
 * Function that calculates the diffuse lighting value from the given cube light source.
 *
 * @param lightIndex  The index of the cube light source to calculate against.
 *
 * @return The diffuse lighting value from the given cube light source.
 */
vec3 getCubeLightDiffuseLighting(int lightIndex, vec3 lightColorIntensity, float distanceFromLight)
{
	// Calculate the strength of the diffuse lighting based on the angle of the light against the normal vector of the
	//   fragment of the surface.
  float diffuseStrength = clamp(dot(fragmentNormal_viewSpace, cubeLightDirection_viewSpace[lightIndex]), 0.0, 1.0);
	// Calculate the final diffuse lighting value using the light's color and intensity, the diffuse strength, and the
	//   distance of the light source from the fragment.
  return (lightColorIntensity * diffuseStrength) / (distanceFromLight * distanceFromLight);
}


/**
 * Function that calculates the specular lighting value from the given simple light source.
 *
 * @param lightIndex  The index of the simple light source to calculate against.
 *
 * @return The specular lighting value from the given simple light source.
 */
vec3 getSimpleLightSpecularLighting(int lightIndex, vec3 lightColorIntensity, float distanceFromLight)
{
	// Calculate the direction of the view from the fragment position.
	vec3 viewDirection_viewSpace = normalize(fragmentPosition_viewSpace.xyz - vec3(0.0, 0.0, 0.0));
	// Calculate the direction of the light after it has reflected from the fragment.
	highp vec3 lightReflection_viewSpace = reflect(simpleLightDirection_viewSpace[lightIndex], fragmentNormal_viewSpace);

	// Calculate the strength of the specular lighting based on the angle of the reflected light against the direction of the camera
	//   from the fragment position.
  float specularStrength = clamp(dot(viewDirection_viewSpace, lightReflection_viewSpace), 0.0, 1.0);
	// Calculate the amount of specular light being reflected by the fragment as a factor of the light's color and intensity, the
	//   specular strength of the light, the specular lobe factor of the surface, and the distance of the light from the fragment.
  vec3 specularLight = (lightColorIntensity * pow(specularStrength, specularLobeFactor)) / (distanceFromLight * distanceFromLight);

	// Return the final specular lighting value as the calculated specular light value factored against the reflectivity of the surface
	//   of the fragment.
	return specularReflectivity * specularLight;
}

/**
 * Function that calculates the specular lighting value from the given cube light source.
 *
 * @param lightIndex  The index of the cube light source to calculate against.
 *
 * @return The specular lighting value from the given cube light source.
 */
vec3 getCubeLightSpecularLighting(int lightIndex, vec3 lightColorIntensity, float distanceFromLight)
{
	// Calculate the direction of the view from the fragment position.
	vec3 viewDirection_viewSpace = normalize(fragmentPosition_viewSpace.xyz - vec3(0.0, 0.0, 0.0));
	// Calculate the direction of the light after it has reflected from the fragment.
	highp vec3 lightReflection_viewSpace = reflect(cubeLightDirection_viewSpace[lightIndex], fragmentNormal_viewSpace);

	// Calculate the strength of the specular lighting based on the angle of the reflected light against the direction of the camera
	//   from the fragment position.
  float specularStrength = clamp(dot(viewDirection_viewSpace, lightReflection_viewSpace), 0.0, 1.0);
	// Calculate the amount of specular light being reflected by the fragment as a factor of the light's color and intensity, the
	//   specular strength of the light, the specular lobe factor of the surface, and the distance of the light from the fragment.
  vec3 specularLight = (lightColorIntensity * pow(specularStrength, specularLobeFactor)) / (distanceFromLight * distanceFromLight);

	// Return the final specular lighting value as the calculated specular light value factored against the reflectivity of the surface
	//   of the fragment.
	return specularReflectivity * specularLight;
}

void main()
{
	// Grab the diffuse color defined in the shot tecture using the given UV coordinates.
	vec3 surfaceColor = texture(diffuseTexture, fragmentUv).rgb;
	// Set the initial color value as the ambient lighting color value of the surface.
	// If lighting is disabled, the ambient factor is set to 1, since lighting should be ignored as a factor.
	color = surfaceColor * clamp(ambientFactor + clamp(disableFeatureMask - 1, 0, 1), 0.0, 1.0);

	// Perform lighting calculations as long as lighting has not been disabled.
	if (disableFeatureMask < DISABLE_LIGHT)
	{
		// Iterate through all the active simple lights.
		for (int lightIndex = 0; lightIndex < simpleLightsCount; lightIndex++)
		{
			// Define variable for storing the visibility of the fragment to the current active simple light source.
			float visibility;
			// Perform shadow visibility calculations as long as shadows have not been disabled.
			if (disableFeatureMask < DISABLE_SHADOW)
			{
				// Calculate the shadow map coordinates of the fragment w.r.t. the current active simple light source (while applying perspective-division).
				vec3 shadowMapCoords = (((simpleLightShadowMapCoord[lightIndex].xyz) / simpleLightShadowMapCoord[lightIndex].w) * 0.5) + 0.5;
				// Calculate the visibilty of the fragment to the current active simple light source.
				visibility = getSimpleLightAverageVisibility(shadowMapCoords.xy, shadowMapCoords.z, lightIndex);
			}
			else
			{
				// Since shadows have been disabled, the fragment will be fully visible to the light source.
				visibility = 1.0;
			}

			// Get the color value from the light after being increased based on the intensity of the light.
  		vec3 lightColorIntensity = simpleLightDetails_fragment[lightIndex].lightColor * simpleLightDetails_fragment[lightIndex].lightIntensity;
			// Calculate the distance of the fragment from the light source.
  		float distanceFromLight = distance(fragmentPosition_worldSpace, vec4(simpleLightDetails_fragment[lightIndex].lightPosition, 1.0));

			// Calculate and add the simple light diffuse lighting value to the final color output, factored against the color of the surface
			//   and the visibility of the fragment to the light source.
			color += visibility * surfaceColor * getSimpleLightDiffuseLighting(lightIndex, lightColorIntensity, distanceFromLight);
			// Calculate and add the simple light specular lighting value to the final color output, factored against the visibility of the
			//   fragment to the light source.
			color += visibility * getSimpleLightSpecularLighting(lightIndex, lightColorIntensity, distanceFromLight);
		}

		// Iterate through all the active cube lights.
		for (int lightIndex = 0; lightIndex < cubeLightsCount; lightIndex++)
		{
			// Define variable for storing the visibility of the fragment to the current active cube light source.
			float visibility;
			// Perform shadow visibility calculations as long as shadows have not been disabled.
			if (disableFeatureMask < DISABLE_SHADOW)
			{
				// Calculate the shadow map coordinates of the fragment w.r.t. the current active cube light source.
				vec3 shadowMapCoords = fragmentPosition_worldSpace.xyz - cubeLightDetails_fragment[lightIndex].lightPosition;
				// Calculate the visibilty of the fragment to the current active cube light source.
				visibility = getCubeLightVisibility(shadowMapCoords.xyz, length(shadowMapCoords), lightIndex);
			}
			else
			{
				// Since shadows have been disabled, the fragment will be fully visible to the light source.
				visibility = 1.0;
			}

			// Get the color value from the light after being increased based on the intensity of the light.
			vec3 lightColorIntensity = cubeLightDetails_fragment[lightIndex].lightColor * cubeLightDetails_fragment[lightIndex].lightIntensity;
			// Calculate the distance of the fragment from the light source.
			float distanceFromLight = distance(fragmentPosition_worldSpace, vec4(cubeLightDetails_fragment[lightIndex].lightPosition, 1.0));

			// Calculate and add the cube light diffuse lighting value to the final color output, factored against the color of the surface
			//   and the visibility of the fragment to the light source.
			color += visibility * surfaceColor * getCubeLightDiffuseLighting(lightIndex, lightColorIntensity, distanceFromLight);
			// Calculate and add the cube light specular lighting value to the final color output, factored against the visibility of the
			//   fragment to the light source.
			color += visibility * getCubeLightSpecularLighting(lightIndex, lightColorIntensity, distanceFromLight);
		}
	}
}