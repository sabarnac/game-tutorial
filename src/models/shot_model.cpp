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
#include "../include/control.cpp"

#include "model_base.cpp"
#include "../light/point_light.cpp"

class ShotModel : public ModelBase
{
private:
  double lastTime;

  ModelManager &modelManager;
  LightManager &lightManager;
  ControlManager &controlManager;

  static bool isShotLightPresent;
  static double lastShotLightChange;
  std::shared_ptr<PointLight> shotLight;

  void createShotLight()
  {
    destroyShotLight();
    shotLight = PointLight::create(getModelId() + "::ShotLight");
    shotLight->setLightPosition(getModelPosition());
    lightManager.registerLight(shotLight);
    isShotLightPresent = true;
  }

  void destroyShotLight()
  {
    if (shotLight != nullptr)
    {
      lightManager.deregisterLight(shotLight);
      shotLight = nullptr;
    }
    isShotLightPresent = false;
  }

  void updateShotLight()
  {
    if (isShotLightPresent)
    {
      if (shotLight == nullptr)
      {
        createShotLight();
      }
      shotLight->setLightPosition(getModelPosition());
    }
    else if (shotLight != nullptr)
    {
      destroyShotLight();
    }
  }

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
        controlManager(ControlManager::getInstance()),
        lastTime(glfwGetTime()),
        shotLight(nullptr) {}

  static std::shared_ptr<ShotModel> create(std::string modelId)
  {
    return std::make_shared<ShotModel>(modelId);
  };

  void init() override
  {
    setModelScale(glm::vec3(0.5));
    if (isShotLightPresent)
    {
      createShotLight();
    }
  }

  void deinit() override
  {
    if (isShotLightPresent)
    {
      destroyShotLight();
      isShotLightPresent = true;
    }
  }

  void update() override
  {
    auto currentTime = glfwGetTime();
    auto deltaTime = currentTime - lastTime;

    auto currentPosition = getModelPosition();
    if (currentPosition.z < -50.0)
    {
      modelManager.deregisterModel(this->getModelId());
      return;
    }

    if (controlManager.isKeyPressed(GLFW_KEY_H) && (currentTime - lastShotLightChange) > 0.5)
    {
      isShotLightPresent = !isShotLightPresent;
      if (isShotLightPresent)
      {
        createShotLight();
      }
      else
      {
        destroyShotLight();
      }
      lastShotLightChange = currentTime;
    }

    setModelPosition(getModelPosition() - glm::vec3(0.0, 0.0, 100.0 * deltaTime));
    updateShotLight();

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
bool ShotModel::isShotLightPresent = true;
double ShotModel::lastShotLightChange = -1;

#endif