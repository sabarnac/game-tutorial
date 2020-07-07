#version 330 core

#define MAX_SIMPLE_LIGHTS 2
#define MAX_CUBE_LIGHTS 8

in vec2 fragmentUv;
in vec4 vertexPosition_worldSpace;
in vec4 vertexPosition_viewSpace;
in vec3 vertexNormal_viewSpace;

in vec4 simpleLightDepthCoord[MAX_SIMPLE_LIGHTS];
in vec3 simpleLightDirection_viewSpace[MAX_SIMPLE_LIGHTS];
in vec3 cubeLightDirection_viewSpace[MAX_CUBE_LIGHTS];

out vec3 color;


struct LightDetails_Fragment
{
	vec3 lightPosition;
	vec3 lightColor;
	float lightIntensity;
	float farPlane;
};

uniform sampler2D diffuseTexture;

uniform sampler2D simpleLightTextures[MAX_SIMPLE_LIGHTS];
uniform samplerCube cubeLightTextures[MAX_CUBE_LIGHTS];

uniform LightDetails_Fragment simpleLightDetails_fragment[MAX_SIMPLE_LIGHTS];
uniform LightDetails_Fragment cubeLightDetails_fragment[MAX_CUBE_LIGHTS];

uniform int simpleLightsCount;
uniform int cubeLightsCount;

uniform float ambientFactor;

uniform int disableFeatureMask;

float simpleLightAcneBias = 0.0001;
float cubeLightAcneBias = 0.03;

float specularReflectivity = 1.25;
float specularLobeFactor = 3.5;

int DISABLE_SHADOW = 1;
int DISABLE_LIGHT = 2;

float getSimpleLightShadowMapCoordValue(vec2 coords, int lightIndex)
{
	float closestDepth;
	if (lightIndex == 0)
	{
		closestDepth = texture(simpleLightTextures[0], coords).r;
	}
	else
	{
		closestDepth = texture(simpleLightTextures[1], coords).r;
	}
	return closestDepth;
}

float getCubeLightShadowMapCoordValue(vec3 coords, int lightIndex)
{
	float closestDepth;
	if (lightIndex == 0)
	{
		closestDepth = texture(cubeLightTextures[0], coords).r;
	}
	else if (lightIndex == 1)
	{
		closestDepth = texture(cubeLightTextures[1], coords).r;
	}
	else if (lightIndex == 2)
	{
		closestDepth = texture(cubeLightTextures[2], coords).r;
	}
	else if (lightIndex == 3)
	{
		closestDepth = texture(cubeLightTextures[3], coords).r;
	}
	else if (lightIndex == 4)
	{
		closestDepth = texture(cubeLightTextures[4], coords).r;
	}
	else if (lightIndex == 5)
	{
		closestDepth = texture(cubeLightTextures[5], coords).r;
	}
	else if (lightIndex == 6)
	{
		closestDepth = texture(cubeLightTextures[6], coords).r;
	}
	else
	{
		closestDepth = texture(cubeLightTextures[7], coords).r;
	}
	return closestDepth * cubeLightDetails_fragment[lightIndex].farPlane;
}

float getSimpleLightAverageVisibility(vec2 depthMapCoords, float currentDepth, int lightIndex)
{
  float visibility = 0.0;
	float closestDepth = getSimpleLightShadowMapCoordValue(depthMapCoords, lightIndex);
	visibility += currentDepth - simpleLightAcneBias > closestDepth ? 0.0 : 1.0;
  return visibility;
}

float getCubeLightAverageVisibility(vec3 depthMapCoords, float currentDepth, int lightIndex)
{
  float visibility = 0.0;
	float closestDepth = getCubeLightShadowMapCoordValue(depthMapCoords, lightIndex);
	visibility += currentDepth - cubeLightAcneBias > closestDepth ? 0.0 : 1.0;
  return visibility;
}


vec3 getSimpleLightDiffuseLighting(int lightIndex)
{
  vec3 lightColorIntensity = simpleLightDetails_fragment[lightIndex].lightColor * simpleLightDetails_fragment[lightIndex].lightIntensity;
  float distanceFromLight = distance(vertexPosition_worldSpace, vec4(simpleLightDetails_fragment[lightIndex].lightPosition, 1.0));

  float diffuseStrength = clamp(dot(vertexNormal_viewSpace, simpleLightDirection_viewSpace[lightIndex]), 0.0, 1.0);
  return (lightColorIntensity * diffuseStrength) / (distanceFromLight * distanceFromLight);
}

