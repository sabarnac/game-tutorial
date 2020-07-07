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
	auto &windowManager = WindowManager::getInstance();
	auto &controlManager = ControlManager::getInstance();
	auto &modelManager = ModelManager::getInstance();
	auto &lightManager = LightManager::getInstance();
	auto &cameraManager = CameraManager::getInstance();
	auto &renderManager = RenderManager::getInstance();
	auto &debugRenderManager = DebugRenderManager::getInstance();
	auto &shaderManager = ShaderManager::getInstance();

	// Set the mouse at the center of the screen
	windowManager.pollEvents();
	controlManager.setCursorPosition(CursorPosition(0.5, 0.5));

	GLuint vertexArrayID;
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	// Create and compile our GLSL program from the shaders
	auto perspectiveCamera = PerspectiveCamera::create("MainCamera");
	cameraManager.registerCamera(perspectiveCamera);
	perspectiveCamera->setCameraPosition(glm::vec3(0.0, 20.0, 40.0));
	perspectiveCamera->setCameraAngles(glm::pi<double>(), -(glm::pi<double>() / 4.3));

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

	auto playerModel = PlayerModel::create("MainPlayer");
	modelManager.registerModel(playerModel);

	auto deadConeLight = ConeLight::create("DeadConeLight");
	lightManager.registerDeadSimpleLight(deadConeLight);
	auto deadPointLight = PointLight::create("DeadPointLight");
	lightManager.registerDeadCubeLight(deadPointLight);

	auto debugEnabled = false;
	auto lastDebugEnabledChange = glfwGetTime() - 10;
	do
	{
		auto currentTime = glfwGetTime();
		if (controlManager.isKeyPressed(GLFW_KEY_B) && (currentTime - lastDebugEnabledChange) > 0.5)
		{
			debugEnabled = !debugEnabled;
			lastDebugEnabledChange = currentTime;
		}

		modelManager.updateAllModels();
		cameraManager.updateAllCameras();

		renderManager.render();
		if (debugEnabled)
		{
			debugRenderManager.render();
		}

		windowManager.swapBuffers();
		windowManager.pollEvents();
	} while (!controlManager.isKeyPressed(GLFW_KEY_ESCAPE) &&
					 !windowManager.isWindowCloseRequested());

	glDeleteVertexArrays(1, &vertexArrayID);

	return 0;
}
