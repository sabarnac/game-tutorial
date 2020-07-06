#version 330 core

layout(location = 0) in vec3 vertexPosition;

struct LightDetails_Vertex
{
	vec3 lightPosition;
	mat4 vpMatrices[1];
	int vpMatrixCount;
};

uniform mat4 modelMatrix;
uniform LightDetails_Vertex lightDetails_vertex;

void main()
{
	gl_Position =  lightDetails_vertex.vpMatrices[0] * modelMatrix * vec4(vertexPosition, 1.0);
}