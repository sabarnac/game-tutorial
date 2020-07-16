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

// The position of the fragment as interpolated by the GPU from the geometry shader.
in vec4 fragmentPosition;

// The color of the fragment.
out vec4 color;

void main()
{
  // The fragment position variable needs to be used, so just assigning it to the output
  //   color (this value will be ignored by the framebuffer).
  color = fragmentPosition;
  // Don't need to do anything. We're capturing the depth component,
  //   which is already set by the fragment shader since it interpolated
  //   it from the position values returned by the vertex shader.
}