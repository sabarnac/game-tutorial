#version 330 core

in vec2 fragmentUv;
in float fragmentUvLayer;

out vec4 color;

uniform sampler2DArray textTexture;

void main()
{    
    vec4 textSample = vec4(1.0, 1.0, 1.0, texture(textTexture, vec3(fragmentUv, fragmentUvLayer)).r);
    color = textSample;
}