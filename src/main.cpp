#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "include/shader.cpp"
#include "include/object.cpp"
#include "include/texture.cpp"

using namespace glm;

int main(void)
{
	// Initialize GLFW library
	if (!glfwInit())
	{
		// Failed to initialize, time to crash.
		std::cout << "Failed at window 1" << std::endl;
		exit(1);
	}

	// Set up OpenGL window hints for creating an OpenGL context.
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Because MacOS.
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a window.
	auto window = glfwCreateWindow(800, 600, "Game Tutorial", nullptr, nullptr);
	// Check if the window creation was successful.
	if (window == NULL)
	{
		// Failed to create, time to crash.
		std::cout << "Failed at window 2" << std::endl;
		exit(1);
	}

	// Set the newly created window as the current context in GLFW.
	glfwMakeContextCurrent(window);

	// Set the option for sticky keys on the window to true. This means keys will remain in the pressed state until they're processed.
	// This allows us to catch key presses that may be missed if we didn't poll in time.
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Set the option to disable the cursor when the window is active, preventing the user from moving the cursor out of the window,
	//   and accidentally clicking something else.
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set what the interval is for swapping buffers. A value of zero means the swap should be immediate.
	// A value of 1 means that a single screen refresh should occur before swapping buffers.
	glfwSwapInterval(0);

	// Setup glew as experimental mode so that we can initialize the core OpenGL profile.
	glewExperimental = true; // Needed for core profile
	// Initialize GLEW.
	if (glewInit() != GLEW_OK)
	{
		// Failed to initialize, time to crash.
		std::cout << "Failed at window 3" << std::endl;
		exit(1);
	}

	// Set the viewport width to the values we got from GLFW.
	glViewport(0, 0, 800, 600);

	// Set the color to use when clearing the screen/framebuffer.
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Enable depth testing of the fragments, allowing the GPU to drop fragments from a single object and not process them if a condition is satisfied.
	glEnable(GL_DEPTH_TEST);
	// Use the less than function for depth testing fragments. This means any fragment of an object that is behind another fragment of the same object is dropped.
	glDepthFunc(GL_LESS);

	// Enable culling of faces/polygons. This means that any face/polygon that is behind another polygon within the same object is dropped.
	glEnable(GL_CULL_FACE);

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
	glm::mat4 scalingMatrix = glm::scale(glm::vec3(1.5, 1.5, 1.5));
	// Define a matrix that represents how much the object should be rotated by (the rotation matrix - rotation across x-axis, y-axis, z-axis).
	glm::mat4 rotationMatrix = glm::toMat4(glm::quat(glm::vec3(5, 10, 15)));
	// Define a matrix that represents how much the object should be translated/moved by (the translation matrix - translate across x-axis, y-axis, z-axis).
	glm::mat4 translationMatrix = glm::translate(glm::vec3(0.5, 1, 1.5));

	// Create the overall model transformation matrix (or just model matrix) by multiplying the individual sub-transformations in the order of:
	//   1. Scaling
	//   2. Rotation
	//   3. Translation
	// In code, this multiplication is done in the reverse order, since the matrix is multiplied against the vertex, which will be at the right side of the operation.
	// This would result in the right side of this order having precedence with being multiplied against the vertex to change its position.
	// This will transform the vertex from a position relative to the model, to its position in the world (called world-space).
	glm::mat4 modelMatrix = translationMatrix * rotationMatrix * scalingMatrix;

	// Define a view matrix that defines the location of the camera and the direction it is pointed towards. This will transform the model from being relative to the world
	//   to instead be relative to the camera (called camera-space or view-space).
	glm::mat4 viewMatrix = glm::lookAt(glm::vec3(3, 3, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	// Define a perspective matrix projection of the 3D world (relative to the camera) onto the viewport. We define the field-of-view of the viewport,
	//   the aspect ratio of the viewport, and the closest and farthest the screen can see till.
	// This will transform the model from being relative to the camera, to instead be relative to the screen in a 3D homogenous space (coordinate range of 0 - 1 across all axes).
	// Anything outside the homogenous space (not within the coordinate range of 0 - 1 at any axis) can be considered to be not visible on the screen.
	// This 3D homogenous space relative to the screen is called clip-space. It aids the GPU in determining what can be drawn on the screen and what can be discard, since
	//   anything outside the homogenous coordinate range will not be visible.
	// This space also provides the GPU information to perform a perspective division of the model to correctly size it based on perspective and distance from the screen.
	glm::mat4 projectionMatrix = glm::perspective(glm::radians(60.0), 4.0 / 3.0, 0.1, 100.0);

	// // Alternative: Define an orthogonal (flat) projection of the 2D world (relative to the camera) onto the viewport. We define the left, right, top,
	// //   bottom boundaries of the screen, and the closest and farthest the screen can see till.
	// An orthogonal projection assumes a flat projection, meaning that if two objects of the same size are at different distances, they will appear the same,
	//   size on screen, unlike perspective projection which would shrink the object that is further back.
	// So an orthogonal projection can capture a 2D world, whereas a perspective projection that can capture a 3D world.
	// glm::mat4 projectionMatrix = glm::ortho(-8.0, 8.0, -6.0, 6.0, 0.1, 100.0);

	// Calculate the final model-view-projection matrix product to be sent to the GPU to be applied to each vertex of the object.
	glm::mat4 mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;

	do
	{
		// Clear the screen.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our created triangle shader program.
		glUseProgram(shaderDetails->getShaderId());

		// Activate the 0th texture unit, which can then read data from the cube texture.
		glActiveTexture(GL_TEXTURE0);
		// Bind the cube texture as a 2D texture to the 0th texture unit (which is now the immediate context for the texture operation).
		glBindTexture(GL_TEXTURE_2D, cubeTextureDetails->getTextureId());
		// Get the location ID of the uniform which represents the texture sampler variable defined in the shader.
		auto cubeTextureUniformId = glGetUniformLocation(shaderDetails->getShaderId(), "cubeTexture");
		// Assign the texture sampler variable in the shader a value of 0, so that it will read texture data from the 0th texture unit.
		glUniform1i(cubeTextureUniformId, 0);

		// Get the location ID of the uniform which represents the model-view-projection matrix variable defined in the shader.
		auto mvpMatrixUniformId = glGetUniformLocation(shaderDetails->getShaderId(), "mvpMatrix");
		// Assign the model-view-projection matrix variable in the shader the model-view-projection matrix result that we calculated.
		// We flag that we are only passing 1 matrix to the GPU, and that it should not be transposed before sending.
		glUniformMatrix4fv(mvpMatrixUniformId, 1, GL_FALSE, &mvpMatrix[0][0]);

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

		// Draw the vertices of the object such that every 3 consecutive vertices draws a triangle.
		glDrawArrays(GL_TRIANGLES, 0, cubeObjectDetails->getBufferSize());

		// Disable the vertex attribute arrays since they're done being used.
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(0);

		// Swap the window buffers.
		glfwSwapBuffers(window);
		// Poll for new control events.
		glfwPollEvents();
	}
	// Check if we need to exit
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

	// Delete the vertex buffer we created since it's no longer required
	objectManager.destroyObject(cubeObjectDetails);

	// Delete the vertex array object.
	glDeleteVertexArrays(1, &vertexArrayID);

	// Destroy the GLFW window on application termination.
	glfwDestroyWindow(window);

	// Bye :)
	return 0;
}
