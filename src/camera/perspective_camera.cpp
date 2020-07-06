#ifndef MODELS_PERSPECTIVE_CAMERA_MODEL_CPP
#define MODELS_PERSPECTIVE_CAMERA_MODEL_CPP

#include <string>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "camera_base.cpp"
#include "../include/control.cpp"

class PerspectiveCamera : public CameraBase
{
private:
  static float keyboardSpeed;
  static float mouseSpeed;

  ControlManager &controlManager;

  double lastTime;
  double horizontalAngle;
  double verticalAngle;
  double fieldOfView;
  double aspectRatio;
  double nearPlane;
  double farPlane;

  bool acceptInput;
  double lastAcceptInputChange;

  void updateCamera(glm::vec3 newDirection)
  {
    auto right = glm::vec3(
        sin(horizontalAngle - glm::pi<double>() / 2.0),
        0,
        cos(horizontalAngle - glm::pi<double>() / 2.0));

    auto up = glm::cross(right, newDirection);

    setCameraDirection(newDirection);
    setCameraUp(up);

    CameraBase::update();
  }

public:
  PerspectiveCamera(std::string cameraId)
      : CameraBase(
            cameraId,
            "PerspectiveCamera",
            glm::vec3(0.0),
            glm::vec3(0.0),
            glm::vec3(0.0, 1.0, 0.0),
            glm::perspective(glm::radians(90.0), 4.0 / 3.0, 0.1, 100.0)),
        controlManager(ControlManager::getInstance()),
        lastTime(glfwGetTime()),
        horizontalAngle(0.0),
        verticalAngle(0.0),
        fieldOfView(60.0),
        aspectRatio(4.0 / 3.0),
        nearPlane(0.1),
        farPlane(100.0),
        acceptInput(false),
        lastAcceptInputChange(glfwGetTime() - 10.0)
  {
    update();
  }

  void update() override
  {
    auto currentTime = glfwGetTime();
    auto deltaTime = float(currentTime - lastTime);

    if (controlManager.isKeyPressed(GLFW_KEY_M) && (currentTime - lastAcceptInputChange) > 0.5)
    {
      acceptInput = !acceptInput;
      if (!acceptInput)
      {
        setCameraPosition(glm::vec3(0.0, 20.0, 40.0));
        setCameraAngles(glm::pi<double>(), -(glm::pi<double>() / 4.1));
      }
      lastAcceptInputChange = currentTime;
    }

    if (!acceptInput)
    {
      auto newDirection = glm::vec3(
          cos(verticalAngle) * sin(horizontalAngle),
          sin(verticalAngle),
          cos(verticalAngle) * cos(horizontalAngle));
      updateCamera(newDirection);
      controlManager.setCursorPosition(CursorPosition(0.5, 0.5));
      return;
    }

    auto currentCursorPosition = controlManager.getCursorPosition();
    controlManager.setCursorPosition(CursorPosition(0.5, 0.5));

    horizontalAngle += mouseSpeed * (0.5 - currentCursorPosition->getX());
    verticalAngle += mouseSpeed * (0.5 - currentCursorPosition->getY());

    auto newDirection = glm::vec3(
        cos(verticalAngle) * sin(horizontalAngle),
        sin(verticalAngle),
        cos(verticalAngle) * cos(horizontalAngle));

    auto right = glm::vec3(
        sin(horizontalAngle - glm::pi<double>() / 2.0),
        0,
        cos(horizontalAngle - glm::pi<double>() / 2.0));

    auto newPosition = getCameraPosition();

    if (controlManager.isKeyPressed(GLFW_KEY_UP))
    {
      newPosition += newDirection * deltaTime * keyboardSpeed;
    }
    if (controlManager.isKeyPressed(GLFW_KEY_DOWN))
    {
      newPosition -= newDirection * deltaTime * keyboardSpeed;
    }
    if (controlManager.isKeyPressed(GLFW_KEY_RIGHT))
    {
      newPosition += right * deltaTime * keyboardSpeed;
    }
    if (controlManager.isKeyPressed(GLFW_KEY_LEFT))
    {
      newPosition -= right * deltaTime * keyboardSpeed;
    }
    setCameraPosition(newPosition);

    updateCamera(newDirection);

    lastTime = currentTime;
  }

  double getHorizontalAngle()
  {
    return horizontalAngle;
  }

  double getVerticalAngle()
  {
    return verticalAngle;
  }

  void setCameraAngles(double newHorizontalAngle, double newVerticalAngle)
  {
    horizontalAngle = newHorizontalAngle;
    verticalAngle = newVerticalAngle;
    auto newDirection = glm::vec3(
        cos(verticalAngle) * sin(horizontalAngle),
        sin(verticalAngle),
        cos(verticalAngle) * cos(horizontalAngle));
    updateCamera(newDirection);
  }

  static std::shared_ptr<PerspectiveCamera> create(std::string cameraId)
  {
    return std::make_shared<PerspectiveCamera>(cameraId);
  };

  glm::mat4 createProjectionMatrix() override
  {
    return glm::perspective(glm::radians(fieldOfView), aspectRatio, nearPlane, farPlane);
  }
};

float PerspectiveCamera::mouseSpeed = 5.0f;
float PerspectiveCamera::keyboardSpeed = 25.0f;

#endif