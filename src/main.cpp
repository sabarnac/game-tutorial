#include <string>

#include <GL/glew.h>

#include "include/window.cpp"
#include "include/control.cpp"
#include "include/camera.cpp"
#include "include/light.cpp"
#include "include/render.cpp"
#include "include/debug_render.cpp"
#include "include/text.cpp"
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
	const auto &debugRenderManager = DebugRenderManager::getInstance();
	auto &textManager = TextManager::getInstance();

	// Poll for events and set the mouse to the center of the screen
	controlManager.pollEvents();
	controlManager.setCursorPosition(CursorPosition(0.5, 0.5));

	// Create and bind a vertex array object so that vertex attribute operations can be stored and managed in them.
	GLuint vertexArrayID;
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	// Create a perspective camera, and set its properties.
	const auto perspectiveCamera = PerspectiveCamera::create("MainCamera");
	cameraManager.registerCamera(perspectiveCamera);
	perspectiveCamera->setCameraPosition(glm::vec3(0.0, 20.0, 40.0));
	perspectiveCamera->setCameraAngles(glm::pi<double_t>(), -(glm::pi<double_t>() / 4.3));

	// Create 45 enemy models stacked in a grid format (5 x 3 x 3), and set their properties.
	for (auto i = -2; i <= 2; i++)
	{
		for (auto j = -1; j <= 1; j++)
		{
			for (auto k = -2; k <= 0; k++)
			{
				const auto enemyModel = EnemyModel::create("Enemy" + std::to_string((9 * (i + 2)) + (3 * (j + 1)) + (k + 2)));
				modelManager.registerModel(enemyModel);
				enemyModel->setModelPosition(glm::vec3(i * 5, j * 5, k * 5));
			}
		}
	}

	// Create a player model.
	const auto playerModel = PlayerModel::create("MainPlayer");
	modelManager.registerModel(playerModel);

	// Set debug mode to initially false.
	auto debugEnabled = false;
	// Set the timestamp for when debug mode toggle was changed to 10 seconds in the past.
	auto lastDebugEnabledChange = glfwGetTime() - 10;

	// Set debug text to initially false.
	auto textEnabled = false;
	// Set the timestamp for when debug text toggle was changed to 10 seconds in the past.
	auto lastTextEnabledChange = glfwGetTime() - 10;

	// Set the timestamp for when debug text toggle was changed to 10 seconds in the past.
	auto lastVsyncToggledChange = glfwGetTime() - 10;

	// Start the game loop.
	auto textRenderTimeLast = 0.0;
	auto frameTimeLast = 0.0;
	auto processTimeLast = 0.0;
	uint32_t textCharsRenderedLast = 0;
	do
	{
		textManager.addText("Window Dimensions: " + std::to_string(WINDOW_WIDTH) + "x" + std::to_string(WINDOW_HEIGHT) + "px", glm::vec2(1, 12), 0.5);
		textManager.addText("Viewport Dimensions: " + std::to_string(VIEWPORT_WIDTH) + "x" + std::to_string(VIEWPORT_HEIGHT) + "px", glm::vec2(1, 11.5), 0.5);
		textManager.addText("Framebuffer Dimensions: " + std::to_string(FRAMEBUFFER_WIDTH) + "x" + std::to_string(FRAMEBUFFER_HEIGHT) + "px", glm::vec2(1, 11), 0.5);
		textManager.addText("Text Dimensions: " + std::to_string(TEXT_WIDTH) + "x" + std::to_string(TEXT_HEIGHT) + "px", glm::vec2(1, 10.5), 0.5);
		textManager.addText("Max Lights:", glm::vec2(1, 10), 0.5);
		textManager.addText(std::to_string(MAX_CONE_LIGHTS) + " Cone Lights", glm::vec2(3, 9.5), 0.5);
		textManager.addText(std::to_string(MAX_POINT_LIGHTS) + " Point Lights", glm::vec2(3, 9), 0.5);
		textManager.addText("Max Text Characters: " + std::to_string(MAX_TEXT_CHARS) + " chars", glm::vec2(1, 8.5), 0.5);

		auto isSwapEnabledStr = SWAP_INTERVAL == 0 ? std::string("False") : SWAP_INTERVAL == 1 ? std::string("True (Single-Sync)") : std::string("True (Double-Sync)");
		textManager.addText("VSync Enabled: " + isSwapEnabledStr, glm::vec2(1, 8), 0.5);

		// Get the time at the start of the loop.
		const auto currentTime = glfwGetTime();
		auto updateStartTime = currentTime, updateEndTime = currentTime;

		// Check if "B" key was pressed beyond 500ms since the last debug mode toggle.
		if (controlManager.isKeyPressed(GLFW_KEY_B) && (currentTime - lastDebugEnabledChange) > 0.5)
		{
			// "B" key was pressed. Toggle debug mode and update the last change timestamp.
			debugEnabled = !debugEnabled;
			lastDebugEnabledChange = currentTime;
		}

		// Check if "T" key was pressed beyond 500ms since the last debug text toggle.
		if (controlManager.isKeyPressed(GLFW_KEY_T) && (currentTime - lastTextEnabledChange) > 0.5)
		{
			// "T" key was pressed. Toggle debug text and update the last change timestamp.
			textEnabled = !textEnabled;
			lastTextEnabledChange = currentTime;
		}

		// Check if "V" key was pressed beyond 500ms since the last vsync toggle.
		if (controlManager.isKeyPressed(GLFW_KEY_V) && (currentTime - lastVsyncToggledChange) > 0.5)
		{
			// "V" key was pressed. Toggle vsync and update the last change timestamp.
			windowManager.toggleVsync();
			lastVsyncToggledChange = currentTime;
		}

		// Update the lights.
		updateStartTime = glfwGetTime();
		lightManager.updateAllLights();
		updateEndTime = glfwGetTime();
		textManager.addText("Light Update: " + std::to_string((updateEndTime - updateStartTime) * 1000) + "ms", glm::vec2(1, 0.5), 0.5);

		// Update the cameras.
		updateStartTime = glfwGetTime();
		modelManager.updateAllModels();
		updateEndTime = glfwGetTime();
		textManager.addText("Model Update: " + std::to_string((updateEndTime - updateStartTime) * 1000) + "ms", glm::vec2(1, 1), 0.5);

		// Update the models.
		updateStartTime = glfwGetTime();
		cameraManager.updateAllCameras();
		updateEndTime = glfwGetTime();
		textManager.addText("Camera Update: " + std::to_string((updateEndTime - updateStartTime) * 1000) + "ms", glm::vec2(1, 1.5), 0.5);

		// Render the scene.
		updateStartTime = glfwGetTime();
		renderManager.render();
		updateEndTime = glfwGetTime();
		textManager.addText("Render: " + std::to_string((updateEndTime - updateStartTime) * 1000) + "ms", glm::vec2(1, 2), 0.5);

		// Check if debug mode is enabled.
		if (debugEnabled)
		{
			// Render the debug models fo the main models and lights.
			updateStartTime = glfwGetTime();
			debugRenderManager.render();
			updateEndTime = glfwGetTime();
			textManager.addText("Debug Render: " + std::to_string((updateEndTime - updateStartTime) * 1000) + "ms", glm::vec2(1, 2.5), 0.5);
		}

		// Render text
		textManager.addText("Text Render (Last Frame): " + std::to_string(textRenderTimeLast) + "ms", glm::vec2(1, 3), 0.5);
		textManager.addText("Text Characters Rendered (Last Frame): " + std::to_string(textCharsRenderedLast) + " chars", glm::vec2(1, 3.5), 0.5);

		textManager.addText("Process Time (Last Frame): " + std::to_string(processTimeLast) + "ms", glm::vec2(1, 4.5), 0.5);
		textManager.addText("Frame Time (Last Frame): " + std::to_string(frameTimeLast) + "ms", glm::vec2(1, 5), 0.5);
		textManager.addText("Frame Rate (Last Frame): " + std::to_string(1000 / frameTimeLast) + "fps", glm::vec2(1, 5.5), 0.5);

		// Check if debug text is enabled.
		updateStartTime = glfwGetTime();
		if (textEnabled)
		{
			textCharsRenderedLast = textManager.render();
		}
		updateEndTime = glfwGetTime();
		textRenderTimeLast = (updateEndTime - updateStartTime) * 1000;
		processTimeLast = (updateEndTime - currentTime) * 1000;

		// Swap the window framebuffers.
		windowManager.swapBuffers();

		updateEndTime = glfwGetTime();
		frameTimeLast = (updateEndTime - currentTime) * 1000;

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
