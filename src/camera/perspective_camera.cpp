#ifndef MODELS_PERSPECTIVE_CAMERA_MODEL_CPP
#define MODELS_PERSPECTIVE_CAMERA_MODEL_CPP

#include <string>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

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

  void updateCamera()
  {
    glm::vec3 newDirection(
        cos(verticalAngle) * sin(horizontalAngle),
        sin(verticalAngle),
        cos(verticalAngle) * cos(horizontalAngle));
    setCameraDirection(newDirection);

    glm::vec3 right = glm::vec3(
        sin(horizontalAngle - glm::pi<double>() / 2.0),
        0,
        cos(horizontalAngle - glm::pi<double>() / 2.0));

    glm::vec3 up = glm::cross(right, newDirection);

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
        acceptInput(false)
  {
    update();
  }

  void init() override
  {
    setCameraPosition(glm::vec3(0.0, 20.0, 40.0));
    horizontalAngle = glm::pi<double>();
    verticalAngle = -(glm::pi<double>() / 4.1);
  }

  void update() override
  {
    if (controlManager.isKeyPressed(GLFW_KEY_M))
    {
      acceptInput = !acceptInput;
      if (!acceptInput)
      {
        init();
      }
    }

    if (!acceptInput)
    {
      updateCamera();
      controlManager.setCursorPosition(CursorPosition(0.5, 0.5));
      return;
    }

    double currentTime = glfwGetTime();
    float deltaTime = float(currentTime - lastTime);

    auto currentCursorPosition = controlManager.getCursorPosition();
    controlManager.setCursorPosition(CursorPosition(0.5, 0.5));

    horizontalAngle += mouseSpeed * (0.5 - currentCursorPosition->getX());
    verticalAngle += mouseSpeed * (0.5 - currentCursorPosition->getY());

    glm::vec3 right = glm::vec3(
        sin(horizontalAngle - glm::pi<double>() / 2.0),
        0,
        cos(horizontalAngle - glm::pi<double>() / 2.0));

    glm::vec3 newPosition = getPosition();
    if (controlManager.isKeyPressed(GLFW_KEY_UP))
    {
      newPosition += getDirection() * deltaTime * keyboardSpeed;
    }
    if (controlManager.isKeyPressed(GLFW_KEY_DOWN))
    {
      newPosition -= getDirection() * deltaTime * keyboardSpeed;
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

    updateCamera();

    lastTime = currentTime;
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