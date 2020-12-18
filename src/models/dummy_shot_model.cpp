#ifndef MODELS_DUMMY_SHOT_MODEL_CPP
#define MODELS_DUMMY_SHOT_MODEL_CPP

#include <string>
#include <memory>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "model_base.cpp"

/**
 * Class that represents a shot/bullet model.
 */
class DummyShotModel : public ModelBase<DummyShotModel>
{
private:
  float_t rotationSpeedY;

  float_t lastTime;

public:
  DummyShotModel(const std::string &modelId)
      : ModelBase(
            modelId,
            glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3((-1.0f * glm::pi<float_t>()) / 2, 0.0f, 0.0f), glm::vec3(0.01f, 0.01f, 0.01f),
            ColliderShapeType::BOX),
        rotationSpeedY(-1.0f),
        lastTime(glfwGetTime()) {}

  static void initModel()
  {
    ModelBase::initModelDeps(
        "DummyShot",
        "assets/objects/shot.obj",
        "assets/textures/shot.bmp",
        "assets/shaders/vertex/unlit.glsl", "assets/shaders/fragment/unlit.glsl");
  }

  static void deinitModel()
  {
    ModelBase::deinitModelDeps();
  }

  const static std::shared_ptr<DummyShotModel> create(const std::string &modelId)
  {
    return std::make_shared<DummyShotModel>(modelId);
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