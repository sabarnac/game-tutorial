#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexUv;
layout(location = 2) in vec3 vertexNormal;

out vec2 fragmentUv;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 mvpMatrix;

uniform float totalTime;
uniform float deltaTime;

void main(){	
	gl_Position = mvpMatrix * vec4(vertexPosition, 1.0);

	fragmentUv = vertexUv;
}

