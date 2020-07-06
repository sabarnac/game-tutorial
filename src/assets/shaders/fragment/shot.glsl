#version 330 core

in vec2 fragmentUv;

out vec3 color;

uniform sampler2D diffuseTexture;

void main()
{
	color = texture(diffuseTexture, fragmentUv).rgb;
}