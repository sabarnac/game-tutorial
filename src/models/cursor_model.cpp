#ifndef MODELS_CURSOR_MODEL_CPP
#define MODELS_CURSOR_MODEL_CPP

#include <string>
#include <memory>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "../include/constants.cpp"
#include "../include/control.cpp"

#include "model_base.cpp"

/**
 * Class that represents a title model.
 */
class CursorModel : public ModelBase<CursorModel>
{
private:
  // The control manager responsible for managing controls and inputs of the window.
  ControlManager &controlManager;

  // Whether to accept input or not.
  bool acceptInput;
  // The last time the ability to accept input was changed.
  float_t lastAcceptInputChange;

  // The timestamp of the last time the update for the camera was started.
  float_t lastTime;

public:
  CursorModel(const std::string &modelId)
      : ModelBase(
            modelId,
            glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.025f, 0.05f, 2.5f),
            ColliderShapeType::BOX),
        controlManager(ControlManager::getInstance()),
        lastTime(glfwGetTime()),
        acceptInput(true),
        lastAcceptInputChange(glfwGetTime() - 10.0f) {}

  static void initModel()
  {
    ModelBase::initModelDeps(
        "Cursor",
        "assets/objects/cursor.obj",
        "assets/textures/cursor.bmp",
        "assets/shaders/vertex/unlit.glsl", "assets/shaders/fragment/unlit_black_alpha.glsl");
  }

  static void deinitModel()
  {
    ModelBase::deinitModelDeps();
  }

  /**
   * Creates a new instance of the title model.
   */
  const static std::shared_ptr<CursorModel> create(const std::string &modelId)
  {
    return std::make_shared<CursorModel>(modelId);
  }

  void update() override
  {
    // Get the current time for the start of the update.
    const auto currentTime = glfwGetTime();
    // Get the time difference since the start of the last update.
    const auto deltaTime = float_t(currentTime - lastTime);

    // Check if the M key was pressed after 500ms since the last accept input change.
    if (controlManager.isKeyPressed(GLFW_KEY_M) && (currentTime - lastAcceptInputChange) > 0.5f)
    {
      // "M" key was pressed. Toggle accepting input.
      acceptInput = !acceptInput;

      setModelPosition(glm::vec3(0.0f));
      controlManager.setCursorPosition(CursorPosition(0.5f, 0.5f));

      // Update the timestamp for the last time accept input flag was changed.
      lastAcceptInputChange = currentTime;
    }

    // If input is not being accepted, just consume mouse inputs and end.
    if (!acceptInput)
    {
      return;
    }

    const auto cursorPosition = controlManager.getCursorPosition();
    const auto clampedCursorPosition = glm::clamp(glm::vec2(cursorPosition->getX(), cursorPosition->getY()), glm::vec2(0.05f), glm::vec2(0.95f));
    controlManager.setCursorPosition(CursorPosition(clampedCursorPosition.x, clampedCursorPosition.y));

    const auto newCursorPosition = glm::vec3((2.0f * ASPECT_RATIO) * (cursorPosition->getX() - 0.5f), -2.0f * (cursorPosition->getY() - 0.5f), 0.0f);
    setModelPosition(newCursorPosition);
  }
};

#endif