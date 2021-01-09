#include <string>
#include <future>

#include <GL/glew.h>

#include "include/scene.cpp"

#include "scenes/main_menu_scene.cpp"
#include "scenes/game_scene.cpp"
#include "scenes/end_scene.cpp"

using namespace glm;

int main(void)
{
	SceneManager &sceneManager = SceneManager::getInstance();

	auto mainMenuScene = MainMenuScene::create("MainMenuScene");
	auto gameScene = GameScene::create("GameScene");
	auto endScene = EndScene::create("EndScene");

	sceneManager.registerScene(mainMenuScene);
	sceneManager.registerScene(gameScene);
	sceneManager.registerScene(endScene);

	sceneManager.registerActiveScene(mainMenuScene->getSceneId());

	GLuint vertexArrayID;
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	while (sceneManager.executeActiveScene())
		;

	sceneManager.deregisterScene("EndScene");
	sceneManager.deregisterScene("GameScene");
	sceneManager.deregisterScene("MainMenuScene");

	glDeleteVertexArrays(1, &vertexArrayID);

	return 0;
}
