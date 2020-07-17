#version 330 core

// We'll receive the position of a single vertex here.
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexUv;

out vec2 fragmentUv;

// We'll output the position of the current vertex to the fragment shader through here.
// For fragments in between vertices, inside the primitive shape being drawn by the GPU
//   (triangles in our case), it will use the distance of the vertices of that shape as
//   weights for linearly interpolating what the value of this output should be for the
//   fragment that is in-between those vertices and inside the shape.
uniform mat4 mvpMatrix;

void main()
{
  // Set the position of the vertex the same as the vertex position we retrieved.
  gl_Position = mvpMatrix * vec4(vertexPosition, 1.0);

  fragmentUv = vertexUv;
}