#ifndef MODELS_DUMMY_ENEMY_MODEL_CPP
#define MODELS_DUMMY_ENEMY_MODEL_CPP

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

#include "model_base.cpp"

class DummyEnemyModel : public ModelBase<DummyEnemyModel>
{
private:
  float_t rotationSpeedY;

  float_t lastTime;

public:
  DummyEnemyModel(const std::string &modelId)
      : ModelBase(
            modelId,
            glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.1f, 0.1f, 0.1f),
            ColliderShapeType::SPHERE),
        rotationSpeedY(-1.0f),
        lastTime(glfwGetTime()) {}

  static void initModel()
  {
    ModelBase::initModelDeps(
        "DummyEnemy",
        "assets/objects/sphere-saw.obj",
        "assets/textures/sphere-saw.bmp",
        "assets/shaders/vertex/unlit.glsl", "assets/shaders/fragment/unlit.glsl");
  }

  static void deinitModel()
  {
    ModelBase::deinitModelDeps();
  }

  const static std::shared_ptr<DummyEnemyModel> create(const std::string &modelId)
  {
    return std::make_shared<DummyEnemyModel>(modelId);
  }

  void update() override
  {
    const auto currentTime = glfwGetTime();
    const auto deltaTime = currentTime - lastTime;

    setModelRotation(getModelRotation() - glm::vec3(0.0f, rotationSpeedY * deltaTime, 0.0f));

    lastTime = currentTime;
  }
};

#endif