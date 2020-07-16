#version 330 core

// We'll receive the position of a single vertex here.
layout(location = 0) in vec3 vertexPosition;

// We'll output the position of the current vertex to the fragment shader through here.
// For fragments in between vertices, inside the primitive shape being drawn by the GPU
//   (triangles in our case), it will use the distance of the vertices of that shape as
//   weights for linearly interpolating what the value of this output should be for the
//   fragment that is in-between those vertices and inside the shape.
out vec3 fragmentPosition;

void main()
{
  // Set the position of the vertex the same as the vertex position we retrieved.
  gl_Position = vec4(vertexPosition, 1.0);
  // Set the position of the fragment as the position of our vertex. The GPU will linearly
  //   interpolate this value to determine what is the correct value for each fragment.
  fragmentPosition = vertexPosition;
}