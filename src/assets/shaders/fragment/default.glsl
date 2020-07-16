#version 330 core

#extension GL_ARB_texture_cube_map_array: require

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

#define MAX_SIMPLE_LIGHTS 10
#define MAX_CUBE_LIGHTS 10

// The UV coordinates of the diffuse color of the fragment in the standard object texture.
// This is interpolated by the GPU for the fragment when passed from the vertex shader.
in vec2 fragmentUv;
// The coordinates of the fragment in the standard object model in world-space.
// This is interpolated by the GPU for the fragment when passed from the vertex shader.
in vec4 fragmentPosition_worldSpace;
// The normal vector of the fragment in the standard object model in view-space.
// This is interpolated by the GPU for the fragment when passed from the vertex shader.
in vec3 fragmentNormal_viewSpace;

// The shadow map coordinates of the current fragment w.r.t all the active cone lights.
in vec4 coneLightShadowMapCoord[MAX_SIMPLE_LIGHTS];


// The final color of the fragment.
out vec3 color;


// The structure defining the details regarding the model.
struct ModelDetails_Fragment
{
	// mat4 modelMatrix;
	mat4 viewMatrix;
	// mat4 projectionMatrix;
};
// The structure defining the details regarding the active lights.
struct LightDetails_Fragment
{
	vec3 lightPosition;
	// mat4 lightVpMatrix;
	vec3 lightColor;
	float lightIntensity;
	// float nearPlane;
	float farPlane;
	int layerId;
};


// The details of the model.
uniform ModelDetails_Fragment modelDetails_fragment;

// The standard object texture sampler.
uniform sampler2D diffuseTexture;

// The texture sampler of the array of shadow maps of cone lights (2D texture lights).
uniform sampler2DArray coneLightTextures;
// The texture samplers of the array of shadow maps of point lights (cubemap texture lights).
uniform samplerCubeArray pointLightTextures;

// The number of active cone lights (2D texture lights).
uniform int coneLightsCount;
// The number of active point lights (cubemap texture lights).
uniform int pointLightsCount;

// The details of the active cone lights (2D texture lights).
uniform LightDetails_Fragment coneLightDetails_fragment[MAX_SIMPLE_LIGHTS];
// The details of the active point lights (cubemap texture lights).
uniform LightDetails_Fragment pointLightDetails_fragment[MAX_CUBE_LIGHTS];

// The ambient light factor to use.
// This defines how much of the surface color is visible from ambient lighting.
uniform float ambientFactor;

// An integer mask defining what features should be disabled (lighting, shadows, etc.)
uniform int disableFeatureMask;

// The bias values to use to combat acne bias with the various light source types.
float coneLightAcneBias = 0.0001;
float pointLightAcneBias = 0.05;

// The specular values to use that define specular reflectivity and the lobe size.
// This could also be passed using a specular map, which would also allow to define
//   these values at a per-fragment level.
float specularReflectivity = 1.25;
float specularLobeFactor = 3.5;

// The mask flags for disabling shadows and lighting.
int DISABLE_SHADOW = 1;
int DISABLE_LIGHT = 2;

/**
 * Function that returns the size of a single texel (texture-pixel) of the given
 *   cone light shadow map texture.
 *
 * @return The texel size of the texture.
 */
vec2 getConeLightShadowMapTexelValue()
{
	// Grab the shadow map texture size from the sampler array
	//   (just the first two coordinates, the third indicates number of layers in the
	//   sampler array).
	vec2 shadowMapSize = textureSize(coneLightTextures, 0).xy;
	// Calculate the size of a single texel by taking the inverse of the texture size,
	//   and return it.
	return 1.0 / shadowMapSize;
}

/**
 * Function that returns the size of a single texel (texture-pixel) of the given
 *   point light shadow map texture.
 *
 * @return The texel size of the texture.
 */
