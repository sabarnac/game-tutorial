#version 330 core

// We'll receive the details of a single vertex here.
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexUv;
layout(location = 2) in vec3 vertexNormal;

// We'll output the details of the current vertex to the fragment shader through here.
// For fragments in between vertices, inside the primitive shape being drawn by the GPU
//   (triangles in our case), it will use the distance of the vertices of that shape as
//   weights for linearly interpolating what the value of this output should be for the
//   fragment that is in-between those vertices and inside the shape.
out vec2 fragmentUv;
out vec3 fragmentPosition_viewSpace;
out vec3 fragmentNormal_viewSpace;

// Define the uniform for the model matrix.
uniform mat4 modelMatrix;
// Define the uniform for the view matrix.
uniform mat4 viewMatrix;
// Define the uniform for the projection matrix.
uniform mat4 projectionMatrix;

void main()
{
  // Calculate the position of the vertex in view-space.
  vec4 vertexPosition_viewSpace = viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);
  // Calculate the position of the in clip-space.
  gl_Position = projectionMatrix * vertexPosition_viewSpace;

  // Set the fragment's UV position.
  fragmentUv = vertexUv;

  // Set the fragment's normal direction in view-space.
  fragmentNormal_viewSpace = (viewMatrix * modelMatrix * vec4(vertexNormal, 0.0)).xyz;
  // Set the fragment's position in view-space.
  fragmentPosition_viewSpace = vertexPosition_viewSpace.xyz;
}