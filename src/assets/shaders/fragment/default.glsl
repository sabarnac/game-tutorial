#version 330 core

in vec2 fragmentUv;

uniform sampler2D textureSampler;

out vec4 color;

void main(){
	color.rgb = texture(textureSampler, fragmentUv).rgb;
	color.a = 1.0;
}