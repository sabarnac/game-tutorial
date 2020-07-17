#version 330 core

in vec2 fragmentUv;

// The output variable through which we'll be returning the final color of the fragment.
out vec3 color;

uniform sampler2D cubeTexture;

void main()
{
  color = texture(cubeTexture, fragmentUv).rgb;
}