vec3 getPointLightShadowMapTexelValue()
{
	// Grab the shadow map texture size value depending on which index is provided
	//   (just the first two coordinates, the third indicates number of layers in the
	//   sampler array).
	vec2 shadowMapSize = textureSize(pointLightTextures, 0).xy;
	// Calculate the size of a single texel by taking the inverse of the texture size,
	//   and return it. The third coordinate is calculated by taking the average of the
	//   size of the texture on the x-axis and y-axis.
	return 1.0 / vec3(shadowMapSize, (shadowMapSize.x + shadowMapSize.y) / 2);
}

/**
 * Function that returns the closest depth value recorded in the given cone
 *   light shadow map texture at the given UV coordinates.
 *
 * @param coords   The UV coordinates from where to get the closest depth value.
 * @param layerId  The index of the cone light shadow map texture to use.
 *
 * @return The closest depth value at the given UV coordinates.
 */
float getConeLightShadowMapCoordValue(vec2 coords, int layerId)
{
	// Grab the closest depth value from the shadow map indexed at the given layer.
	// We grab the value from the red channel because that is where the depth value
	//   is recorded.
	return texture(coneLightTextures, vec3(coords, layerId)).r;
}

/**
 * Function that returns the closest depth value recorded in the given point
 *   light shadow map at the given UV coordinates.
 *
 * @param coords    The UV coordinates from where to get the closest depth value.
 * @param layerId   The index of the point light shadow map texture to use.
 * @param farPlane  The maximum distance the light source can travel till.
 *
 * @return The closest depth value at the given UV coordinates.
 */
float getPointLightShadowMapCoordValue(vec3 coords, int layerId, float farPlane)
{
	// Grab the closest depth value from the shadow map indexed at the given layer.
	// We grab the value from the red channel because that is where the depth value
	//   is recorded.
	float closestDepth = texture(pointLightTextures, vec4(coords, layerId)).r;
	// Since for point lights, we divided the actual depth against the max distance
	//   the light could reach till (the far plane), multiply by the same value again
	//   to the actual value back and return it.
	return closestDepth * farPlane;
}

/**
 * Function that returns the visibility of the fragment from the given
 *   cone light source.
 *
 * @param shadowMapCoords  The shadow map coordinates of the current fragment.
 * @param currentDepth     The depth of the current fragment w.r.t. the light source.
 * @param layerId          The index of the cone light shadow map texture to use.
 *
 * @return The visibility of the fragment.
 */
float getConeLightVisibility(vec2 shadowMapCoords, float currentDepth, int layerId)
{
	// Grab the depth of the fragment that was closest to the light source at the given coordinates from the
	//   shadow map at the given layer.
	float closestDepth = getConeLightShadowMapCoordValue(shadowMapCoords, layerId);
	// If the depth of the current fragment w.r.t. the light source is larger than the depth of the closest
	//   recorded fragment (accounting for some bias), that means the current fragment is not visible to the
	//   light source, so the fragment should not be visible. If this is the case, return 0, otherwise return 1.
	return currentDepth - coneLightAcneBias > closestDepth ? 0.0 : 1.0;
}

/**
 * Function that returns the average visibility of the fragment from the given
 *   cone light source by taking multiple samples at and around the given shadow
 *   map coordinates.
 *
 * @param shadowMapCoords  The shadow map coordinates of the current fragment.
 * @param currentDepth     The depth of the current fragment w.r.t. the light source.
 * @param layerId          The index of the cone light shadow map texture to use.
 *
 * @return The average visibility of the fragment.
 */
float getConeLightAverageVisibility(vec2 shadowMapCoords, float currentDepth, int layerId)
{
	// Define the variable where we'll store the average visibility.
  float visibility = 0.0;
	// Get the texel size of the shadow map texture.
	vec2 texelSize = getConeLightShadowMapTexelValue();
	// We'll sample the closest depth values from the given coordinate and the
	//   immediately surrounding coordinates as well to get a better average
	//   visibility value.
	for (int x = -2; x <= 2; x++)
	{
		for (int y = -2; y <= 2; y++)
		{
			// Get the visibility of the fragment at the given shadow map coordinates (with variance).
			visibility += getConeLightVisibility(shadowMapCoords + (vec2(x, y) * texelSize), currentDepth, layerId);
		}
	}
	// Return the average visibility across the number of shadow map samples taken (5 * 5 = 25).
  return visibility / 25.0;
}

