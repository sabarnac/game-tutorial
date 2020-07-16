#version 330 core

in vec4 fragmentPosition;

// The reason for suffixing the structure and variables with the
//   shader component name, is so that they don't collide with
//   definitions in other shaders.
// GPU shader compilers optimize and remove any unused variables,
//   and if there are different unused variables in the same structure
//   definition in different shader components, with both being used
//   through the same variable, then the shader first deletes the unused
//   variables in the initial shader component compilation step, then
//   fails to link the two shader components together because their
//   structures are now different.

// The structure defining the details regarding the point light.
struct LightDetails_Fragment
{
  int layerId;
	vec3 lightPosition;
	mat4 vpMatrices[6];
	int vpMatrixCount;
};

// The structure defining the details regarding the projection of
//   the point light.
struct ProjectionDetails_Fragment
{
  float nearPlane;
  float farPlane;
};

// The details of the current point light.
uniform LightDetails_Fragment lightDetails_fragment;
// The details of the current point light.
uniform ProjectionDetails_Fragment projectionDetails_fragment;

void main()
{
  // Calculate the distance of the fragment from the light source.
  float lightDistance = length(fragmentPosition.xyz - lightDetails_fragment.lightPosition);
  
  // Normalize the light distance against the farthest distance the
  //   light can go till (as defined by the far plane).
  // This makes the light distance a percentage value against the max
  //   distance the light can travel.
  lightDistance = lightDistance / projectionDetails_fragment.farPlane;
  
  // Set the fragment depth to the normalized light distance. This
  //   makes it easier to process the shadow map.
  gl_FragDepth = lightDistance;
}