#ifndef MODELS_SHOT_MODEL_CPP
#define MODELS_SHOT_MODEL_CPP

#include <string>
#include <memory>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "../include/models.cpp"
#include "../include/light.cpp"

#include "model_base.cpp"
#include "../light/point_light.cpp"

class ShotModel : public ModelBase
{
private:
  double lastTime;

  ModelManager &modelManager;
  LightManager &lightManager;

  std::shared_ptr<PointLight> shotLight;

public:
  ShotModel(std::string modelId)
      : ModelBase(
            modelId,
            "Shot",
            glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f),
            "assets/objects/sphere.obj",
            "assets/textures/shot.bmp", TextureType::BMP,
            "assets/shaders/vertex/shot.glsl", "assets/shaders/fragment/shot.glsl",
            ColliderShapeType::SPHERE),
        modelManager(ModelManager::getInstance()),
        lightManager(LightManager::getInstance()),
        lastTime(glfwGetTime()) {}

  static std::shared_ptr<ShotModel> create(std::string modelId)
  {
    return std::make_shared<ShotModel>(modelId);
  };

  void init() override
  {
    setModelScale(glm::vec3(0.5));

    shotLight = PointLight::create(getModelId() + "::ShotLight");
    shotLight->setLightPosition(getModelPosition());
    lightManager.registerLight(shotLight);
  }

  void deinit() override
  {
    lightManager.deregisterLight(shotLight);
  }

  void update() override
  {
    auto currentPosition = getModelPosition();
    if (currentPosition.z < -50.0)
    {
      modelManager.deregisterModel(this->getModelId());
      return;
    }

    auto currentTime = glfwGetTime();
    auto deltaTime = currentTime - lastTime;

    setModelPosition(getModelPosition() - glm::vec3(0.0, 0.0, 100.0 * deltaTime));
    shotLight->setLightPosition(getModelPosition());

    auto models = modelManager.getAllModels();
    for (auto model = models.begin(); model != models.end(); model++)
    {
      if ((*model)->getModelId().rfind("Enemy", 0) == 0)
      {
        if (DeepCollisionValidator::haveShapesCollided(getColliderDetails()->getColliderShape(), (*model)->getColliderDetails()->getColliderShape(), true))
        {
          modelManager.deregisterModel(*model);
          modelManager.deregisterModel(this->getModelId());
          break;
        }
      }
    }

    lastTime = currentTime;
  }
};

#endif