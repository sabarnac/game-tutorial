#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "include/window.cpp"
#include "include/control.cpp"
#include "include/shader.cpp"
#include "include/object.cpp"
#include "include/texture.cpp"
#include "camera/perspective_camera.cpp"

using namespace glm;

int main(void)
{
		// Instantiate the required managers.
		auto &windowManager = WindowManager::getInstance();
		auto &controlManager = ControlManager::getInstance();

		// Create and bind a vertex array object so that vertex attribute operations can be stored and managed in them.
		GLuint vertexArrayID;
		glGenVertexArrays(1, &vertexArrayID);
		glBindVertexArray(vertexArrayID);

		// Get an instance of the shader manager and generate the shader program for the cube.
		auto &shaderManager = ShaderManager::getInstance();
		auto shaderDetails = shaderManager.createShaderProgram("CubeShader", "assets/vertex.glsl", "assets/fragment.glsl");

		// Get an instance of the object manager and read and store the object details of the cube.
		auto &objectManager = ObjectManager::getInstance();
		auto cubeObjectDetails = objectManager.createObject("CubeObject", "assets/objects/cube.obj");

		// Get an instance of the texture manager and read and store the surface image texture of the cube.
		auto &textureManager = TextureManager::getInstance();
		auto cubeTextureDetails = textureManager.create2dTexture("CubeTexture", "assets/textures/cube.bmp");

		// Define a matrix that represents how much the object should be scaled by (the scaling matrix - scale across x-axis, y-axis, z-axis).
		glm::mat4 scalingMatrix = glm::scale(glm::vec3(1, 1, 1));
		// Define a matrix that represents how much the object should be translated/moved by (the translation matrix - translate across x-axis, y-axis, z-axis).
		glm::mat4 translationMatrix = glm::translate(glm::vec3(-1, -2, -3));

		// Define the position of the light source in world-space.
		glm::vec3 lightPosition_worldSpace = glm::vec3(1, 2, 3);
		// Define the color of the light from the light source.
		glm::vec3 lightColor = glm::vec3(1, 1, 1);
		// Define the intensity of the light.
		float lightIntensity = 35;
		// Define the ambient factor of the light on the world. That means that objects will be at bright by this factor at minimum.
		float sceneAmbientFactor = 0.2;

		// Define the specular reflectivity of the model (how reflective the model is).
		// The range is from 0.0 to 1.0 (0.0 being not reflective, 1.0 being fully reflective)
		float modelSpecularReflectivity = 1.0;
		// Define the specular lobe factor of the model (how much the specular reflection spreads out).
		// The higher the value, the tighter the lobe.
		float modelSpecularLobeFactor = 35.0;

		// Create a perspective camera and initialize it.
		auto perspectiveCamera = PerspectiveCamera::create("mainCamera");
		perspectiveCamera->init();

		// Get the location ID of the uniform which represents the model matrix variable defined in the shader.
		auto modelMatrixUniformId = glGetUniformLocation(shaderDetails->getShaderId(), "modelMatrix");
		// Get the location ID of the uniform which represents the view matrix variable defined in the shader.
		auto viewMatrixUniformId = glGetUniformLocation(shaderDetails->getShaderId(), "viewMatrix");
		// Get the location ID of the uniform which represents the projection matrix variable defined in the shader.
		auto projectionMatrixUniformId = glGetUniformLocation(shaderDetails->getShaderId(), "projectionMatrix");

		// Get the location ID of the uniform which represents the light position in world-space variable defined in the shader.
		auto lightPosition_worldSpaceUniformId = glGetUniformLocation(shaderDetails->getShaderId(), "lightPosition_worldSpace");
		// Get the location ID of the uniform which represents the camera position in world-space variable defined in the shader.
		auto cameraPosition_worldSpaceUniformId = glGetUniformLocation(shaderDetails->getShaderId(), "cameraPosition_worldSpace");
		// Get the location ID of the uniform which represents the light color variable defined in the shader.
		auto lightColorUniformId = glGetUniformLocation(shaderDetails->getShaderId(), "lightColor");
		// Get the location ID of the uniform which represents the light intensity variable defined in the shader.
		auto lightIntensityUniformId = glGetUniformLocation(shaderDetails->getShaderId(), "lightIntensity");
		// Get the location ID of the uniform which represents the light ambient factor variable defined in the shader.
		auto sceneAmbientFactorUniformId = glGetUniformLocation(shaderDetails->getShaderId(), "sceneAmbientFactor");

		// Get the location ID of the uniform which represents the model specular reflectivity variable defined in the shader.
		auto specularReflectivityUniformId = glGetUniformLocation(shaderDetails->getShaderId(), "specularReflectivity");
		// Get the location ID of the uniform which represents the model lobe factor variable defined in the shader.
		auto specularLobeFactorUniformId = glGetUniformLocation(shaderDetails->getShaderId(), "specularLobeFactor");

		do
		{
				// Get the current time.
				auto currentTime = glfwGetTime();

				// Input Step.
				{
						perspectiveCamera->update();
				}

				// Render Step.
				{ // Clear the screen.
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

						// Use our created triangle shader program.
						glUseProgram(shaderDetails->getShaderId());

						// Assign the light position in world-space variable in the shader to the value that we defined.
						glUniform3f(lightPosition_worldSpaceUniformId, lightPosition_worldSpace.x, lightPosition_worldSpace.y, lightPosition_worldSpace.z);
						// Assign the camera position in world-space variable in the shader to the value that we defined.
						glUniform3f(cameraPosition_worldSpaceUniformId, perspectiveCamera->getCameraPosition().x, perspectiveCamera->getCameraPosition().y, perspectiveCamera->getCameraPosition().z);
						// Assign the light color variable in the shader to the value that we defined.
						glUniform3f(lightColorUniformId, lightColor.r, lightColor.g, lightColor.b);
						// Assign the light intensity variable in the shader to the value that we defined.
						glUniform1f(lightIntensityUniformId, lightIntensity);
						// Assign the light ambient factor variable in the shader to the value that we defined.
						glUniform1f(sceneAmbientFactorUniformId, sceneAmbientFactor);

						// Assign the model specular reflectivity variable in the shader to the value that we defined.
						glUniform1f(specularReflectivityUniformId, modelSpecularReflectivity);
						// Assign the model specular lobe factor variable in the shader to the value that we defined.
						glUniform1f(specularLobeFactorUniformId, modelSpecularLobeFactor);

						// Define a matrix that represents how much the object should be rotated by (the rotation matrix - rotation across x-axis, y-axis, z-axis).
						// We'll have this be based on current time, so that it rotates over the period of execution.
						glm::mat4 rotationMatrix = glm::toMat4(glm::quat(glm::vec3(currentTime / 5, (currentTime / 5) * 2, (currentTime / 5) * 3)));

						// Create the overall model transformation matrix (or just model matrix) by multiplying the individual sub-transformations in the order of:
						//   1. Scaling
						//   2. Rotation
						//   3. Translation
						// In code, this multiplication is done in the reverse order, since the matrix is multiplied against the vertex, which will be at the right side of the operation.
						// This would result in the right side of this order having precedence with being multiplied against the vertex to change its position.
						// This will transform the vertex from a position relative to the model, to its position in the world (called world-space).
						glm::mat4 modelMatrix = translationMatrix * rotationMatrix * scalingMatrix;

						// Assign the model matrix variable in the shader the model matrix result that we calculated.
						// We flag that we are only passing 1 matrix to the GPU, and that it should not be transposed before sending.
						glUniformMatrix4fv(modelMatrixUniformId, 1, GL_FALSE, &modelMatrix[0][0]);
						// Assign the view matrix variable in the shader the model matrix result that we calculated. Flagged similarly as the model matrix.
						glUniformMatrix4fv(viewMatrixUniformId, 1, GL_FALSE, &(perspectiveCamera->getViewMatrix())[0][0]);
						// Assign the projection matrix variable in the shader the model matrix result that we calculated. Flagged similarly as the model matrix.
						glUniformMatrix4fv(projectionMatrixUniformId, 1, GL_FALSE, &(perspectiveCamera->getProjectionMatrix())[0][0]);

						// Activate the 0th texture unit, which can then read data from the cube texture.
						glActiveTexture(GL_TEXTURE0);
						// Bind the cube texture as a 2D texture to the 0th texture unit (which is now the immediate context for the texture operation).
						glBindTexture(GL_TEXTURE_2D, cubeTextureDetails->getTextureId());
						// Get the location ID of the uniform which represents the texture sampler variable defined in the shader.
						auto cubeTextureUniformId = glGetUniformLocation(shaderDetails->getShaderId(), "cubeTexture");
						// Assign the texture sampler variable in the shader a value of 0, so that it will read texture data from the 0th texture unit.
						glUniform1i(cubeTextureUniformId, 0);

						// Enable the first vertex attribute array, which will carry our triangle vertices position information and loop through them.
						glEnableVertexAttribArray(0);
						// Bind the vertex buffer as an array buffer to the first vertex attribute array (which is now the immediate context for the buffer operation).
						glBindBuffer(GL_ARRAY_BUFFER, cubeObjectDetails->getVertexBufferId());
						// Define the attributes of the data being sent through the first vertex attribute array buffer.
						glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

						// Enable the second vertex attribute array, which will carry our triangle vertices UV coordinate information and loop through them.
						glEnableVertexAttribArray(1);
						// Bind the UV coordinates buffer as an array buffer to the second vertex attribute array (which is now the immediate context for the buffer operation).
						glBindBuffer(GL_ARRAY_BUFFER, cubeObjectDetails->getUvBufferId());
						// Define the attributes of the data being sent through the second vertex attribute array buffer.
						glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

						// Enable the second vertex attribute array, which will carry our triangle vertices UV coordinate information and loop through them.
						glEnableVertexAttribArray(2);
						// Bind the UV coordinates buffer as an array buffer to the second vertex attribute array (which is now the immediate context for the buffer operation).
						glBindBuffer(GL_ARRAY_BUFFER, cubeObjectDetails->getNormalBufferId());
						// Define the attributes of the data being sent through the second vertex attribute array buffer.
						glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

						// Draw the vertices of the object such that every 3 consecutive vertices draws a triangle.
						glDrawArrays(GL_TRIANGLES, 0, cubeObjectDetails->getBufferSize());

						// Disable the vertex attribute arrays since they're done being used.
						glDisableVertexAttribArray(2);
						glDisableVertexAttribArray(1);
						glDisableVertexAttribArray(0);

						// Swap the window buffers.
						windowManager.swapBuffers();
						// Poll for new control events.
						controlManager.pollEvents();
				}
		}
		// Check if we need to exit
		while (!controlManager.isKeyPressed(GLFW_KEY_ESCAPE) && !windowManager.isWindowCloseRequested());

		// De-initialize the camera.
		perspectiveCamera->deinit();

		// Delete the vertex buffer we created since it's no longer required
		objectManager.destroyObject(cubeObjectDetails);

		// Delete the vertex array object.
		glDeleteVertexArrays(1, &vertexArrayID);

		// Bye :)
		return 0;
}
