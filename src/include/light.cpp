#ifndef INCLUDE_LIGHT_CPP
#define INCLUDE_LIGHT_CPP

#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

  // The text manager responsible for rendering text.
  TextManager &textManager;

  // The map of registered lights.
  std::map<const std::string, std::shared_ptr<LightBase>> registeredLights;
  std::vector<std::string> registeredLightsInsertionOrder;

  LightManager()
      : textManager(TextManager::getInstance()),
        registeredLights({}),
        registeredLightsInsertionOrder({}) {}

public:
  // Preventing copying the light manager, making sure only one instance can exist.
  LightManager(const LightManager &) = delete;

  /**
   * Register a new light into the light manager.
   * 
   * @param light  The light to register.
   */
  void registerLight(const std::shared_ptr<LightBase> &&light)
  {
    // Insert the light to the map of registered lights.
    registeredLights.emplace(light->getLightId(), std::move(light));
    registeredLightsInsertionOrder.push_back(light->getLightId());
  }

  /**
   * De-register an existing light from the light manager.
   * 
   * @param light  The ID of the light to de-register.
   */
  void deregisterLight(const std::string &lightId)
  {
    // Check if light actually exists. If not, just return since it's not registered.
    if (std::find(registeredLightsInsertionOrder.begin(), registeredLightsInsertionOrder.end(), lightId) == registeredLightsInsertionOrder.end())
    {
      return;
    }

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
    registeredLightsInsertionOrder.erase(std::remove(registeredLightsInsertionOrder.begin(), registeredLightsInsertionOrder.end(), light->getLightId()), registeredLightsInsertionOrder.end());
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
    for (const auto &lightId : registeredLightsInsertionOrder)
    {
      // Push each registered model into the models list.
      lights.push_back(registeredLights.find(lightId)->second);
    }
    // Return the list of registered lights.
    return lights;
  }

  /**
   * Run the initialize operation on all the registered lights.
   */
  void initAllLights()
  {
    // Iterate through the list of light IDs.
    for (const auto &lightId : registeredLightsInsertionOrder)
    {
      // Find the light registered with the given light ID.
      const auto result = registeredLights.find(lightId);
      // Check if the light still exists in the registration map.
      if (result != registeredLights.end())
      {
        result->second->init();
      }
    }
  }

  /**
   * Run the de-initialize operation on all the registered lights.
   */
  void deinitAllLights()
  {
    // Iterate through the list of light IDs.
    for (const auto &lightId : registeredLightsInsertionOrder)
    {
      // Find the light registered with the given light ID.
      const auto result = registeredLights.find(lightId);
      // Check if the light still exists in the registration map.
      if (result != registeredLights.end())
      {
        result->second->deinit();
      }
    }
  }

  /**
   * Run the update operation on all the registered lights.
   */
  void updateAllLights()
  {
    auto lightNamesCount = std::map<const std::string, int>({});
    auto lightNamesProcessTime = std::map<const std::string, double>({});

    // Iterate through the list of light IDs.
    for (const auto &lightId : registeredLightsInsertionOrder)
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
          lightNamesProcessTime[result->second->getLightName()] = 0.0f;
        }

        // If it does, tell the light to perform an update on itself.
        const auto startTime = glfwGetTime();
        result->second->update();
        const auto endTime = glfwGetTime();
        lightNamesProcessTime[result->second->getLightName()] += (endTime - startTime) * 1000;
      }
    }

    auto height = 15.0f;
    for (const auto &lightCounts : lightNamesCount)
    {
      const auto avgRenderTime = lightNamesProcessTime[lightCounts.first] / lightCounts.second;
      textManager.addText(lightCounts.first + " Light Object Instances: " + std::to_string(lightCounts.second) + " | Update (avg): " + std::to_string(avgRenderTime) + "ms", glm::vec2(1, height), 0.5f);
      height -= 0.5f;
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