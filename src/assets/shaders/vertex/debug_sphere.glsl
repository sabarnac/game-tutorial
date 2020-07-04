#version 330 core

layout(location = 0) in vec3 vertexPosition;

uniform mat4 mvpMatrix;
uniform float radius;

void main(){	
	gl_Position = mvpMatrix * vec4(normalize(vertexPosition) * radius, 1.0);
}

