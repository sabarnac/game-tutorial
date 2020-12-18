#ifndef INCLUDE_MODELS_CPP
#define INCLUDE_MODELS_CPP

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
#include "../models/model_base_intf.cpp"

/**
 * A manager class for managing models in a scene.
 */
class ModelManager
{
private:
  // Singleton instance of the model manager.
  static ModelManager instance;

  // The text manager responsible for rendering text.
  TextManager &textManager;

  // The map of registered models.
  std::map<const std::string, std::shared_ptr<ModelBaseIntf>> registeredModels;
  std::vector<std::string> registeredModelsInsertionOrder;

  ModelManager()
      : textManager(TextManager::getInstance()),
        registeredModels({}),
        registeredModelsInsertionOrder({}) {}

public:
  // Preventing copying the model manager, making sure only one instance can exist.
  ModelManager(const ModelManager &) = delete;

  /**
   * Register a new model into the model manager.
   * 
   * @param model  The model to register.
   */
  void registerModel(const std::shared_ptr<ModelBaseIntf> &&model)
  {
    // Insert the model to the map of registered models.
    registeredModels.emplace(model->getModelId(), std::move(model));
    registeredModelsInsertionOrder.push_back(model->getModelId());
  }

  /**
   * De-register an existing model from the model manager.
   * 
   * @param model  The ID of the model to de-register.
   */
  void deregisterModel(const std::string &modelId)
  {
    // Check if model actually exists. If not, just return since it's not registered.
    if (std::find(registeredModelsInsertionOrder.begin(), registeredModelsInsertionOrder.end(), modelId) == registeredModelsInsertionOrder.end())
    {
      return;
    }

    // Get the model that is registered with the given model ID.
    auto model = registeredModels[modelId];
    // Remove the model from the map of registered models.
    deregisterModel(model);
  }

  /**
   * De-register an existing model from the model manager.
   * 
   * @param model  The model to de-register.
   */
  void deregisterModel(const std::shared_ptr<ModelBaseIntf> &model)
  {
    // Remove the model from the map of registered models.
    registeredModels.erase(model->getModelId());
    registeredModelsInsertionOrder.erase(std::remove(registeredModelsInsertionOrder.begin(), registeredModelsInsertionOrder.end(), model->getModelId()), registeredModelsInsertionOrder.end());
  }

  /**
   * Return the model registered with the given model ID.
   * 
   * @param model  The ID of the model to return.
   * 
   * @return The model registered with the given model ID.
   */
  const std::shared_ptr<ModelBaseIntf> &getModel(const std::string &modelId) const
  {
    return registeredModels.at(modelId);
  }

  /**
   * Return the list of all models registered with the model manager.
   * 
   * @return The list of all registered models.
   */
  const std::vector<std::shared_ptr<ModelBaseIntf>> getAllModels() const
  {
    // Define a vector to store the list of registered models.
    std::vector<std::shared_ptr<ModelBaseIntf>> models({});
    // Iterate through the map of registered models.
    for (const auto &modelId : registeredModelsInsertionOrder)
    {
      // Push each registered model into the models list.
      models.push_back(registeredModels.find(modelId)->second);
    }

    // Return the list of registered models.
    return models;
  }

  /**
   * Run the initialize operation on all the registered models.
   */
  void initAllModels()
  {
    // Iterate through the list of model IDs.
    for (const auto &modelId : registeredModelsInsertionOrder)
    {
      // Find the model registered with the given model ID.
      const auto result = registeredModels.find(modelId);
      // Check if the model still exists in the registration map.
      if (result != registeredModels.end())
      {
        result->second->init();
      }
    }
  }

  /**
   * Run the de-initialize operation on all the registered models.
   */
  void deinitAllModels()
  {
    // Iterate through the list of model IDs.
    for (const auto &modelId : registeredModelsInsertionOrder)
    {
      // Find the model registered with the given model ID.
      const auto result = registeredModels.find(modelId);
      // Check if the model still exists in the registration map.
      if (result != registeredModels.end())
      {
        result->second->deinit();
      }
    }
  }

  /**
   * Run the update operation on all the registered models.
   */
  void updateAllModels()
  {
    auto modelNamesCount = std::map<const std::string, int>({});
    auto modelNamesProcessTime = std::map<const std::string, double>({});

    // Iterate through the list of model IDs.
    for (const auto &modelId : registeredModelsInsertionOrder)
    {
      // Find the model registered with the given model ID.
      const auto result = registeredModels.find(modelId);
      // Check if the model still exists in the registration map.
      if (result != registeredModels.end())
      {
        const auto model = result->second;
        if (modelNamesCount.find(model->getModelName()) != modelNamesCount.end())
        {
          modelNamesCount[model->getModelName()]++;
        }
        else
        {
          modelNamesCount[model->getModelName()] = 1;
          modelNamesProcessTime[model->getModelName()] = 0.0f;
        }

        // If it does, tell the model to perform an update on itself.
        const auto startTime = glfwGetTime();
        model->update();
        const auto endTime = glfwGetTime();
        modelNamesProcessTime[model->getModelName()] += (endTime - startTime) * 1000;
      }
    }

    auto height = 17.0f;
    for (const auto &modelCounts : modelNamesCount)
    {
      const auto avgRenderTime = modelNamesProcessTime[modelCounts.first] / modelCounts.second;
      textManager.addText(modelCounts.first + " Model Object Instances: " + std::to_string(modelCounts.second) + " | Update (avg): " + std::to_string(avgRenderTime) + "ms", glm::vec2(1, height), 0.5f);
      height -= 0.5f;
    }
  }

  /**
   * Returns the singleton instance of the model manager.
   * 
   * @return The model manager singleton instance.
   */
  static ModelManager &getInstance()
  {
    return instance;
  }
};

// Initialize the model manager singleton instance static variable.
ModelManager ModelManager::instance;

#endif