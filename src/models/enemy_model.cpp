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

class EnemyModel : public ModelBase<EnemyModel>
{
private:
  static std::mt19937 mtGenerator;
  static std::uniform_real_distribution<float_t> mtInitialRotationDistribution;
  static std::uniform_real_distribution<float_t> mtRotationSpeedDistribution;

  float_t rotationSpeedY;

  float_t lastTime;

public:
  EnemyModel(const std::string &modelId)
      : ModelBase(
            modelId,
            glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, mtInitialRotationDistribution(mtGenerator), 0.0f), glm::vec3(1.0f, 1.0f, 1.0f),
            ColliderShapeType::SPHERE),
        rotationSpeedY(mtRotationSpeedDistribution(mtGenerator)),
        lastTime(glfwGetTime()) {}

  static void initModel()
  {
    ModelBase::initModelDeps(
        "Enemy",
        "assets/objects/sphere-saw.obj",
        "assets/textures/sphere-saw.bmp",
        "assets/shaders/vertex/default.glsl", "assets/shaders/fragment/default.glsl");
  }

  static void deinitModel()
  {
    ModelBase::deinitModelDeps();
  }

  const static std::shared_ptr<EnemyModel> create(const std::string &modelId)
  {
    return std::make_shared<EnemyModel>(modelId);
  }

  void update() override
  {
    const auto currentTime = glfwGetTime();
    const auto deltaTime = currentTime - lastTime;

    setModelRotation(getModelRotation() - glm::vec3(0.0f, rotationSpeedY * deltaTime, 0.0f));

    lastTime = currentTime;
  }
};

std::mt19937 EnemyModel::mtGenerator = std::mt19937(std::clock());
std::uniform_real_distribution<float_t> EnemyModel::mtInitialRotationDistribution = std::uniform_real_distribution<float_t>(0.0f, glm::radians(359.99f));
std::uniform_real_distribution<float_t> EnemyModel::mtRotationSpeedDistribution = std::uniform_real_distribution<float_t>(glm::radians(30.0f), glm::radians(180.0f));

#endif