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

class PlayerModel : public ModelBase
{
private:
  static float keyboardSpeed;

  double lastTime;
  double lastShot;
  short shotId;

  std::shared_ptr<ConeLight> eyeLight1;
  std::shared_ptr<ConeLight> eyeLight2;

  ModelManager &modelManager;
  LightManager &lightManager;
  ControlManager &controlManager;

public:
  PlayerModel(std::string modelId)
      : ModelBase(
            modelId,
            "Player",
            glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f),
            "assets/objects/monkey.obj",
            "assets/textures/player.bmp", TextureType::BMP,
            "assets/shaders/vertex/default.glsl", "assets/shaders/fragment/default.glsl",
            ColliderShapeType::SPHERE),
        modelManager(ModelManager::getInstance()),
        lightManager(LightManager::getInstance()),
        controlManager(ControlManager::getInstance()),
        lastTime(glfwGetTime()),
        lastShot(glfwGetTime() - 10.0),
        shotId(0) {}

  static std::shared_ptr<PlayerModel> create(std::string modelId)
  {
    return std::make_shared<PlayerModel>(modelId);
  };

  void init() override
  {
    auto position = getModelPosition();

    setModelPosition(glm::vec3(0.0, 0.0, 30.0));
    setModelRotation(glm::vec3(glm::pi<double>() / 2, glm::pi<double>(), 0));

    eyeLight1 = ConeLight::create(getModelId() + "::EyeLight1");
    eyeLight1->setLightPosition(glm::vec3(position.x - 0.35, position.y + 0.25, position.z - 1.0));
    eyeLight1->setLightAngles(glm::pi<double>(), 0.0);
    eyeLight1->setLightIntensity(800.0);

    eyeLight2 = ConeLight::create(getModelId() + "::EyeLight2");
    eyeLight2->setLightPosition(glm::vec3(position.x + 0.35, position.y + 0.25, position.z - 1.0));
    eyeLight2->setLightAngles(glm::pi<double>(), 0.0);
    eyeLight2->setLightIntensity(800.0);

    lightManager.registerLight(eyeLight1);
    lightManager.registerLight(eyeLight2);
  }

  void update() override
  {
    auto currentTime = glfwGetTime();
    auto deltaTime = currentTime - lastTime;

    auto newPosition = getModelPosition();
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

    newPosition = glm::vec3(glm::clamp<double>(newPosition.x, -11.0, 11.0), glm::clamp<double>(newPosition.y, -6.0, 6.0), newPosition.z);
    setModelPosition(newPosition);
    eyeLight1->setLightPosition(glm::vec3(newPosition.x - 0.35, newPosition.y + 0.25, newPosition.z - 1.0));
    eyeLight2->setLightPosition(glm::vec3(newPosition.x + 0.35, newPosition.y + 0.25, newPosition.z - 1.0));

    if (controlManager.isKeyPressed(GLFW_KEY_SPACE) && (currentTime - lastShot) > 0.25)
    {
      auto newShot = ShotModel::create("Shot" + std::to_string(shotId));
      newShot->setModelPosition(glm::vec3(newPosition.x, newPosition.y, newPosition.z - 1.0));
      modelManager.registerModel(newShot);

      shotId++;
      lastShot = currentTime;
    }

    lastTime = currentTime;
  }
};

float PlayerModel::keyboardSpeed = 10.0f;

#endif