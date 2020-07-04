#version 330 core

layout(location = 0) in vec3 vertexPosition;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main(){	
	gl_Position = projectionMatrix * viewMatrix * mat4(1.0) * vec4(vertexPosition, 1.0);
}

