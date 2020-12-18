#version 330 core

// The reason for suffixing the structure and variables with the
//   shader component name, is so that they don't collide with
//   definitions in other shaders.
// GPU shader compilers optimize and remove any unused variables,
//   and if there are different unused variables in the same structure
//   definition in different shader components, with both being used
//   through the same variable, then the shader first deletes the unused
//   variables in the initial shader component compilation step, then
//   fails to link the two shader components together because their
//   structures are now different.

// The UV coordinates of the diffuse color of the fragment in the shot object texture.
// This is interpolated by the GPU for the fragment when passed from the vertex shader.
in vec2 fragmentUv;

// The final color of the fragment.
out vec3 color;

// The shot object texture sampler.
uniform sampler2D diffuseTexture;

void main()
{
	// Grab the diffuse color defined in the shot tecture using the given UV coordinates,
	//   and set that as the color of the fragment.
	color = texture(diffuseTexture, fragmentUv).rgb;
}