#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexUv;
layout(location = 2) in vec3 vertexNormal;

out vec2 fragmentUv;
out vec4 vertexPosition_worldSpace;
out vec4 vertexPosition_viewSpace;
out vec3 vertexNormal_viewSpace;

out vec4 simpleLightDepthCoord[8];
out vec3 simpleLightDirection_viewSpace[8];
out vec3 cubeLightDirection_viewSpace[8];


struct ModelDetails
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
	mat4 mvpMatrix;
};

struct TimeDetails
{
	float totalTime;
	float deltaTime;
};

struct LightDetails_Vertex
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

uniform ModelDetails modelDetails;

uniform LightDetails_Vertex simpleLightDetails_vertex[4];
uniform LightDetails_Vertex cubeLightDetails_vertex[6];

uniform int simpleLightsCount;
uniform int cubeLightsCount;

void main()
{
	vertexPosition_worldSpace = modelDetails.modelMatrix * vec4(vertexPosition, 1.0);
	vertexPosition_viewSpace = modelDetails.viewMatrix * vertexPosition_worldSpace;

	gl_Position = modelDetails.projectionMatrix * vertexPosition_viewSpace;

	fragmentUv = vertexUv;
	vertexNormal_viewSpace = (modelDetails.viewMatrix * modelDetails.modelMatrix * vec4(vertexNormal, 0.0)).xyz;

	for (int i = 0; i < simpleLightsCount; i++)
	{
		vec4 lightPosition_viewSpace = modelDetails.viewMatrix * vec4(simpleLightDetails_vertex[i].lightPosition, 1.0);
		simpleLightDirection_viewSpace[i] = normalize((lightPosition_viewSpace - vertexPosition_viewSpace).xyz);
		simpleLightDepthCoord[i] = simpleLightDetails_vertex[i].lightVpMatrix * modelDetails.modelMatrix * vec4(vertexPosition, 1.0);
	}

	for (int i = 0; i < cubeLightsCount; i++)
	{
		vec4 lightPosition_viewSpace = modelDetails.viewMatrix * vec4(cubeLightDetails_vertex[i].lightPosition, 1.0);
		cubeLightDirection_viewSpace[i] = normalize((lightPosition_viewSpace - vertexPosition_viewSpace).xyz);
	}
}