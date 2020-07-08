#ifndef MODELS_PLAYER_MODEL_CPP
#define MODELS_PLAYER_MODEL_CPP

#include <string>
#include <memory>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "../include/control.cpp"
#include "../include/light.cpp"
#include "../include/models.cpp"

#include "model_base.cpp"
#include "shot_model.cpp"
#include "../light/cone_light.cpp"

/**
 * Class that represents a player model.
 */
class PlayerModel : public ModelBase
{
private:
  // The speed of movement with key inputs.
  static float keyboardSpeed;

  // The model manager responsible for managing the models in the scene.
  ModelManager &modelManager;
  // The light manager responsible for managing the lights in the scene.
  LightManager &lightManager;
  // The control manager responsible for managing controls and inputs of the window.
  ControlManager &controlManager;

  // The timestamp of the last time the update for the camera was started.
  double lastTime;
  // The timestamp of the last time a shot was created.
  double lastShot;
  // The ID of the last created shot.
  unsigned int shotId;

  // Whether to show the eye light or not.
  static bool isEyeLightPresent;
  // The timestamp for the last time the eye light was toggled.
  static double lastEyeLightChange;
  // The instance of the first eye light for the player.
  std::shared_ptr<ConeLight> eyeLight1;
  // The instance of the second eye light for the player.
  std::shared_ptr<ConeLight> eyeLight2;

  /**
   * Create a new eye light.
   * 
   * @param position  The position of the model.
   */
  void createEyeLight(glm::vec3 position)
  {
    // Create first eye light and set its properties.
    eyeLight1 = ConeLight::create(getModelId() + "::EyeLight1");
    eyeLight1->setLightPosition(glm::vec3(position.x - 0.35, position.y + 0.25, position.z - 1.0));
    eyeLight1->setLightAngles(glm::pi<double>(), 0.0);
    eyeLight1->setLightIntensity(350.0);
    // Register the first eye light.
    lightManager.registerLight(eyeLight1);

    // Create first eye light and set its properties.
    eyeLight2 = ConeLight::create(getModelId() + "::EyeLight2");
    eyeLight2->setLightPosition(glm::vec3(position.x + 0.35, position.y + 0.25, position.z - 1.0));
    eyeLight2->setLightAngles(glm::pi<double>(), 0.0);
    eyeLight2->setLightIntensity(350.0);
    // Register the first eye light.
    lightManager.registerLight(eyeLight2);

    // Update the eye light toggle.
    isEyeLightPresent = true;
  }

  /**
   * Destroy the existing eye light.
   */
  void destroyEyeLight()
  {
    // Destroy the eye lights.
    lightManager.deregisterLight(eyeLight1);
    lightManager.deregisterLight(eyeLight2);
    // Update the eye light toggle.
    isEyeLightPresent = false;
  }

  /**
   * Update the existing eye light.
   */
  void updateEyeLight(glm::vec3 newPosition)
  {
    // Check if eye light toggle is enabled.
    if (isEyeLightPresent)
    {
      // Update the eye lights.
      eyeLight1->setLightPosition(glm::vec3(newPosition.x - 0.35, newPosition.y + 0.25, newPosition.z - 1.0));
      eyeLight2->setLightPosition(glm::vec3(newPosition.x + 0.35, newPosition.y + 0.25, newPosition.z - 1.0));
    }
  }

public:
  PlayerModel(std::string modelId)
      : ModelBase(
            modelId,
            "Player",
            glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f),
            "assets/objects/monkey.obj",
            "assets/textures/player.bmp",
            "assets/shaders/vertex/default.glsl", "assets/shaders/fragment/default.glsl",
            ColliderShapeType::SPHERE),
        modelManager(ModelManager::getInstance()),
        lightManager(LightManager::getInstance()),
        controlManager(ControlManager::getInstance()),
        lastTime(glfwGetTime()),
        lastShot(glfwGetTime() - 10.0),
        shotId(0) {}

  /**
   * Creates a new instance of the player model.
   */
  static std::shared_ptr<PlayerModel> create(std::string modelId)
  {
    return std::make_shared<PlayerModel>(modelId);
  };

  void init() override
  {
    // Set the rotation of the model.
    setModelPosition(glm::vec3(0.0, 0.0, 30.0));
    setModelRotation(glm::vec3(glm::pi<double>() / 2, glm::pi<double>(), 0));

    // Check if eye light toggle is enabled.
    if (isEyeLightPresent)
    {
      // Create the eye lights.
      createEyeLight(getModelPosition());
    }
  }

  void update() override
  {
    // Get the timestamp for the start of the update.
    auto currentTime = glfwGetTime();
    // Get the time difference since the start of the last update.
    auto deltaTime = currentTime - lastTime;

    // Check if "J" key was pressed after 500ms since the last eye light toggle.
    if (controlManager.isKeyPressed(GLFW_KEY_J) && (currentTime - lastEyeLightChange) > 0.5)
    {
      // "H" key was pressed. Toggle the eye light and create/destroy it accordingly.
      isEyeLightPresent = !isEyeLightPresent;
      if (isEyeLightPresent)
      {
        createEyeLight(getModelPosition());
      }
      else
      {
        destroyEyeLight();
      }
      // Update the timestamp for when the eye light was last toggled.
      lastEyeLightChange = currentTime;
    }

    // Get the player position.
    auto newPosition = getModelPosition();
    // Depending on which key was pressed, move player accordingly (up, down, left, or right).
    if (controlManager.isKeyPressed(GLFW_KEY_W))
    {
      newPosition.y += deltaTime * keyboardSpeed;
    }
    if (controlManager.isKeyPressed(GLFW_KEY_S))
    {
      newPosition.y -= deltaTime * keyboardSpeed;
    }
    if (controlManager.isKeyPressed(GLFW_KEY_D))
    {
      newPosition.x += deltaTime * keyboardSpeed;
    }
    if (controlManager.isKeyPressed(GLFW_KEY_A))
    {
      newPosition.x -= deltaTime * keyboardSpeed;
    }

    // Clamp the player position within the defined bounds of the player space.
    newPosition = glm::vec3(glm::clamp<double>(newPosition.x, -11.0, 11.0), glm::clamp<double>(newPosition.y, -6.0, 6.0), newPosition.z);
    // Update the shot position.
    setModelPosition(newPosition);
    // Update the eye lights.
    updateEyeLight(newPosition);

    // Check if "Space" key was pressed after 500ms since the last shot creation.
    if (controlManager.isKeyPressed(GLFW_KEY_SPACE) && (currentTime - lastShot) > 0.25)
    {
      // "Space" was pressed. Create a new shot and set its properties.
      auto newShot = ShotModel::create("Shot" + std::to_string(shotId));
      newShot->setModelPosition(glm::vec3(newPosition.x, newPosition.y, newPosition.z - 1.0));
      // Register the shot model.
      modelManager.registerModel(newShot);

      // Increment the next usable shot ID.
      shotId++;
      // Update the timestamp for when a shot was last created.
      lastShot = currentTime;
    }

    // Set the timestamp for the start of the last update to the starting timestamp of the current update.
    lastTime = currentTime;
  }
};

// Initialize the keyboard speed static variable.
float PlayerModel::keyboardSpeed = 10.0f;
// Initialize the eye light toggle static variable.
bool PlayerModel::isEyeLightPresent = true;
// Initialize the last time the eye light toggle was changed static variable.
double PlayerModel::lastEyeLightChange = -1;

#endif