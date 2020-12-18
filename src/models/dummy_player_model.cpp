#ifndef MODELS_DUMMY_PLAYER_MODEL_CPP
#define MODELS_DUMMY_PLAYER_MODEL_CPP

#include <string>
#include <memory>
#include <any>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "model_base.cpp"

/**
 * Class that represents a player model.
 */
class DummyPlayerModel : public ModelBase<DummyPlayerModel>
{
private:
  float_t rotationSpeedY;

  float_t lastTime;

public:
  DummyPlayerModel(const std::string &modelId)
      : ModelBase(
            modelId,
            glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(glm::pi<float_t>() / 2.0f, 0.0f, 0.0f), glm::vec3(0.075f, 0.075f, 0.075f),
            ColliderShapeType::BOX),
        rotationSpeedY(-1.0f),
        lastTime(glfwGetTime()) {}

  static void initModel()
  {
    ModelBase::initModelDeps(
        "DummyPlayer",
        "assets/objects/spaceship.obj",
        "assets/textures/spaceship.bmp",
        "assets/shaders/vertex/unlit.glsl", "assets/shaders/fragment/unlit.glsl");
  }

  static void deinitModel()
  {
    ModelBase::deinitModelDeps();
  }

  const static std::shared_ptr<DummyPlayerModel> create(const std::string &modelId)
  {
    return std::make_shared<DummyPlayerModel>(modelId);
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