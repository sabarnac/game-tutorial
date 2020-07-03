// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <GL/glew.h>

#include "include/window.cpp"
#include "include/control.cpp"
#include "models/cube_model.cpp"
#include "camera/perspective_camera.cpp"

using namespace glm;

int main(void)
{
	auto &windowManager = WindowManager::getInstance();
	auto &controlManager = ControlManager::getInstance();

	// Set the mouse at the center of the screen
	windowManager.pollEvents();
	controlManager.setCursorPosition(CursorPosition(0.5, 0.5));

	GLuint vertexArrayID;
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	// Create and compile our GLSL program from the shaders
	auto cameraModel = PerspectiveCamera::create(
			glm::vec3(0.0, 0.0, 5.0),
			glm::pi<double>(),
			0.0,
			90.0,
			4.0 / 3.0,
			0.1,
			100.0);
	auto cubeModel = CubeModel::create();

	// Get a handle for our "MVP" uniform
	GLuint matrixId = glGetUniformLocation(cubeModel->getShaderDetails()->getShaderId(), "MVP");
	GLuint textureId = glGetUniformLocation(cubeModel->getShaderDetails()->getShaderId(), "myTextureSampler");
	do
	{
		cameraModel->update();
		cubeModel->update();

		// Clear the screen
		windowManager.clearScreen(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(cubeModel->getShaderDetails()->getShaderId());

		// Compute the MVP matrix from keyboard and mouse input
		glm::mat4 MVP = cameraModel->getProjectionMatrix() * cameraModel->getViewMatrix() * cubeModel->getModelMatrix();

		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(matrixId, 1, GL_FALSE, &MVP[0][0]);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cubeModel->getTextureDetails()->getTextureId());
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(textureId, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, cubeModel->getObjectDetails()->getVertexBufferId());
		glVertexAttribPointer(
				0,				// attribute
				3,				// size
				GL_FLOAT, // type
				GL_FALSE, // normalized?
				0,				// stride
				(void *)0 // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, cubeModel->getObjectDetails()->getUvBufferId());
		glVertexAttribPointer(
				1,				// attribute
				2,				// size
				GL_FLOAT, // type
				GL_FALSE, // normalized?
				0,				// stride
				(void *)0 // array buffer offset
		);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, cubeModel->getObjectDetails()->getNormalBufferId());
		glVertexAttribPointer(
				2,				// attribute
				3,				// size
				GL_FLOAT, // type
				GL_FALSE, // normalized?
				0,				// stride
				(void *)0 // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, cubeModel->getObjectDetails()->getBufferSize());

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// Swap buffers
		windowManager.swapBuffers();
		windowManager.pollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (!controlManager.isKeyPressed(GLFW_KEY_ESCAPE) &&
				 !windowManager.isWindowCloseRequested());

	glDeleteVertexArrays(1, &vertexArrayID);

	return 0;
}
