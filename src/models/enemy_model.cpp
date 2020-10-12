#ifndef MODELS_ENEMY_MODEL_CPP
#define MODELS_ENEMY_MODEL_CPP

#include <string>
#include <memory>
#include <random>
#include <ctime>

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
  static std::mt19937 mtGenerator;
  static std::uniform_real_distribution<double> mtInitialRotationDistribution;
  static std::uniform_real_distribution<double> mtRotationSpeedDistribution;

  ModelManager &modelManager;

  double rotationSpeedX;
  double rotationSpeedY;
  double rotationSpeedZ;

  double lastTime;

public:
  EnemyModel(const std::string &modelId)
      : ModelBase(
            modelId,
            "Enemy",
            glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(mtInitialRotationDistribution(mtGenerator), mtInitialRotationDistribution(mtGenerator), mtInitialRotationDistribution(mtGenerator)), glm::vec3(1.0f, 1.0f, 1.0f),
            "assets/objects/cube.obj",
            "assets/textures/cube.bmp",
            "assets/shaders/vertex/default.glsl", "assets/shaders/fragment/default.glsl",
            ColliderShapeType::BOX),
        modelManager(ModelManager::getInstance()),
        rotationSpeedX(mtRotationSpeedDistribution(mtGenerator)),
        rotationSpeedY(mtRotationSpeedDistribution(mtGenerator)),
        rotationSpeedZ(mtRotationSpeedDistribution(mtGenerator)),
        lastTime(glfwGetTime()) {}

  const static std::shared_ptr<EnemyModel> create(const std::string &modelId)
  {
    return std::make_shared<EnemyModel>(modelId);
  };

  void update() override
  {
    const auto currentTime = glfwGetTime();
    const auto deltaTime = currentTime - lastTime;

    setModelRotation(getModelRotation() - glm::vec3(rotationSpeedX * deltaTime, rotationSpeedY * deltaTime, rotationSpeedZ * deltaTime));

    lastTime = currentTime;
  }
};

std::mt19937 EnemyModel::mtGenerator = std::mt19937(std::clock());
std::uniform_real_distribution<double> EnemyModel::mtInitialRotationDistribution = std::uniform_real_distribution<double>(0.0, 359.0);
std::uniform_real_distribution<double> EnemyModel::mtRotationSpeedDistribution = std::uniform_real_distribution<double>(0.0, 5.0);

#endif