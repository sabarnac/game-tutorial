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

class ModelManager
{
private:
  static ModelManager instance;

  RenderManager &renderManager;

  std::map<std::string, std::shared_ptr<ModelBase>> registeredModels;

  ModelManager()
      : renderManager(RenderManager::getInstance()),
        registeredModels({}) {}

public:
  ModelManager(ModelManager &) = delete;

  void registerModel(std::shared_ptr<ModelBase> model)
  {
    registeredModels.insert(std::pair<std::string, std::shared_ptr<ModelBase>>(model->getModelId(), model));
    renderManager.registerModel(model);
    model->init();
  }

  void deregisterModel(std::shared_ptr<ModelBase> model)
  {
    registeredModels.erase(model->getModelId());
    renderManager.deregisterModel(model);
    model->deinit();
  }

  void deregisterModel(std::string modelId)
  {
    auto model = registeredModels[modelId];
    registeredModels.erase(modelId);
    renderManager.deregisterModel(modelId);
    model->deinit();
  }

  std::shared_ptr<ModelBase> getModel(std::string modelId)
  {
    return registeredModels[modelId];
  }

  std::vector<std::shared_ptr<ModelBase>> getAllModels()
  {
    std::vector<std::shared_ptr<ModelBase>> models({});
    for (auto model = registeredModels.begin(); model != registeredModels.end(); model++)
    {
      models.push_back(model->second);
    }
    return models;
  }

  void updateAllModels()
  {
    std::vector<std::string> registeredModelIds({});
    for (auto model = registeredModels.begin(); model != registeredModels.end(); model++)
    {
      registeredModelIds.push_back(model->first);
    }

    for (auto modelId = registeredModelIds.begin(); modelId != registeredModelIds.end(); modelId++)
    {
      auto result = registeredModels.find(*(modelId));
      if (result != registeredModels.end())
      {
        result->second->update();
      }
    }
  }

  static ModelManager &getInstance()
  {
    return instance;
  }
};

ModelManager ModelManager::instance;

#endif