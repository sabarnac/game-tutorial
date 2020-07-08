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

// The vertex position attribute of the model.
layout(location = 0) in vec3 vertexPosition;
// The vertex UV coordinate attribute of the model.
layout(location = 1) in vec2 vertexUv;
// The vertex normal vector attribute of the model.
layout(location = 2) in vec3 vertexNormal;

// The UV coordinates of the diffuse color of the fragment in the standard object texture.
// This is interpolated by the GPU for the fragment when passed from the vertex shader.
out vec2 fragmentUv;
// The coordinates of the fragment in the standard object model in world-space.
// This is interpolated by the GPU for the fragment when passed from the vertex shader.
out vec4 fragmentPosition_worldSpace;
// The coordinates of the fragment in the standard object model in view-space.
// This is interpolated by the GPU for the fragment when passed from the vertex shader.
out vec4 fragmentPosition_viewSpace;
// The normal vector of the fragment in the standard object model in view-space.
// This is interpolated by the GPU for the fragment when passed from the vertex shader.
out vec3 fragmentNormal_viewSpace;

// The shadow map coordinates of the current fragment w.r.t all the active simple lights.
out vec4 simpleLightShadowMapCoord[MAX_SIMPLE_LIGHTS];
// The direction of the light going towards the current fragment from of all the active
//   simple light sources.
out vec3 simpleLightDirection_viewSpace[MAX_SIMPLE_LIGHTS];
// The direction of the light going towards the current fragment from of all the active
//   cube light sources.
out vec3 cubeLightDirection_viewSpace[MAX_CUBE_LIGHTS];


// The structure defining the details regarding the model.
struct ModelDetails
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
};

// The structure defining the details regarding the active lights.
struct LightDetails_Vertex
{
	vec3 lightPosition;
	mat4 lightVpMatrix;
	vec3 lightColor;
	float lightIntensity;
	float nearPlane;
	float farPlane;
};

// The details of the model.
uniform ModelDetails modelDetails;

// The number of active simple lights (2D texture lights).
uniform int simpleLightsCount;
// The number of active cube lights (cubemap texture lights).
uniform int cubeLightsCount;

// The details of the active simple lights (2D texture lights).
uniform LightDetails_Vertex simpleLightDetails_vertex[MAX_SIMPLE_LIGHTS];
// The details of the active cube lights (cubemap texture lights).
uniform LightDetails_Vertex cubeLightDetails_vertex[MAX_CUBE_LIGHTS];

void main()
{
	// Calculate the position of the model vertex in world-space.
	vec4 vertexPosition_worldSpace = modelDetails.modelMatrix * vec4(vertexPosition, 1.0);
	// Calculate the position of the model vertex in view-space.
	vec4 vertexPosition_viewSpace = modelDetails.viewMatrix * vertexPosition_worldSpace;

	// Transform the model vertex from view-space using the projection matrix of the camera,
	//   and set that as the position of the vertex.
	gl_Position = modelDetails.projectionMatrix * vertexPosition_viewSpace;

	// Calculate the direction of the vertex normal in view-space.
	vec3 vertexNormal_viewSpace = (modelDetails.viewMatrix * modelDetails.modelMatrix * vec4(vertexNormal, 0.0)).xyz;

	// Set the value of the UV coordinate of all fragments that are interpolated through this vertex.
	fragmentUv = vertexUv;
	// Set the value of the world-space position of all fragments that are interpolated through this vertex.
	fragmentPosition_worldSpace = vertexPosition_worldSpace;
	// Set the value of the view-space position of all fragments that are interpolated through this vertex.
	fragmentPosition_viewSpace = vertexPosition_viewSpace;
	// Set the value of the view-space normal vector of all fragments that are interpolated through this vertex.
	fragmentNormal_viewSpace = vertexNormal_viewSpace;

	// Iterate through all the active simple lights.
	for (int i = 0; i < simpleLightsCount; i++)
	{
		// Calculate the position of the light in view-space.
		vec4 lightPosition_viewSpace = modelDetails.viewMatrix * vec4(simpleLightDetails_vertex[i].lightPosition, 1.0);
		// Calculate the direction of the light from the source to the interpolated fragment in view-space.
		simpleLightDirection_viewSpace[i] = normalize((lightPosition_viewSpace - vertexPosition_viewSpace).xyz);
		// Calculate the depth of the interpolated fragment w.r.t to the light source (without accounting for perspective division).
		simpleLightShadowMapCoord[i] = simpleLightDetails_vertex[i].lightVpMatrix * modelDetails.modelMatrix * vec4(vertexPosition, 1.0);
	}

	// Iterate through all the active cube lights.
	for (int i = 0; i < cubeLightsCount; i++)
	{
		// Calculate the position of the light in view-space.
		vec4 lightPosition_viewSpace = modelDetails.viewMatrix * vec4(cubeLightDetails_vertex[i].lightPosition, 1.0);
		// Calculate the direction of the light from the source to the interpolated fragment in view-space.
		cubeLightDirection_viewSpace[i] = normalize((lightPosition_viewSpace - vertexPosition_viewSpace).xyz);
	}
}