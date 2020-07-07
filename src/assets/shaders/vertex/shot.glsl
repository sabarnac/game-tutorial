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

// The vertex position attribute of the model.
layout(location = 0) in vec3 vertexPosition;
// The vertex UV coordinate attribute of the model.
layout(location = 1) in vec2 vertexUv;
// The vertex normal vector attribute of the model.
layout(location = 2) in vec3 vertexNormal;

// The UV coordinates of the diffuse color of the fragment in the shot object texture.
// This is interpolated by the GPU for the fragment when passed from the vertex shader.
out vec2 fragmentUv;


// The structure defining the details regarding the model.
struct ModelDetails
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
};

// The details of the model.
uniform ModelDetails modelDetails;

void main()
{
	// Transform the model vertex into world-space, and then further transform it
	//   based on the view and projection of the camera, and return that as the
	//   vertex position.
	gl_Position = modelDetails.projectionMatrix * modelDetails.viewMatrix * modelDetails.modelMatrix * vec4(vertexPosition, 1.0);

	// Set the value of the UV coordinate of all fragments that are interpolated through this vertex.
	fragmentUv = vertexUv;
}