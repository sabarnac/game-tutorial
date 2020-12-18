#ifndef SCENES_SCENE_BASE_CPP
#define SCENES_SCENE_BASE_CPP

#include <string>
#include <map>
#include <vector>
#include <optional>
#include <memory>
#include <algorithm>
#include <iterator>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

/**
 * Base class for creating scenes.
 */
class SceneBase
{

private:
  // The ID of the scene.
  const std::string sceneId;
  // The name of the scene.
  const std::string sceneName;

protected:
  WindowManager &windowManager;
  TextManager &textManager;

  SceneBase(const std::string &sceneId, const std::string &sceneName)
      : windowManager(WindowManager::getInstance()),
        textManager(TextManager::getInstance()),
        sceneId(sceneId), sceneName(sceneName)
  {
  }

  virtual ~SceneBase()
  {
  }

  void renderLoadingText(const std::string &content, const glm::vec2 &position, const float_t &scale)
  {
    textManager.addText(content, position, scale);

    windowManager.clearScreen(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    textManager.render();

    windowManager.swapBuffers();
  }

public:
  /**
   * Get the ID of the scene.
   * 
   * @return The scene ID.
   */
  const std::string &getSceneId() const
  {
    return sceneId;
  }

  /**
   * Get the name of the scene.
   * 
   * @return The scene name.
   */
  const std::string &getSceneName() const
  {
    return sceneName;
  }

  /**
   * Initialize the scene once registered.
   */
  virtual const void init() {}

  /**
   * De-initialize the scene once de-registered.
   */
  virtual const void deinit() {}

  /**
   * Execute the scene during the update step before starting rendering.
   */
  virtual const std::optional<std::string> execute() = 0;
};

#endif