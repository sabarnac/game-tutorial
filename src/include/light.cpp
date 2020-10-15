#ifndef INCLUDE_LIGHT_CPP
#define INCLUDE_LIGHT_CPP

#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "render.cpp"
#include "text.cpp"
#include "../light/light_base.cpp"

/**
 * A manager class for managing lights in a scene.
 */
class LightManager
{
private:
  // Singleton instance of the light manager.
  static LightManager instance;

  // The render manager responsible for rendering objects.
  RenderManager &renderManager;
  TextManager &textManager;

  // The map of registered lights.
  std::map<const std::string, std::shared_ptr<LightBase>> registeredLights;

  LightManager()
      : renderManager(RenderManager::getInstance()),
        textManager(TextManager::getInstance()),
        registeredLights({}) {}

public:
  // Preventing copying the light manager, making sure only one instance can exist.
  LightManager(const LightManager &) = delete;

  /**
   * Register a new light into the light manager.
   * 
   * @param light  The light to register.
   */
  void registerLight(const std::shared_ptr<LightBase> &light)
  {
    // Let the light initialize itself.
    light->init();
    // Insert the light to the map of registered lights.
    registeredLights.insert(std::pair<const std::string, std::shared_ptr<LightBase>>(light->getLightId(), light));
    // Register the light with the render manager as well so that it can be used for rendering.
    renderManager.registerLight(light);
  }

  /**
   * De-register an existing light from the light manager.
   * 
   * @param light  The ID of the light to de-register.
   */
  void deregisterLight(const std::string &lightId)
  {
    // Get the light that is registered with the given light ID.
    auto light = registeredLights[lightId];
    // Remove the light from the map of registered lights.
    deregisterLight(light);
  }

  /**
   * De-register an existing light from the light manager.
   * 
   * @param light  The light to de-register.
   */
  void deregisterLight(const std::shared_ptr<LightBase> &light)
  {
    // Remove the light from the map of registered lights.
    registeredLights.erase(light->getLightId());
    // De-register the light from the render manager as well.
    renderManager.deregisterLight(light);
    // Let the light de-initialize itself.
    light->deinit();
  }

  /**
   * Return the light registered with the given light ID.
   * 
   * @param light  The ID of the light to return.
   * 
   * @return The light registered with the given light ID.
   */
  const std::shared_ptr<LightBase> &getLight(const std::string &lightId) const
  {
    return registeredLights.at(lightId);
  }

  /**
   * Return the list of all lights registered with the light manager.
   * 
   * @return The list of all registered lights.
   */
  const std::vector<std::shared_ptr<LightBase>> getAllLights() const
  {
    // Define a vector to store the list of registered lights.
    std::vector<std::shared_ptr<LightBase>> lights({});
    // Iterate through the map of registered lights.
    for (const auto &light : registeredLights)
    {
      // Push each registered light into the lights list.
      lights.push_back(light.second);
    }
    // Return the list of registered lights.
    return lights;
  }

  /**
   * Run the update operation on all the registered lights.
   */
  void updateAllLights()
  {
    // Define a vector to store the IDs of the registered lights.
    std::vector<std::string> registeredLightIds({});
    // iterate through the map of registered lights.
    for (const auto &light : registeredLights)
    {
      // Push the ID of each registered light into the lights ID list.
      registeredLightIds.push_back(light.first);
    }

    auto lightNamesCount = std::map<const std::string, int>({});
    auto lightNamesProcessTime = std::map<const std::string, double>({});

    // Iterate through the list of light IDs.
    for (const auto &lightId : registeredLightIds)
    {
      // Find the light registered with the given light ID.
      const auto result = registeredLights.find(lightId);
      // Check if the light still exists in the registration map.
      if (result != registeredLights.end())
      {
        if (lightNamesCount.find(result->second->getLightName()) != lightNamesCount.end())
        {
          lightNamesCount[result->second->getLightName()]++;
        }
        else
        {
          lightNamesCount[result->second->getLightName()] = 1;
          lightNamesProcessTime[result->second->getLightName()] = 0.0;
        }

        // If it does, tell the light to perform an update on itself.
        const auto startTime = glfwGetTime();
        result->second->update();
        const auto endTime = glfwGetTime();
        lightNamesProcessTime[result->second->getLightName()] += (endTime - startTime) * 1000;
      }
    }

    auto height = 15.0;
    for (const auto &lightCounts : lightNamesCount)
    {
      const auto avgRenderTime = lightNamesProcessTime[lightCounts.first] / lightCounts.second;
      textManager.addText(lightCounts.first + " Light Object Instances: " + std::to_string(lightCounts.second) + " | Update (avg): " + std::to_string(avgRenderTime) + "ms", glm::vec2(1, height), 0.5);
      height -= 0.5;
    }
  }

  /**
   * Returns the singleton instance of the light manager.
   * 
   * @return The light manager singleton instance.
   */
  static LightManager &getInstance()
  {
    return instance;
  }
};

// Initialize the light manager singleton instance static variable.
LightManager LightManager::instance;

#endif