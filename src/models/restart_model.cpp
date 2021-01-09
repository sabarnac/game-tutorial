#ifndef MODELS_RESTART_MODEL_CPP
#define MODELS_RESTART_MODEL_CPP

#include <string>
#include <memory>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "../include/collider.cpp"
#include "../include/control.cpp"
#include "../include/models.cpp"

#include "model_base.cpp"

/**
 * Class that represents a restart button model.
 */
class RestartModel : public ModelBase<RestartModel>
{
private:
  inline static const auto DEFAULT_SCALE = glm::vec3(0.2f, 0.1f, 1.0f);

  // The model manager responsible for managing the models in the scene.
  ModelManager &modelManager;
  // The control manager responsible for managing controls and inputs of the window.
  const ControlManager &controlManager;

  bool _isClicked;

  std::shared_ptr<ModelBaseIntf> cursor;

public:
  RestartModel(const std::string &modelId)
      : ModelBase(
            modelId,
            glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), DEFAULT_SCALE,
            ColliderShapeType::BOX),
        modelManager(ModelManager::getInstance()),
        controlManager(ControlManager::getInstance()),
        _isClicked(false) {}

  static void initModel()
  {
    ModelBase::initModelDeps(
        "Start",
        "assets/objects/start.obj",
        "assets/textures/exit.bmp",
        "assets/shaders/vertex/unlit.glsl", "assets/shaders/fragment/unlit.glsl");
  }

  static void deinitModel()
  {
    ModelBase::deinitModelDeps();
  }

  /**
   * Creates a new instance of the start button model.
   */
  const static std::shared_ptr<RestartModel> create(const std::string &modelId)
  {
    return std::make_shared<RestartModel>(modelId);
  }

  bool isClicked()
  {
    return _isClicked;
  }

  void init() override
  {
    cursor = modelManager.getModel("Cursor");
  }

  void update() override
  {
    const auto cursorCollided = DeepCollisionValidator::haveShapesCollided(cursor->getColliderDetails()->getColliderShape(), this->getColliderDetails()->getColliderShape(), false);
    if (cursorCollided)
    {
      if (controlManager.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
      {
        _isClicked = true;
      }

      setModelScale(DEFAULT_SCALE * glm::vec3(1.1f));
    }
    else
    {
      setModelScale(DEFAULT_SCALE);
    }
  }
};

#endif