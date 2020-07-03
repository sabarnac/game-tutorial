#ifndef MODELS_PERSPECTIVE_CAMERA_MODEL_CPP
#define MODELS_PERSPECTIVE_CAMERA_MODEL_CPP

#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "../include/camera.cpp"
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

public:
  PerspectiveCamera(glm::vec3 position, glm::vec3 direction, glm::vec3 up, double horizontalAngle, double verticalAngle, double fieldOfView, double aspectRatio, double nearPlane, double farPlane)
      : CameraBase(
            "PerspectiveCamera",
            position,
            direction,
            up,
            glm::perspective(glm::radians(fieldOfView), aspectRatio, nearPlane, farPlane)),
        controlManager(ControlManager::getInstance()),
        lastTime(glfwGetTime()),
        horizontalAngle(horizontalAngle),
        verticalAngle(verticalAngle),
        fieldOfView(fieldOfView),
        aspectRatio(aspectRatio),
        nearPlane(nearPlane),
        farPlane(farPlane)
  {
    std::cout << "[PerspectiveCamera] "
              << "Constructing PerspectiveCamera." << std::endl;
    update();
    std::cout << "[PerspectiveCamera] "
              << "Constructed PerspectiveCamera." << std::endl;
  }

  void update()
  {
    std::cout << "[PerspectiveCamera] "
              << "Updating camera attributes: " << getCameraName() << std::endl;

    // Compute time difference between current and last frame
    double currentTime = glfwGetTime();
    float deltaTime = float(currentTime - lastTime);

    std::cout << "[PerspectiveCamera] "
              << "Time since last update: " << deltaTime << std::endl;

    auto currentCursorPosition = controlManager.getCursorPosition();
    std::cout << "[PerspectiveCamera] "
              << "Cursor position: " << currentCursorPosition->getX() << " | " << currentCursorPosition->getY() << std::endl;

    controlManager.setCursorPosition(CursorPosition(0.5, 0.5));
    std::cout << "[PerspectiveCamera] "
              << "Resetting cursor position." << std::endl;

    // Compute new orientation
    horizontalAngle += mouseSpeed * (0.5 - currentCursorPosition->getX());
    verticalAngle += mouseSpeed * (0.5 - currentCursorPosition->getY());
    std::cout << "[PerspectiveCamera] "
              << "Cursor movement angles: " << horizontalAngle << " | " << verticalAngle << std::endl;

    // Direction : Spherical coordinates to Cartesian coordinates conversion
    glm::vec3 newDirection(
        cos(verticalAngle) * sin(horizontalAngle),
        sin(verticalAngle),
        cos(verticalAngle) * cos(horizontalAngle));
    setDirection(newDirection);
    std::cout << "[PerspectiveCamera] "
              << "New camera direction: " << glm::to_string(newDirection) << std::endl;

    // Right vector
    glm::vec3 right = glm::vec3(
        sin(horizontalAngle - glm::pi<double>() / 2.0),
        0,
        cos(horizontalAngle - glm::pi<double>() / 2.0));

    // Up vector
    glm::vec3 up = glm::cross(right, newDirection);
    std::cout << "[PerspectiveCamera] "
              << "Camera up direction: " << glm::to_string(up) << std::endl;

    // Move forward
    glm::vec3 positionChange = glm::vec3(0.0);
    if (controlManager.isKeyPressed(GLFW_KEY_W))
    {
      positionChange = newDirection * deltaTime * keyboardSpeed;
    }
    // Move backward
    if (controlManager.isKeyPressed(GLFW_KEY_S))
    {
      positionChange = -1.0f * newDirection * deltaTime * keyboardSpeed;
    }
    // Strafe right
    if (controlManager.isKeyPressed(GLFW_KEY_D))
    {
      positionChange = right * deltaTime * keyboardSpeed;
    }
    // Strafe left
    if (controlManager.isKeyPressed(GLFW_KEY_A))
    {
      positionChange = -1.0f * right * deltaTime * keyboardSpeed;
    }
    std::cout << "[PerspectiveCamera] "
              << "Camera position change: " << glm::to_string(positionChange) << std::endl;

    setPosition(getPosition() + positionChange);
    setDirection(newDirection);
    setUp(up);

    CameraBase::update();

    std::cout << "[PerspectiveCamera] "
              << "Updated camera attributes." << std::endl;

    // For the next frame, the "last time" will be "now"
    lastTime = currentTime;
  }

  static std::shared_ptr<PerspectiveCamera> create(
      glm::vec3 position,
      double horizontalAngle,
      double verticalAngle,
      double fieldOfView,
      double aspectRatio,
      double nearPlane,
      double farPlane)
  {
    std::cout << "[PerspectiveCamera] "
              << "Creating new perspective camera." << std::endl;

    glm::vec3 direction(
        cos(verticalAngle) * sin(horizontalAngle),
        sin(verticalAngle),
        cos(verticalAngle) * cos(horizontalAngle));

    // Right vector
    glm::vec3 right = glm::vec3(
        sin(horizontalAngle - glm::pi<double>() / 2.0),
        0,
        cos(horizontalAngle - glm::pi<double>() / 2.0));

    // Up vector
    glm::vec3 up = glm::cross(right, direction);

    return std::make_shared<PerspectiveCamera>(position, direction, up, horizontalAngle, verticalAngle, fieldOfView, aspectRatio, nearPlane, farPlane);
  };

  glm::mat4 updateProjectionMatrix() override
  {
    std::cout << "[PerspectiveCamera] "
              << "Updating projection matrix." << std::endl
              << "[PerspectiveCamera] "
              << "\tField of View: " << fieldOfView << std::endl
              << "[PerspectiveCamera] "
              << "\tAspect Ratio: " << aspectRatio << std::endl
              << "[PerspectiveCamera] "
              << "\tNear Plane: " << nearPlane << std::endl
              << "[PerspectiveCamera] "
              << "\tFar Plane: " << nearPlane << std::endl;
    return glm::perspective(glm::radians(fieldOfView), aspectRatio, nearPlane, farPlane);
  }
};

float PerspectiveCamera::mouseSpeed = 5.0f;
float PerspectiveCamera::keyboardSpeed = 3.0f;

#endif