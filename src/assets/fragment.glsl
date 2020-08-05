#version 330 core

// We'll receive the details of the current fragment from the vertex shader through here.
// For fragments in between vertices, inside the primitive shape being drawn by the GPU
//   (triangles in our case), it will use the distance of the vertices of that shape as
//   weights for linearly interpolating what the value of this output should be for the
//   fragment that is in-between those vertices and inside the shape.
in vec2 fragmentUv;
in vec3 fragmentPosition_viewSpace;
in vec3 fragmentNormal_viewSpace;

// The output variable through which we'll be returning the final color of the fragment.
out vec3 color;

// Define the uniform for the view matrix.
uniform mat4 viewMatrix;

// Define the uniform for the light position in world-space.
uniform vec3 lightPosition_worldSpace;
// Define the uniform for the camera position in world-space.
uniform vec3 cameraPosition_worldSpace;

// Define the uniform for the 2D texture sampler of the diffuse texture of the object.
uniform sampler2D cubeTexture;

// Define the uniform for the light color.
uniform vec3 lightColor;
// Define the uniform for the light intensity.
uniform float lightIntensity;

// Define the uniform for the models' specular reflectivity.
uniform float specularReflectivity;
// Define the uniform for the models' specular lobe factor.
uniform float specularLobeFactor;

// Define the uniform for the ambient factor of the scene.
uniform float sceneAmbientFactor;

void main()
{
  // Get the fragment's diffuse/surface color from the diffuse texture.
  vec3 surfaceColor = texture(cubeTexture, fragmentUv).rgb;
  
  // Calculate the ambient lighting on the object by multiplying the ambient factor against the color of the surface.
  vec3 ambientLighting = sceneAmbientFactor * surfaceColor;
  
  // Calculate the direction of the light source in view-space.
  vec3 lightDirection_viewSpace = (viewMatrix * vec4(lightPosition_worldSpace, 1.0)).xyz - fragmentPosition_viewSpace;
  // Calculate the distance of the light from the fragment.
  float distanceFromLight = length(lightDirection_viewSpace);
  // Calculate the product of the color and intensity of the light.
  vec3 lightColorIntensity = lightColor * lightIntensity;
  // Calculate the squared distance of the light from the fragment.
  float distanceFromLightSquared = distanceFromLight * distanceFromLight;

  // Calculate the strength of the diffuse lighting on the fragment, based on the angle of the direction of the light against the normal of the surface.
  // The closer the two, the lower the diffusion strength.
  float diffuseStrength = clamp(dot(normalize(lightDirection_viewSpace), normalize(fragmentNormal_viewSpace)), 0.0, 1.0);
  // Calculate the diffuse lighting on the object by multiplying the diffuse strength against the light's color and intensity, and the color of the surface,
  // and divide it by the distance from the light squared (due to the square-inverse law describing how the strength of light falls of with increase in distance).
  vec3 diffuseLighting = (lightColorIntensity * diffuseStrength * surfaceColor) / distanceFromLightSquared;

  // Calculate the reflection vector of the light from the fragment, using the normal direction of the fragment.
  vec3 lightReflection_viewSpace = normalize(reflect(lightDirection_viewSpace, fragmentNormal_viewSpace));
  // Calculate the direction of the fragment to the camera in view-space.
  vec3 cameraDirection_viewSpace = normalize(fragmentPosition_viewSpace - vec3(0.0, 0.0, 0.0));
  // Calculate the strength of the specular lighting on the fragment, based on the angle of the direction of the light against the direction of the camera from the fragment.
  // The closer the two, the lower the diffusion strength.
  float specularStrength = specularReflectivity * clamp(dot(lightReflection_viewSpace, cameraDirection_viewSpace), 0.0, 1.0);
  // Calculate the specular lighting on the object by increasing the specular strength value by the power of the specular lobe factor,
  // multiplying against the light's color and intensity, and divide it by the distance from the light squared
  // (due to the square-inverse law describing how the strength of light falls of with increase in distance).
  vec3 specularLighting = (lightColorIntensity * pow(specularStrength, specularLobeFactor)) / distanceFromLightSquared;

  // Calculate the final of the surface by combining the lighting values calculated.
  color = ambientLighting + diffuseLighting + specularLighting;
}