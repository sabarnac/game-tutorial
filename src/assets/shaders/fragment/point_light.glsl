#version 330 core

in vec4 fragmentPosition;

struct LightDetails_Fragment
{
	vec3 lightPosition;
	mat4 vpMatrices[6];
	int vpMatrixCount;
};

struct ProjectionDetails_Fragment
{
  float nearPlane;
  float farPlane;
};

uniform LightDetails_Fragment lightDetails_fragment;
uniform ProjectionDetails_Fragment projectionDetails_fragment;

void main()
{
    float lightDistance = length(fragmentPosition.xyz - lightDetails_fragment.lightPosition);
    
    lightDistance = lightDistance / projectionDetails_fragment.farPlane;
    
    gl_FragDepth = lightDistance;
}