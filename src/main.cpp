#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "include/shader.cpp"

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

	// Define what the vertices of our triangle look like.
	const GLfloat triangleVertices[] = {
			-1, -1, 0,
			1, -1, 0,
			0, 1, 0};

	// Get an instance of the shader manager and generate the shader program for the triangle.
	auto &shaderManager = ShaderManager::getInstance();
	auto shaderDetails = shaderManager.createShaderProgram("TriangleShader", "assets/vertex.glsl", "assets/fragment.glsl");

	// Generate a buffer object for storing our triangle vertices data into.
	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer);
	// Bind the generated vertex buffer as an array buffer, telling the GPU it will store data as an array of elements.
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	// Now that we're working under the context of the currently bound vertex buffer, copy the data of the triangle vertices,
	//   providing the size and reference to the data, and how the data is being used.
	// STATIC -> The data won't be changed.
	// DRAW -> The data will be used for drawing/rendering.
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	do
	{
		// Clear the screen.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our created triangle shader program.
		glUseProgram(shaderDetails->getShaderId());

		// Enable the first vertex attribute array, which will carry our triangle vertices information and loop through them.
		glEnableVertexAttribArray(0);
		// Bind the vertex buffer as an array buffer to the 0th vertex attribute array.
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		// Define the attributes of the data being sent through the vertex attribute array buffer.
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

		// Draw the vertices such that every 3 consecutive vertices draw a triangle.
		glDrawArrays(GL_TRIANGLES, 0, 3);

		// Disable the vertex attribute array since it's done being used.
		glDisableVertexAttribArray(0);

		// Swap the window buffers.
		glfwSwapBuffers(window);
		// Poll for new control events.
		glfwPollEvents();
	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

	// Delete the vertex buffer we created since it's no longer required.
	glDeleteBuffers(1, &vertexBuffer);

	// Delete the vertex array object.
	glDeleteVertexArrays(1, &vertexArrayID);

	// Destroy the GLFW window on application termination.
	glfwDestroyWindow(window);

	// Bye :)
	return 0;
}
