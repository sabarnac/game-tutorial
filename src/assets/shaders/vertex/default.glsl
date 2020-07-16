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

#define MAX_SIMPLE_LIGHTS 10
#define MAX_CUBE_LIGHTS 10

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
// The normal vector of the fragment in the standard object model in view-space.
// This is interpolated by the GPU for the fragment when passed from the vertex shader.
out vec3 fragmentNormal_viewSpace;

// The shadow map coordinates of the current fragment w.r.t all the active cone lights.
out vec4 coneLightShadowMapCoord[MAX_SIMPLE_LIGHTS];


// The structure defining the details regarding the model.
struct ModelDetails_Vertex
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
};

// The structure defining the details regarding the active lights.
struct LightDetails_Vertex
{
	// vec3 lightPosition;
	mat4 lightVpMatrix;
	// vec3 lightColor;
	// float lightIntensity;
	// float nearPlane;
	// float farPlane;
	// int layerId;
};


// The details of the model.
uniform ModelDetails_Vertex modelDetails_vertex;

// The number of active cone lights (2D texture lights).
uniform int coneLightsCount;

// The details of the active cone lights (2D texture lights).
uniform LightDetails_Vertex coneLightDetails_vertex[MAX_SIMPLE_LIGHTS];

void main()
{
	// Calculate the position of the model vertex in world-space.
	vec4 vertexPosition_worldSpace = modelDetails_vertex.modelMatrix * vec4(vertexPosition, 1.0);

	// Transform the model vertex from view-space using the projection matrix of the camera,
	//   and set that as the position of the vertex.
	gl_Position = modelDetails_vertex.projectionMatrix * modelDetails_vertex.viewMatrix * vertexPosition_worldSpace;

	// Calculate the direction of the vertex normal in view-space.
	vec3 vertexNormal_viewSpace = (modelDetails_vertex.viewMatrix * modelDetails_vertex.modelMatrix * vec4(vertexNormal, 0.0)).xyz;

	// Set the value of the UV coordinate of all fragments that are interpolated through this vertex.
	fragmentUv = vertexUv;
	// Set the value of the world-space position of all fragments that are interpolated through this vertex.
	fragmentPosition_worldSpace = vertexPosition_worldSpace;
	// Set the value of the view-space normal vector of all fragments that are interpolated through this vertex.
	fragmentNormal_viewSpace = vertexNormal_viewSpace;

	// Iterate through all the active cone lights.
	for (int i = 0; i < coneLightsCount; i++)
	{
		// Calculate the depth of the interpolated fragment w.r.t to the light source (without accounting for perspective division).
		coneLightShadowMapCoord[i] = coneLightDetails_vertex[i].lightVpMatrix * modelDetails_vertex.modelMatrix * vec4(vertexPosition, 1.0);
	}
}