/**
 * Function that returns the average visibility of the fragment from the given
 *   point light source.
 *
 * @param shadowMapCoords  The shadow map coordinates of the current fragment.
 * @param currentDepth     The depth of the current fragment w.r.t. the light source.
 * @param layerId          The index of the point light shadow map texture to use.
 *
 * @return The visibility of the fragment.
 */
float getPointLightVisibility(vec3 shadowMapCoords, float currentDepth, int layerId, float farPlane)
{
	// Grab the depth of the fragment that was closest to the light source at the given coordinates from the
	//   shadow map at the given layer.
	float closestDepth = getPointLightShadowMapCoordValue(shadowMapCoords, layerId, farPlane);
	// If the depth of the current fragment w.r.t. the light source is larger than the depth of the closest
	//   recorded fragment (accounting for some bias), that means the current fragment is not visible to the
	//   light source, so the fragment should not be visible. If this is the case, return 0, otherwise return 1.
	return currentDepth - pointLightAcneBias > closestDepth ? 0.0 : 1.0;
}

/**
 * Function that returns the average visibility of the fragment from the given
 *   point light source.
 *
 * @param shadowMapCoords  The shadow map coordinates of the current fragment.
 * @param currentDepth     The depth of the current fragment w.r.t. the light source.
 * @param layerId          The index of the point light shadow map texture to use.
 *
 * @return The average visibility of the fragment.
 */
float getPointLightAverageVisibility(vec3 shadowMapCoords, float currentDepth, int layerId, float farPlane)
{
	// Define the variable where we'll store the average visibility.
  float visibility = 0.0;
	// Get the texel size of the shadow map texture.
	vec3 texelSize = getPointLightShadowMapTexelValue();
	// We'll sample the closest depth values from the given coordinate and the
	//   immediately surrounding coordinates as well to get a better average
	//   visibility value.
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			for (int z = -1; z <= 1; z++)
			{
				// Get the visibility of the fragment at the given shadow map coordinates (with variance).
				visibility += getPointLightVisibility(shadowMapCoords + (vec3(x, y, z) * texelSize), currentDepth, layerId, farPlane);
			}
		}
	}
	// Return the average visibility across the number of shadow map samples taken (3 * 3 * 3 = 27).
  return visibility / 27.0;
}

/**
 * Function that calculates the diffuse lighting value from the given cone light source.
 *
 * @param lightColorIntensity           The product of the color and intensity value of the light sources.
 * @param distanceFromLight             The distance from the light source to the current fragment.
 * @param coneLightDirection_viewSpace  The direction of the light from the light source to the current fragment.
 *
 * @return The diffuse lighting value from the given cone light source.
 */
vec3 getConeLightDiffuseLighting(vec3 lightColorIntensity, float distanceFromLight, vec3 coneLightDirection_viewSpace)
{
	// Calculate the strength of the diffuse lighting based on the angle of the light against the normal vector of the
	//   fragment on the surface.
  float diffuseStrength = clamp(dot(fragmentNormal_viewSpace, coneLightDirection_viewSpace), 0.0, 1.0);
	// Calculate the final diffuse lighting value using the light's color and intensity, the diffuse strength, and the
	//   distance of the light source from the fragment.
  return (lightColorIntensity * diffuseStrength) / (distanceFromLight * distanceFromLight);
}

/**
 * Function that calculates the diffuse lighting value from the given point light source.
 *
 * @param lightColorIntensity            The product of the color and intensity value of the light sources.
 * @param distanceFromLight              The distance from the light source to the current fragment.
 * @param pointLightDirection_viewSpace  The direction of the light from the light source to the current fragment.
 *
 * @return The diffuse lighting value from the given point light source.
 */
