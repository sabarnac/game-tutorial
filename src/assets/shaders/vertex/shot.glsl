#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexUv;
layout(location = 2) in vec3 vertexNormal;

out vec2 fragmentUv;


struct ModelDetails
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
	mat4 mvpMatrix;
};

uniform ModelDetails modelDetails;

void main()
{
	gl_Position = modelDetails.projectionMatrix * modelDetails.viewMatrix * modelDetails.modelMatrix * vec4(vertexPosition, 1.0);

	fragmentUv = vertexUv;
}