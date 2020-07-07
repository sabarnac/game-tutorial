#version 330 core

// The reason for suffixing structures and uniform variables with
//   the shader component name, is so that they don't collide with
//   definitions in other shaders.
// GPU shader compilers optimize and remove any unused variables,
//   and if there are different unused variables in the same structure
//   definition in different shader components, with both being used
//   through the same variable, then the shader first deletes the unused
//   variables in the initial shader component compilation step, then
//   fails to link the two shader components together because their
//   structures are now different.
// Note that for primitive uniform variables this cannot be an issue,
//   because there is no structure to change. Either the entire
//   variable is kept as is, or completely removed.

// The type of the primitive being accepted by the geometry shader.
layout (triangles) in;
// The type of the primitive being outputed by the geometry shader.
layout (triangle_strip, max_vertices=18) out;

// The structure defining the details regarding the cone light.
struct LightDetails_Geometry
{
	vec3 lightPosition;
	mat4 vpMatrices[6];
	int vpMatrixCount;
};

// The details of the current cone light.
uniform LightDetails_Geometry lightDetails_geometry;

// The vertex position being used to interpolate fragments.
out vec4 fragmentPosition;

void main()
{
    // Generate the vertex position for each face of the point light's cube map.
    for(int face = 0; face < lightDetails_geometry.vpMatrixCount; ++face)
    {
        // Set the current face of the cube being modified.
        gl_Layer = face;
        // Iterate through each vertex in the input triangle.
        for(int i = 0; i < 3; ++i)
        {
            // Set the vertex position used to interpolate fragments to the position
            //   of the vertex in the incoming triangle.
            fragmentPosition = gl_in[i].gl_Position;
            // Transform the position of the model vertex using the view and projection
            //   matrices of the point light.
            gl_Position = lightDetails_geometry.vpMatrices[face] * fragmentPosition;
            // Emit the resultant model vertex.
            EmitVertex();
        }
        // Emit the resultant model primitive.
        EndPrimitive();
    }
}