vec3 getCubeLightDiffuseLighting(int lightIndex)
{
	vec3 lightColorIntensity = cubeLightDetails_fragment[lightIndex].lightColor * cubeLightDetails_fragment[lightIndex].lightIntensity;
  float distanceFromLight = distance(vertexPosition_worldSpace, vec4(cubeLightDetails_fragment[lightIndex].lightPosition, 1.0));

  float diffuseStrength = clamp(dot(vertexNormal_viewSpace, cubeLightDirection_viewSpace[lightIndex]), 0.0, 1.0);
  return (lightColorIntensity * diffuseStrength) / (distanceFromLight * distanceFromLight);
}


vec3 getSimpleLightSpecularLighting(int lightIndex)
{
  vec3 lightColorIntensity = simpleLightDetails_fragment[lightIndex].lightColor * simpleLightDetails_fragment[lightIndex].lightIntensity;
  float distanceFromLight = distance(vertexPosition_worldSpace, vec4(simpleLightDetails_fragment[lightIndex].lightPosition, 1.0));

	vec3 viewDirection_viewSpace = normalize(vertexPosition_viewSpace.xyz - vec3(0.0, 0.0, 0.0));
	highp vec3 lightReflection_viewSpace = reflect(simpleLightDirection_viewSpace[lightIndex], vertexNormal_viewSpace);

  float specularStrength = clamp(dot(viewDirection_viewSpace, lightReflection_viewSpace), 0.0, 1.0);
  vec3 specularLight = (lightColorIntensity * pow(specularStrength, specularLobeFactor)) / (distanceFromLight * distanceFromLight);

	return specularReflectivity * specularLight;
}

vec3 getCubeLightSpecularLighting(int lightIndex)
{
	vec3 lightColorIntensity = cubeLightDetails_fragment[lightIndex].lightColor * cubeLightDetails_fragment[lightIndex].lightIntensity;
  float distanceFromLight = distance(vertexPosition_worldSpace, vec4(cubeLightDetails_fragment[lightIndex].lightPosition, 1.0));

	vec3 viewDirection_viewSpace = normalize(vertexPosition_viewSpace.xyz - vec3(0.0, 0.0, 0.0));
	highp vec3 lightReflection_viewSpace = reflect(cubeLightDirection_viewSpace[lightIndex], vertexNormal_viewSpace);

  float specularStrength = clamp(dot(viewDirection_viewSpace, lightReflection_viewSpace), 0.0, 1.0);
  vec3 specularLight = (lightColorIntensity * pow(specularStrength, specularLobeFactor)) / (distanceFromLight * distanceFromLight);

	return specularReflectivity * specularLight;
}

void main()
{
	vec3 surfaceColor = texture(diffuseTexture, fragmentUv).rgb;
	color = surfaceColor * clamp(ambientFactor + clamp(disableFeatureMask - 1, 0, 1), 0.0, 1.0);

	if (disableFeatureMask < DISABLE_LIGHT)
	{
		for (int i = 0; i < simpleLightsCount; i++)
		{
			float visibility;
			if (disableFeatureMask < DISABLE_SHADOW)
			{
				vec3 depthMapCoords = (((simpleLightDepthCoord[i].xyz) / simpleLightDepthCoord[i].w) * 0.5) + 0.5;
				visibility = getSimpleLightAverageVisibility(depthMapCoords.xy, depthMapCoords.z, i);
			}
			else
			{
				visibility = 1.0;
			}
			color += visibility * surfaceColor * getSimpleLightDiffuseLighting(i);
			color += visibility * getSimpleLightSpecularLighting(i);
		}
		for (int i = 0; i < cubeLightsCount; i++)
		{
			float visibility;
			if (disableFeatureMask < DISABLE_SHADOW)
			{
				vec3 depthMapCoords = vertexPosition_worldSpace.xyz - cubeLightDetails_fragment[i].lightPosition;
				visibility = getCubeLightAverageVisibility(depthMapCoords.xyz, length(depthMapCoords), i);
			}
			else
			{
				visibility = 1.0;
			}
			color += visibility * surfaceColor * getCubeLightDiffuseLighting(i);
			color += visibility * getCubeLightSpecularLighting(i);
		}
	}
}