#version 330 core

// We receive the position of the current fragment, being interpolated by the GPU using the
//   outputs of the vertices that define the shape the fragment is inside.
in vec3 fragmentPosition;

// The output variable through which we'll be returning the final color of the fragment.
out vec3 color;

void main()
{
  // We'll set the color of the fragment as the position of the fragment. This way we can visualize
  //   the positions of all the fragments on the screen.
  // Note that the output colors have to be provided as a float value between the range of 0-1.
  // Values lower than 0 are black, higher than 1 are white.
  color = fragmentPosition;
}