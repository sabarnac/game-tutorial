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

// The vertex position attribute of the model (already in world-space).
layout(location = 0) in vec3 vertexPosition;

// The view matrix of the camera.
uniform mat4 viewMatrix;
// The projection matrix of the camera.
uniform mat4 projectionMatrix;

void main()
{
	// Transform the model vertex based on the view and projection of the camera,
	//   and return that as the vertex position.
	gl_Position = projectionMatrix * viewMatrix * vec4(vertexPosition, 1.0);
}

