#version 330 core

layout (location = 0) in vec2 vertexPosition;
layout (location = 1) in vec2 vertexUv;
layout (location = 2) in float vertexUvLayer;

out vec2 fragmentUv;
out float fragmentUvLayer;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertexPosition.xy, 1.0, 1.0);
    fragmentUv = vertexUv;
    fragmentUvLayer = vertexUvLayer;
}