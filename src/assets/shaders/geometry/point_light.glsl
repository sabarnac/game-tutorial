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

#define MAX_LIGHTS 5

// The type of the primitive being accepted by the geometry shader.
layout (triangles) in;
// The type of the primitive being outputed by the geometry shader.
layout (triangle_strip, max_vertices=18) out;

// The structure defining the details regarding the light.
struct LightDetails_Geometry
{
  int layerId;
  vec3 lightPosition;
  mat4 vpMatrices[6];
  int vpMatrixCount;
};

// The details of the current light.
uniform LightDetails_Geometry lightDetails_geometry[MAX_LIGHTS];
uniform int lightsCount;

// The vertex position being used to interpolate fragments.
out vec4 fragmentPosition;
// The index of the light for that fragment.
out float lightIndex;

void main()
{
  for(int light = 0; light < lightsCount; light++)
  {
    // Generate the vertex position for each face of the light's shadow map.
    // For point lights, this will be 6 faces, for other maps it is just 1 face.
    for(int face = 0; face < lightDetails_geometry[light].vpMatrixCount; ++face)
    {
      // Set the current layer of the shadow map being modified.
      // Since shadow maps of lights are saved in arrays, we need to make sure that
      //   the current positions are saved only for the layer in the array that this
      //   light is associated with. So we set the base layer value as the layer
      //   ID sent through us through the light details uniform, and then also add
      //   in the ID of the current face we're working on, giving us the final layer-face
      //   value to tell the geometry shader to operate in.
      gl_Layer = lightDetails_geometry[light].layerId + face;
      // Iterate through each vertex in the input triangle.
      for(int i = 0; i < 3; ++i)
      {
        // Set the vertex position used to interpolate fragments to the position
        //   of the vertex in the incoming triangle.
        fragmentPosition = gl_in[i].gl_Position;
        lightIndex = float(light);
        // Transform the position of the model vertex using the view and projection
        //   matrices of the light.
        gl_Position = lightDetails_geometry[light].vpMatrices[face] * fragmentPosition;
        // Emit the resultant model vertex.
        EmitVertex();
      }
      // Emit the resultant model primitive.
      EndPrimitive();
    }
  }
}