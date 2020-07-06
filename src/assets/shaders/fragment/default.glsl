#version 330 core

in vec2 fragmentUv;
in vec4 vertexPosition_worldSpace;
in vec3 vertexNormal_viewSpace;

in vec4 simpleLightDepthCoord[8];
in vec3 simpleLightDirection_viewSpace[8];
in vec3 cubeLightDirection_viewSpace[8];

out vec3 color;


struct LightDetails_Fragment
{
	vec3 lightPosition;
	mat4 lightVpMatrix;
	vec3 lightColor;
	float lightIntensity;
	int mapWidth;
	int mapHeight;
	float nearPlane;
	float farPlane;
};

uniform sampler2D diffuseTexture;

uniform sampler2D simpleLightTextures[7];
uniform samplerCube cubeLightTextures[8];

uniform LightDetails_Fragment simpleLightDetails_fragment[7];
uniform LightDetails_Fragment cubeLightDetails_fragment[8];

uniform int simpleLightsCount;
uniform int cubeLightsCount;

uniform float ambientFactor;

float simpleLightAcneBias = 0.0001;
float cubeLightAcneBias = 0.0;

vec2 getSimpleLightShadowMapTexelSize(int lightIndex)
{
	vec2 texelSize;
	if (lightIndex == 0)
	{
		texelSize = 1.0 / textureSize(simpleLightTextures[0], 0);
	}
	else if (lightIndex == 1)
	{
		texelSize = 1.0 / textureSize(simpleLightTextures[1], 0);
	}
	else if (lightIndex == 2)
	{
		texelSize = 1.0 / textureSize(simpleLightTextures[2], 0);
	}
	else if (lightIndex == 3)
	{
		texelSize = 1.0 / textureSize(simpleLightTextures[3], 0);
	}
	else if (lightIndex == 4)
	{
		texelSize = 1.0 / textureSize(simpleLightTextures[4], 0);
	}
	else if (lightIndex == 5)
	{
		texelSize = 1.0 / textureSize(simpleLightTextures[5], 0);
	}
	else
	{
		texelSize = 1.0 / textureSize(simpleLightTextures[6], 0);
	}
	return texelSize;
}

float getSimpleLightShadowMapCoordValue(vec2 coords, int lightIndex)
{
	float closestDepth;
	if (lightIndex == 0)
	{
		closestDepth = texture(simpleLightTextures[0], coords).r;
	}
	else if (lightIndex == 1)
	{
		closestDepth = texture(simpleLightTextures[1], coords).r;
	}
	else if (lightIndex == 2)
	{
		closestDepth = texture(simpleLightTextures[2], coords).r;
	}
	else if (lightIndex == 3)
	{
		closestDepth = texture(simpleLightTextures[3], coords).r;
	}
	else if (lightIndex == 4)
	{
		closestDepth = texture(simpleLightTextures[4], coords).r;
	}
	else if (lightIndex == 5)
	{
		closestDepth = texture(simpleLightTextures[5], coords).r;
	}
	else
	{
		closestDepth = texture(simpleLightTextures[6], coords).r;
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
	vec2 texelSize = getSimpleLightShadowMapTexelSize(lightIndex);
  float visibility = 0.0;
  for (int x = -1; x <= 1; x++)
	{
    for (int y = -1; y <= 1; y++)
		{
      float closestDepth = getSimpleLightShadowMapCoordValue(depthMapCoords + (vec2(x, y) * texelSize), lightIndex);
      visibility += currentDepth - simpleLightAcneBias > closestDepth ? 0.0 : 1.0;
    }
  }
  return visibility / 9.0;
}

float getCubeLightAverageVisibility(vec3 depthMapCoords, float currentDepth, int lightIndex)
{
  float visibility = 0.0;
  for (int x = -1; x <= 1; x++)
	{
    for (int y = -1; y <= 1; y++)
		{
	    for (int z = -1; z <= 1; z++)
			{
    	  float closestDepth = getCubeLightShadowMapCoordValue(depthMapCoords + vec3(x * 0.01, y * 0.01, z * 0.01), lightIndex);
      	visibility += currentDepth - cubeLightAcneBias > closestDepth ? 0.0 : 1.0;
			}
    }
  }
  return visibility / 27.0;
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

void main()
{
	vec3 surfaceColor = texture(diffuseTexture, fragmentUv).rgb;
	color = surfaceColor * ambientFactor;

	for (int i = 0; i < simpleLightsCount; i++)
	{
		vec3 depthMapCoords = (((simpleLightDepthCoord[i].xyz) / simpleLightDepthCoord[i].w) * 0.5) + 0.5;
		float visibility = getSimpleLightAverageVisibility(depthMapCoords.xy, depthMapCoords.z, i);
		color += visibility * surfaceColor * getSimpleLightDiffuseLighting(i);
	}
	for (int i = 0; i < cubeLightsCount; i++)
	{
		vec3 depthMapCoords = vertexPosition_worldSpace.xyz - cubeLightDetails_fragment[i].lightPosition;
		float visibility = getCubeLightAverageVisibility(vec3(0.0, 0.0, 0.0), 2.0, i);
		color += visibility * surfaceColor * getCubeLightDiffuseLighting(i);
	}
}