vec3 getPointLightDiffuseLighting(vec3 lightColorIntensity, float distanceFromLight, vec3 pointLightDirection_viewSpace)
{
	// Calculate the strength of the diffuse lighting based on the angle of the light against the normal vector of the
	//   fragment on the surface.
  float diffuseStrength = clamp(dot(fragmentNormal_viewSpace, pointLightDirection_viewSpace), 0.0, 1.0);
	// Calculate the final diffuse lighting value using the light's color and intensity, the diffuse strength, and the
	//   distance of the light source from the fragment.
  return (lightColorIntensity * diffuseStrength) / (distanceFromLight * distanceFromLight);
}


/**
 * Function that calculates the specular lighting value from the given cone light source.
 *
 * @param fragmentPosition_viewSpace     The position of the current fragment in view-space.
 * @param lightColorIntensity           The product of the color and intensity value of the light sources.
 * @param distanceFromLight             The distance from the light source to the current fragment.
 * @param coneLightDirection_viewSpace  The direction of the light from the light source to the current fragment.
 *
 * @return The specular lighting value from the given cone light source.
 */
vec3 getConeLightSpecularLighting(vec4 fragmentPosition_viewSpace, vec3 lightColorIntensity, float distanceFromLight, vec3 coneLightDirection_viewSpace)
{
	// Calculate the direction of the view from the fragment position.
	vec3 viewDirection_viewSpace = normalize(fragmentPosition_viewSpace.xyz - vec3(0.0, 0.0, 0.0));
	// Calculate the direction of the light after it has reflected from the fragment.
	highp vec3 lightReflection_viewSpace = reflect(coneLightDirection_viewSpace, fragmentNormal_viewSpace);

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
 * Function that calculates the specular lighting value from the given point light source.
 *
 * @param fragmentPosition_viewSpace     The position of the current fragment in view-space.
 * @param lightColorIntensity            The product of the color and intensity value of the light sources.
 * @param distanceFromLight              The distance from the light source to the current fragment.
 * @param pointLightDirection_viewSpace  The direction of the light from the light source to the current fragment.
 *
 * @return The specular lighting value from the given point light source.
 */
vec3 getPointLightSpecularLighting(vec4 fragmentPosition_viewSpace, vec3 lightColorIntensity, float distanceFromLight, vec3 pointLightDirection_viewSpace)
{
	// Calculate the direction of the view from the fragment position.
	vec3 viewDirection_viewSpace = normalize(fragmentPosition_viewSpace.xyz - vec3(0.0, 0.0, 0.0));
	// Calculate the direction of the light after it has reflected from the fragment.
	highp vec3 lightReflection_viewSpace = reflect(pointLightDirection_viewSpace, fragmentNormal_viewSpace);

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
	// Grab the diffuse color defined in the shot texture using the given UV coordinates.
	vec3 surfaceColor = texture(diffuseTexture, fragmentUv).rgb;
	// Set the initial color value as the ambient lighting color value of the surface.
	// If lighting is disabled, the ambient factor is set to 1, since lighting should be ignored as a factor.
	color = surfaceColor * clamp(ambientFactor + clamp(disableFeatureMask - 1, 0, 1), 0.0, 1.0);

	// Perform lighting calculations as long as lighting has not been disabled.
	if (disableFeatureMask < DISABLE_LIGHT)
	{
		// Iterate through all the active cone lights.
		for (int lightIndex = 0; lightIndex < coneLightsCount; lightIndex++)
		{
			// Calculate the position of the current fragment in view-space.
			vec4 fragmentPosition_viewSpace = modelDetails_fragment.viewMatrix * fragmentPosition_worldSpace;
			// Calculate the position of the light in view-space.
			vec4 lightPosition_viewSpace = modelDetails_fragment.viewMatrix * vec4(coneLightDetails_fragment[lightIndex].lightPosition, 1.0);
			// Calculate the direction of the light from the source to the fragment in view-space.
			vec3 coneLightDirection_viewSpace = normalize((lightPosition_viewSpace - fragmentPosition_viewSpace).xyz);

			// Define variable for storing the visibility of the fragment to the current light source.
			float visibility;
			// Perform shadow visibility calculations as long as shadows have not been disabled.
			if (disableFeatureMask < DISABLE_SHADOW)
			{
				// Calculate the shadow map coordinates of the fragment w.r.t. the current light source (while applying perspective-division).
				vec3 shadowMapCoords = (((coneLightShadowMapCoord[lightIndex].xyz) / coneLightShadowMapCoord[lightIndex].w) * 0.5) + 0.5;
				// Calculate the visibilty of the fragment to the current light source.
				visibility = getConeLightAverageVisibility(shadowMapCoords.xy, shadowMapCoords.z, coneLightDetails_fragment[lightIndex].layerId);
			}
			else
			{
				// Since shadows have been disabled, the fragment will be fully visible to the light source.
				visibility = 1.0;
			}

			// Get the color value from the light after being increased based on the intensity of the light.
  		vec3 lightColorIntensity = coneLightDetails_fragment[lightIndex].lightColor * coneLightDetails_fragment[lightIndex].lightIntensity;
			// Calculate the distance of the fragment from the light source.
  		float distanceFromLight = distance(fragmentPosition_worldSpace.xyz, coneLightDetails_fragment[lightIndex].lightPosition);

			// Calculate and add the light diffuse lighting value to the final color output, factored against the color of the surface
			//   and the visibility of the fragment to the light source.
			color += visibility * surfaceColor * getConeLightDiffuseLighting(lightColorIntensity, distanceFromLight, coneLightDirection_viewSpace);
			// Calculate and add the light specular lighting value to the final color output, factored against the visibility of the
			//   fragment to the light source.
			color += visibility * getConeLightSpecularLighting(fragmentPosition_viewSpace, lightColorIntensity, distanceFromLight, coneLightDirection_viewSpace);
		}

		// Iterate through all the active point lights.
		for (int lightIndex = 0; lightIndex < pointLightsCount; lightIndex++)
		{
			// Calculate the position of the current fragment in view-space.
			vec4 fragmentPosition_viewSpace = modelDetails_fragment.viewMatrix * fragmentPosition_worldSpace;
			// Calculate the position of the light in view-space.
			vec4 lightPosition_viewSpace = modelDetails_fragment.viewMatrix * vec4(pointLightDetails_fragment[lightIndex].lightPosition, 1.0);
			// Calculate the direction of the light from the source to the fragment in view-space.
			vec3 pointLightDirection_viewSpace = normalize((lightPosition_viewSpace - fragmentPosition_viewSpace).xyz);

			// Define variable for storing the visibility of the fragment to the current light source.
			float visibility;
			// Perform shadow visibility calculations as long as shadows have not been disabled.
			if (disableFeatureMask < DISABLE_SHADOW)
			{
				// Calculate the shadow map coordinates of the fragment w.r.t. the current light source.
				vec3 shadowMapCoords = fragmentPosition_worldSpace.xyz - pointLightDetails_fragment[lightIndex].lightPosition;
				// Calculate the visibilty of the fragment to the current light source.
				visibility = getPointLightAverageVisibility(shadowMapCoords.xyz, length(shadowMapCoords), pointLightDetails_fragment[lightIndex].layerId, pointLightDetails_fragment[lightIndex].farPlane);
			}
			else
			{
				// Since shadows have been disabled, the fragment will be fully visible to the light source.
				visibility = 1.0;
			}

			// Get the color value from the light after being increased based on the intensity of the light.
			vec3 lightColorIntensity = pointLightDetails_fragment[lightIndex].lightColor * pointLightDetails_fragment[lightIndex].lightIntensity;
			// Calculate the distance of the fragment from the light source.
			float distanceFromLight = distance(fragmentPosition_worldSpace.xyz, pointLightDetails_fragment[lightIndex].lightPosition);

			// Calculate and add the light diffuse lighting value to the final color output, factored against the color of the surface
			//   and the visibility of the fragment to the light source.
			color += visibility * surfaceColor * getPointLightDiffuseLighting(lightColorIntensity, distanceFromLight, pointLightDirection_viewSpace);
			// Calculate and add the light specular lighting value to the final color output, factored against the visibility of the
			//   fragment to the light source.
			color += visibility * getPointLightSpecularLighting(fragmentPosition_viewSpace, lightColorIntensity, distanceFromLight, pointLightDirection_viewSpace);
		}
	}
}