#ifndef MODELS_PERSPECTIVE_CAMERA_MODEL_CPP
#define MODELS_PERSPECTIVE_CAMERA_MODEL_CPP

#include <string>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "camera_base.cpp"
#include "../include/control.cpp"

/**
 * Class that represents a 3D perspective-based camera.
 */
class PerspectiveCamera : public CameraBase
{
private:
  // The speed of movement with key inputs.
  const static float_t keyboardSpeed;
  // The speed of movement with mouse inputs.
  const static float_t mouseSpeed;

  // The control manager responsible for managing controls and inputs of the window.
  ControlManager &controlManager;

  // The FoV of the camera.
  const double_t fieldOfView;
  // The aspect ratio of the camera.
  const double_t aspectRatio;
  // The closest distance the camera can capture from.
  const double_t nearPlane;
  // The farthest distance the camera can capture till.
  const double_t farPlane;

  // The timestamp of the last time the update for the camera was started.
  double_t lastTime;
  // The horizontal angle of the camera.
  double_t horizontalAngle;
  // The vertical angle of the camera.
  double_t verticalAngle;
  // Whether to accept input or not.
  bool acceptInput;
  // The last time the ability to accept input was changed.
  double_t lastAcceptInputChange;

  /**
   * Update the camera.
   * 
   * @param newDirection  The new direction the camera is pointing towards.
   */
  void updateCamera(const glm::vec3 &newDirection)
  {
    // Calculate the vector pointing to the right of the camera.
    auto right = glm::vec3(
        sin(horizontalAngle - glm::pi<double_t>() / 2.0),
        0,
        cos(horizontalAngle - glm::pi<double_t>() / 2.0));

    // Calculate the vector pointing upward of the camera.
    auto up = glm::cross(right, newDirection);

    // Update the camera direction.
    setCameraDirection(newDirection);
    // Update the camera up vector.
    setCameraUp(up);

    // Call the base class update method to update the view and projection matrices.
    CameraBase::update();
  }

public:
  PerspectiveCamera(const std::string &cameraId)
      : CameraBase(
            cameraId,
            "Perspective",
            glm::vec3(0.0),
            glm::vec3(0.0),
            glm::vec3(0.0, 1.0, 0.0),
            glm::perspective(glm::radians(90.0), 4.0 / 3.0, 0.1, 100.0)),
        controlManager(ControlManager::getInstance()),
        fieldOfView(60.0),
        aspectRatio(4.0 / 3.0),
        nearPlane(0.1),
        farPlane(100.0),
        lastTime(glfwGetTime()),
        horizontalAngle(0.0),
        verticalAngle(0.0),
        acceptInput(false),
        lastAcceptInputChange(glfwGetTime() - 10.0) {}

  void update() override
  {
    // Get the current time for the start of the update.
    const auto currentTime = glfwGetTime();
    // Get the time difference since the start of the last update.
    const auto deltaTime = float_t(currentTime - lastTime);

    // Check if the M key was pressed after 500ms since the last accept input change.
    if (controlManager.isKeyPressed(GLFW_KEY_M) && (currentTime - lastAcceptInputChange) > 0.5)
    {
      // "M" key was pressed. Toggle accepting input.
      acceptInput = !acceptInput;
      if (!acceptInput)
      {
        // If input is no longer being accepted, reset the camera back to the base position.
        setCameraPosition(glm::vec3(0.0, 20.0, 40.0));
        setCameraAngles(glm::pi<double_t>(), -(glm::pi<double_t>() / 4.1));
      }
      // Update the timestamp for the last time accept input flag was changed.
      lastAcceptInputChange = currentTime;
      // Consume any keypresses used for moving the camera.
      controlManager.isKeyPressed(GLFW_KEY_UP);
      controlManager.isKeyPressed(GLFW_KEY_DOWN);
      controlManager.isKeyPressed(GLFW_KEY_RIGHT);
      controlManager.isKeyPressed(GLFW_KEY_LEFT);
    }

    // If input is not being accepted, just consume mouse inputs and end.
    if (!acceptInput)
    {
      controlManager.setCursorPosition(CursorPosition(0.5, 0.5));
      return;
    }

    // Get the position of the cursor and reset it back to the center of the screen.
    const auto currentCursorPosition = controlManager.getCursorPosition();
    controlManager.setCursorPosition(CursorPosition(0.5, 0.5));

    // Calculate the camera angles based on the mouse movement.
    horizontalAngle += mouseSpeed * (0.5 - currentCursorPosition->getX());
    verticalAngle += mouseSpeed * (0.5 - currentCursorPosition->getY());

    // Calculate the new direction the camera is pointing towards.
    const auto newDirection = glm::vec3(
        cos(verticalAngle) * sin(horizontalAngle),
        sin(verticalAngle),
        cos(verticalAngle) * cos(horizontalAngle));

    // Calculate the right vector of he camera.
    const auto right = glm::vec3(
        sin(horizontalAngle - glm::pi<double_t>() / 2.0),
        0,
        cos(horizontalAngle - glm::pi<double_t>() / 2.0));

    // Get the position of the camera.
    auto newPosition = getCameraPosition();

    // Based on what keys are pressed, update the camera to move forward, backward, left, or right.
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
    /// Update the camera position.
    setCameraPosition(newPosition);

    /// Update the camera.
    updateCamera(newDirection);

    // Set the timestamp for the start of the last update to the starting timestamp of the current update.
    lastTime = currentTime;
  }

  /**
   * Get the horizontal angle of the camera.
   * 
   * @return The camera horizontal angle.
   */
  const double_t &getHorizontalAngle() const
  {
    return horizontalAngle;
  }

  /**
   * Get the vertical angle of the camera.
   * 
   * @return The camera vertical angle.
   */
  const double_t &getVerticalAngle() const
  {
    return verticalAngle;
  }

  /**
   * Set the angles of the camera.
   * 
   * @param newHorizontalAngle  The camera horizontal angle.
   * @param newVerticalAngle    The camera vertical angle.
   */
  void setCameraAngles(const double_t &newHorizontalAngle, const double_t &newVerticalAngle)
  {
    // Set the camera angles.
    horizontalAngle = newHorizontalAngle;
    verticalAngle = newVerticalAngle;

    // Calculate the new direction of the camera.
    const auto newDirection = glm::vec3(
        cos(verticalAngle) * sin(horizontalAngle),
        sin(verticalAngle),
        cos(verticalAngle) * cos(horizontalAngle));

    // Update the camera.
    updateCamera(newDirection);
  }

  /**
   * Creates a new instance of the perspective camera.
   */
  const static std::shared_ptr<PerspectiveCamera> create(const std::string &cameraId)
  {
    return std::make_shared<PerspectiveCamera>(cameraId);
  }

  /**
   * Calculates and returns the projection matrix of the camera.
   * 
   * @return The camera's projection matrix.
   */
  const glm::mat4 createProjectionMatrix() override
  {
    return glm::perspective(glm::radians(fieldOfView), aspectRatio, nearPlane, farPlane);
  }
};

// Initialize the mouse speed static variable.
const float_t PerspectiveCamera::mouseSpeed = 5.0f;
// Initialize the keyboard speed static variable.
const float_t PerspectiveCamera::keyboardSpeed = 20.0f;
#endif