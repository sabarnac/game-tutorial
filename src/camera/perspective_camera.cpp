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
  static float keyboardSpeed;
  // The speed of movement with mouse inputs.
  static float mouseSpeed;

  // The control manager responsible for managing controls and inputs of the window.
  ControlManager &controlManager;

  // The timestamp of the last time the update for the camera was started.
  double lastTime;
  // The horizontal angle of the camera.
  double horizontalAngle;
  // The vertical angle of the camera.
  double verticalAngle;
  // The FoV of the camera.
  double fieldOfView;
  // The aspect ratio of the camera.
  double aspectRatio;
  // The closest distance the camera can capture from.
  double nearPlane;
  // The farthest distance the camera can capture till.
  double farPlane;

  // Whether to accept input or not.
  bool acceptInput;
  // The last time the ability to accept input was changed.
  double lastAcceptInputChange;

  /**
   * Update the camera.
   * 
   * @param newDirection  The new direction the camera is pointing towards.
   */
  void updateCamera(glm::vec3 newDirection)
  {
    // Calculate the vector pointing to the right of the camera.
    auto right = glm::vec3(
        sin(horizontalAngle - glm::pi<double>() / 2.0),
        0,
        cos(horizontalAngle - glm::pi<double>() / 2.0));

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
        lastAcceptInputChange(glfwGetTime() - 10.0) {}

  void update() override
  {
    // Get the current time for the start of the update.
    auto currentTime = glfwGetTime();
    // Get the time difference since the start of the last update.
    auto deltaTime = float(currentTime - lastTime);

    // Check if the M key was pressed after 500ms since the last accept input change.
    if (controlManager.isKeyPressed(GLFW_KEY_M) && (currentTime - lastAcceptInputChange) > 0.5)
    {
      // "M" key was pressed. Toggle accepting input.
      acceptInput = !acceptInput;
      if (!acceptInput)
      {
        // If input is no longer being accepted, reset the camera back to the base position.
        setCameraPosition(glm::vec3(0.0, 20.0, 40.0));
        setCameraAngles(glm::pi<double>(), -(glm::pi<double>() / 4.1));
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
    auto currentCursorPosition = controlManager.getCursorPosition();
    controlManager.setCursorPosition(CursorPosition(0.5, 0.5));

    // Calculate the camera angles based on the mouse movement.
    horizontalAngle += mouseSpeed * (0.5 - currentCursorPosition->getX());
    verticalAngle += mouseSpeed * (0.5 - currentCursorPosition->getY());

    // Calculate the new direction the camera is pointing towards.
    auto newDirection = glm::vec3(
        cos(verticalAngle) * sin(horizontalAngle),
        sin(verticalAngle),
        cos(verticalAngle) * cos(horizontalAngle));

    // Calculate the right vector of he camera.
    auto right = glm::vec3(
        sin(horizontalAngle - glm::pi<double>() / 2.0),
        0,
        cos(horizontalAngle - glm::pi<double>() / 2.0));

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
  double getHorizontalAngle()
  {
    return horizontalAngle;
  }

  /**
   * Get the vertical angle of the camera.
   * 
   * @return The camera vertical angle.
   */
  double getVerticalAngle()
  {
    return verticalAngle;
  }

  /**
   * Set the angles of the camera.
   * 
   * @param newHorizontalAngle  The camera horizontal angle.
   * @param newVerticalAngle    The camera vertical angle.
   */
  void setCameraAngles(double newHorizontalAngle, double newVerticalAngle)
  {
    // Set the camera angles.
    horizontalAngle = newHorizontalAngle;
    verticalAngle = newVerticalAngle;

    // Calculate the new direction of the camera.
    auto newDirection = glm::vec3(
        cos(verticalAngle) * sin(horizontalAngle),
        sin(verticalAngle),
        cos(verticalAngle) * cos(horizontalAngle));

    // Update the camera.
    updateCamera(newDirection);
  }

  /**
   * Creates a new instance of the perspective camera.
   */
  static std::shared_ptr<PerspectiveCamera> create(std::string cameraId)
  {
    return std::make_shared<PerspectiveCamera>(cameraId);
  };

  /**
   * Calculates and returns the projection matrix of the camera.
   * 
   * @return The camera's projection matrix.
   */
  glm::mat4 createProjectionMatrix() override
  {
    return glm::perspective(glm::radians(fieldOfView), aspectRatio, nearPlane, farPlane);
  }
};

// Initialize the mouse speed static variable.
float PerspectiveCamera::mouseSpeed = 5.0f;
// Initialize the keyboard speed static variable.
float PerspectiveCamera::keyboardSpeed = 20.0f;
#endif