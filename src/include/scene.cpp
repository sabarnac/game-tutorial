#ifndef INCLUDE_SCENE_CPP
#define INCLUDE_SCENE_CPP

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <algorithm>
#include <iterator>
#include <any>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "object.cpp"
#include "texture.cpp"
#include "shader.cpp"
#include "collider.cpp"
#include "text.cpp"
#include "../scenes/scene_base.cpp"

/**
 * A manager class for managing scenes in a scene.
 */
class SceneManager
{
private:
  // Singleton instance of the scene manager.
  static SceneManager instance;

  std::string activeSceneId;
  // The map of registered scenes.
  std::map<const std::string, std::shared_ptr<SceneBase>> registeredScenes;

  SceneManager() : registeredScenes({}) {}

public:
  // Preventing copying the scene manager, making sure only one instance can exist.
  SceneManager(const SceneManager &) = delete;

  /**
   * Register a new scene into the scene manager.
   * 
   * @param scene  The scene to register.
   */
  void registerScene(const std::shared_ptr<SceneBase> &&scene)
  {
    // Insert the scene to the map of registered scenes.
    registeredScenes.emplace(scene->getSceneId(), std::move(scene));
  }

  /**
   * Register a given scene ID as the active scene.
   * 
   * @param scene  The ID of the scene to mark as active.
   */
  void registerActiveScene(const std::string &sceneId)
  {
    activeSceneId = sceneId;
  }

  /**
   * De-register an existing scene from the scene manager.
   * 
   * @param scene  The ID of the scene to de-register.
   */
  void deregisterScene(const std::string &sceneId)
  {
    // Check if scene actually exists. If not, just return since it's not registered.
    if (registeredScenes.find(sceneId) == registeredScenes.end())
    {
      return;
    }

    // Get the scene that is registered with the given scene ID.
    auto scene = registeredScenes[sceneId];
    // Remove the scene from the map of registered scenes.
    deregisterScene(scene);
  }

  /**
   * De-register an existing scene from the scene manager.
   * 
   * @param scene  The scene to de-register.
   */
  void deregisterScene(const std::shared_ptr<SceneBase> &scene)
  {
    // Remove the scene from the map of registered scenes.
    registeredScenes.erase(scene->getSceneId());
  }

  /**
   * Return the scene registered with the given scene ID.
   * 
   * @param scene  The ID of the scene to return.
   * 
   * @return The scene registered with the given scene ID.
   */
  const std::shared_ptr<SceneBase> &getScene(const std::string &sceneId) const
  {
    return registeredScenes.at(sceneId);
  }

  /**
   * Return the list of all scenes registered with the scene manager.
   * 
   * @return The list of all registered scenes.
   */
  const std::vector<std::shared_ptr<SceneBase>> getAllScenes() const
  {
    // Define a vector to store the list of registered scenes.
    std::vector<std::shared_ptr<SceneBase>> scenes({});
    // Iterate through the map of registered scenes.
    for (const auto &scene : registeredScenes)
    {
      // Push each registered scene into the scenes list.
      scenes.push_back(scene.second);
    }
    // Return the list of registered scenes.
    return scenes;
  }

  /**
   * Run the execute operation on the active scene.
   * 
   * @return Whether an active scene is still present.
   */
  bool executeActiveScene()
  {
    const auto activeScene = registeredScenes.find(activeSceneId);
    if (activeScene == registeredScenes.end())
    {
      return false;
    }

    activeScene->second->init();
    const auto nextSceneId = activeScene->second->execute();
    activeScene->second->deinit();
    if (!nextSceneId.has_value())
    {
      return false;
    }

    activeSceneId = nextSceneId.value();

    return true;
  }

  /**
   * Returns the singleton instance of the scene manager.
   * 
   * @return The scene manager singleton instance.
   */
  static SceneManager &getInstance()
  {
    return instance;
  }
};

// Initialize the scene manager singleton instance static variable.
SceneManager SceneManager::instance;

#endif