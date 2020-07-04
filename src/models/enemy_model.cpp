#ifndef MODELS_ENEMY_MODEL_CPP
#define MODELS_ENEMY_MODEL_CPP

#include <string>
#include <memory>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "../include/models.cpp"

#include "model_base.cpp"

class EnemyModel : public ModelBase
{
private:
  double lastTime;

  ModelManager &modelManager;

public:
  EnemyModel(std::string modelId)
      : ModelBase(
            modelId,
            "Enemy",
            glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f),
            "assets/objects/cube.obj",
            "assets/textures/cube.bmp", TextureType::BMP,
            "assets/shaders/vertex/default.glsl", "assets/shaders/fragment/default.glsl",
            ColliderShapeType::BOX),
        modelManager(ModelManager::getInstance()),
        lastTime(glfwGetTime()) {}

  static std::shared_ptr<EnemyModel> create(std::string modelId)
  {
    return std::make_shared<EnemyModel>(modelId);
  };

  void update() override
  {
    auto currentTime = glfwGetTime();
    auto deltaTime = currentTime - lastTime;

    setModelRotation(getModelRotation() - glm::vec3(1.0 * deltaTime, 1.5 * deltaTime, 2.25 * deltaTime));

    lastTime = currentTime;
  }
};

#endif