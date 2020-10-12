#ifndef INCLUDE_MODELS_CPP
#define INCLUDE_MODELS_CPP

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <algorithm>
#include <iterator>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "object.cpp"
#include "texture.cpp"
#include "shader.cpp"
#include "collider.cpp"
#include "render.cpp"
#include "../models/model_base.cpp"

/**
 * A manager class for managing models in a scene.
 */
class ModelManager
{
private:
  // Singleton instance of the model manager.
  static ModelManager instance;

  // The render manager responsible for rendering objects.
  RenderManager &renderManager;

  // The map of registered models.
  std::map<const std::string, std::shared_ptr<ModelBase>> registeredModels;

  ModelManager()
      : renderManager(RenderManager::getInstance()),
        registeredModels({}) {}

public:
  // Preventing copying the model manager, making sure only one instance can exist.
  ModelManager(const ModelManager &) = delete;

  /**
   * Register a new model into the model manager.
   * 
   * @param model  The model to register.
   */
  void registerModel(const std::shared_ptr<ModelBase> &model)
  {
    // Let the model initialize itself.
    model->init();
    // Insert the model to the map of registered models.
    registeredModels.insert(std::pair<const std::string, std::shared_ptr<ModelBase>>(model->getModelId(), model));
    // Register the model with the render manager as well so that it can be used for rendering.
    renderManager.registerModel(model);
  }

  /**
   * De-register an existing model from the model manager.
   * 
   * @param model  The ID of the model to de-register.
   */
  void deregisterModel(const std::string &modelId)
  {
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
  void deregisterModel(const std::shared_ptr<ModelBase> &model)
  {
    // Remove the model from the map of registered models.
    registeredModels.erase(model->getModelId());
    // De-register the model from the render manager as well.
    renderManager.deregisterModel(model);
    // Let the model de-initialize itself.
    model->deinit();
  }

  /**
   * Return the model registered with the given model ID.
   * 
   * @param model  The ID of the model to return.
   * 
   * @return The model registered with the given model ID.
   */
  const std::shared_ptr<ModelBase> &getModel(const std::string &modelId) const
  {
    return registeredModels.at(modelId);
  }

  /**
   * Return the list of all models registered with the model manager.
   * 
   * @return The list of all registered models.
   */
  const std::vector<std::shared_ptr<ModelBase>> getAllModels() const
  {
    // Define a vector to store the list of registered models.
    std::vector<std::shared_ptr<ModelBase>> models({});
    // Iterate through the map of registered models.
    for (const auto &model : registeredModels)
    {
      // Push each registered model into the models list.
      models.push_back(model.second);
    }
    // Return the list of registered models.
    return models;
  }

  /**
   * Run the update operation on all the registered models.
   */
  void updateAllModels()
  {
    // Define a vector to store the IDs of the registered models.
    std::vector<std::string> registeredModelIds({});
    // iterate through the map of registered models.
    for (const auto &model : registeredModels)
    {
      // Push the ID of each registered model into the models ID list.
      registeredModelIds.push_back(model.first);
    }

    // Iterate through the list of model IDs.
    for (const auto &modelId : registeredModelIds)
    {
      // Find the model registered with the given model ID.
      const auto result = registeredModels.find(modelId);
      // Check if the model still exists in the registration map.
      if (result != registeredModels.end())
      {
        // If it does, tell the model to perform an update on itself.
        result->second->update();
      }
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