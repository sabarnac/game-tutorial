#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

struct LightDetails_Geometry
{
	vec3 lightPosition;
	mat4 vpMatrices[6];
	int vpMatrixCount;
};

uniform LightDetails_Geometry lightDetails_geometry;

out vec4 fragmentPosition;

void main()
{
    for(int face = 0; face < lightDetails_geometry.vpMatrixCount; ++face)
    {
        gl_Layer = face;
        for(int i = 0; i < 3; ++i)
        {
            fragmentPosition = gl_in[i].gl_Position;
            gl_Position = lightDetails_geometry.vpMatrices[face] * fragmentPosition;
            EmitVertex();
        }
        EndPrimitive();
    }
}