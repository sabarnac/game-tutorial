#include <string>

#include <GL/glew.h>

#include "include/window.cpp"
#include "include/control.cpp"
#include "include/camera.cpp"
#include "include/light.cpp"
#include "include/render.cpp"
#include "include/debug_render.cpp"
#include "light/cone_light.cpp"
#include "light/point_light.cpp"
#include "camera/perspective_camera.cpp"
#include "models/enemy_model.cpp"
#include "models/player_model.cpp"

using namespace glm;

int main(void)
{
	// Get instances of the required managers
	auto &windowManager = WindowManager::getInstance();
	auto &controlManager = ControlManager::getInstance();
	auto &modelManager = ModelManager::getInstance();
	auto &lightManager = LightManager::getInstance();
	auto &cameraManager = CameraManager::getInstance();
	auto &renderManager = RenderManager::getInstance();
	auto &debugRenderManager = DebugRenderManager::getInstance();

	// Poll for events and set the mouse to the center of the screen
	controlManager.pollEvents();
	controlManager.setCursorPosition(CursorPosition(0.5, 0.5));

	// Create and bind a vertex array object so that vertex attribute operations can be stored and managed in them.
	GLuint vertexArrayID;
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	// Create a perspective camera, and set its properties.
	auto perspectiveCamera = PerspectiveCamera::create("MainCamera");
	cameraManager.registerCamera(perspectiveCamera);
	perspectiveCamera->setCameraPosition(glm::vec3(0.0, 20.0, 40.0));
	perspectiveCamera->setCameraAngles(glm::pi<double>(), -(glm::pi<double>() / 4.3));

	// Create 45 enemy models stacked in a grid format (5 x 3 x 3), and set their properties.
	for (auto i = -2; i <= 2; i++)
	{
		for (auto j = -1; j <= 1; j++)
		{
			for (auto k = -2; k <= 0; k++)
			{
				auto enemyModel = EnemyModel::create("Enemy" + std::to_string((9 * (i + 2)) + (3 * (j + 1)) + (k + 2)));
				modelManager.registerModel(enemyModel);
				enemyModel->setModelPosition(glm::vec3(i * 5, j * 5, k * 5));
			}
		}
	}

	// Create a player model.
	auto playerModel = PlayerModel::create("MainPlayer");
	modelManager.registerModel(playerModel);

	// Create dead lights for the render manager.
	auto deadConeLight = ConeLight::create("DeadConeLight");
	lightManager.registerDeadSimpleLight(deadConeLight);
	auto deadPointLight = PointLight::create("DeadPointLight");
	lightManager.registerDeadCubeLight(deadPointLight);

	// Set debug mode to initially false.
	auto debugEnabled = false;
	// Set the timestamp for when debug mode toggle was changed to 10 seconds in the past.
	auto lastDebugEnabledChange = glfwGetTime() - 10;

	// Start the game loop.
	do
	{
		// Get the time at the start of the loop.
		auto currentTime = glfwGetTime();

		// Check if "B" key was pressed beyond 500ms since the last debug mode toggle.
		if (controlManager.isKeyPressed(GLFW_KEY_B) && (currentTime - lastDebugEnabledChange) > 0.5)
		{
			// "B" key was pressed. Toggle debug mode and update the last change timestamp.
			debugEnabled = !debugEnabled;
			lastDebugEnabledChange = currentTime;
		}

		// Update the lights, cameras, models.
		lightManager.updateAllLights();
		modelManager.updateAllModels();
		cameraManager.updateAllCameras();

		// Render the scene.
		renderManager.render();
		// Check if debug mode is enabled.
		if (debugEnabled)
		{
			// Render the debug models fo the main models and lights.
			debugRenderManager.render();
		}

		// Swap the window framebuffers.
		windowManager.swapBuffers();
		// Poll for window events.
		controlManager.pollEvents();

		// Continue loop as long as escape key isn't pressed or the window close is not requested.
	} while (!controlManager.isKeyPressed(GLFW_KEY_ESCAPE) &&
					 !windowManager.isWindowCloseRequested());

	// Delete the vertex array object.
	glDeleteVertexArrays(1, &vertexArrayID);

	// Bye :)
	return 0;
}
