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

/**
 * Class that represents a shot/bullet model.
 */
class ShotModel : public ModelBase<ShotModel>
{
private:
  // The speed of the shot.
  static double shotSpeed;
  // Whether to show the light or not.
  static bool isShotLightPresent;
  // The timestamp for the last time the shot light was toggled.
  static float_t lastShotLightChange;

  // The model manager responsible for managing the models in the scene.
  ModelManager &modelManager;
  // The light manager responsible for managing the lights in the scene.
  LightManager &lightManager;
  // The control manager responsible for managing controls and inputs of the window.
  const ControlManager &controlManager;

  float_t rotationSpeedZ;

  // The timestamp of the last time the update for the camera was started.
  float_t lastTime;

  // The instance of the point light for the shot.
  std::shared_ptr<PointLight> shotLight;

  /**
   * Create a new shot light.
   */
  void createShotLight()
  {
    // Destroy any existing shot light.
    destroyShotLight();

    // Create shot light and set its properties.
    shotLight = PointLight::create(getModelId() + "::ShotLight");
    shotLight->setLightPosition(getModelPosition() + glm::vec3(0.0f, 0.0f, 0.75f));

    // Register the shot light.
    shotLight->init();
    lightManager.registerLight(shotLight);

    // Update the shot light toggle.
    isShotLightPresent = true;
  }

  /**
   * Destroy the existing shot light.
   */
  void destroyShotLight()
  {
    // Check if shot light exists
    if (shotLight != nullptr)
    {
      // Destroy the shot light.
      shotLight->deinit();
      lightManager.deregisterLight(shotLight);
      shotLight = nullptr;
    }
    // Update the shot light toggle.
    isShotLightPresent = false;
  }

  /**
   * Update the existing shot light.
   */
  void updateShotLight()
  {
    // Check if shot light toggle is enabled.
    if (isShotLightPresent)
    {
      // check if shot light doesn't exist.
      if (shotLight == nullptr)
      {
        // Create shot light.
        createShotLight();
      }
      // Update the shot light.
      shotLight->setLightPosition(getModelPosition() + glm::vec3(0.0f, 0.0f, 0.75f));
    }
    // Check if shot light exists.
    else if (shotLight != nullptr)
    {
      // Destroy any existing shot light.
      destroyShotLight();
    }
  }

public:
  ShotModel(const std::string &modelId)
      : ModelBase(
            modelId,
            glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.075f),
            ColliderShapeType::BOX),
        modelManager(ModelManager::getInstance()),
        lightManager(LightManager::getInstance()),
        controlManager(ControlManager::getInstance()),
        rotationSpeedZ(glm::radians(5.0f)),
        lastTime(glfwGetTime()),
        shotLight(nullptr) {}

  static void initModel()
  {
    ModelBase::initModelDeps(
        "Shot",
        "assets/objects/shot.obj",
        "assets/textures/shot.bmp",
        "assets/shaders/vertex/shot.glsl", "assets/shaders/fragment/shot.glsl");
  }

  static void deinitModel()
  {
    ModelBase::deinitModelDeps();
  }

  /**
   * Creates a new instance of the shot model.
   */
  const static std::shared_ptr<ShotModel> create(const std::string &modelId)
  {
    return std::make_shared<ShotModel>(modelId);
  }

  void init() override
  {
    // Set the rotation of the model.
    setModelRotation(glm::vec3(0.0f, glm::radians(180.0f), 0.0f));

    // Check if shot light toggle is enabled.
    if (isShotLightPresent)
    {
      // Create a shot light.
      createShotLight();
    }
  }

  void deinit() override
  {
    // Check if shot light toggle is enabled.
    if (isShotLightPresent)
    {
      // Destroy the shot light and update the toggle.
      destroyShotLight();
      isShotLightPresent = true;
    }
  }

  void update() override
  {
    // Get the timestamp for the start of the update.
    const auto currentTime = glfwGetTime();
    // Get the time difference since the start of the last update.
    const auto deltaTime = currentTime - lastTime;

    // Get the position of the shot.
    const auto currentPosition = getModelPosition();
    // Check if the shot has already gone beyond a threshold.
    if (currentPosition.z < -50.0f)
    {
      // If it has, destroy it.
      this->deinit();
      modelManager.deregisterModel(this->getModelId());
      return;
    }

    // Check if "H" key was pressed after 500ms since the last shot light toggle.
    if (controlManager.isKeyPressed(GLFW_KEY_H) && (currentTime - lastShotLightChange) > 0.5f)
    {
      // "H" key was pressed. Toggle the shot light.
      isShotLightPresent = !isShotLightPresent;
      // Update the timestamp for when the shot light was last toggled.
      lastShotLightChange = currentTime;
    }

    const auto timeSlices = 12;
    for (auto i = 0; i < timeSlices; i++)
    {
      // Update the shot position.
      setModelPosition(getModelPosition() - glm::vec3(0.0f, 0.0f, (shotSpeed * deltaTime) / timeSlices));

      if (currentPosition.z > 1.5f)
      {
        continue;
      }

      // Get the list of registered models.
      const auto models = modelManager.getAllModels();
      // Iterate over the list of registered models.
      for (const auto &model : models)
      {
        // Check if the current model is an enemy model.
        if (model->getModelName() != "Enemy")
        {
          continue;
        }

        if (glm::distance(model->getModelPosition(), currentPosition) > 3.0f)
        {
          continue;
        }

        // Check if collided with the enemy model.
        if (DeepCollisionValidator::haveShapesCollided(getColliderDetails()->getColliderShape(), model->getColliderDetails()->getColliderShape(), true))
        {
          // Shot has collided with an enemy. Destroy both.
          model->deinit();
          modelManager.deregisterModel(model);

          this->deinit();
          modelManager.deregisterModel(this->getModelId());
          return;
        }
      }
    }

    setModelRotation(getModelRotation() - glm::vec3(0.0f, 0.0f, rotationSpeedZ));

    // Update the shot light.
    updateShotLight();

    // Set the timestamp for the start of the last update to the starting timestamp of the current update.
    lastTime = currentTime;
  }
};

// Initialize the shot speed static variable.
double ShotModel::shotSpeed = 120.0f;
// Initialize the shot light toggle static variable.
bool ShotModel::isShotLightPresent = true;
// Initialize the last time the shot light toggle was changed static variable.
float_t ShotModel::lastShotLightChange = -1;

#endif