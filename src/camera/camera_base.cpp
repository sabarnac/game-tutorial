#ifndef CAMERA_CAMERA_BASE_CPP
#define CAMERA_CAMERA_BASE_CPP

#include <iostream>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/**
 * Base class for creating cameras.
 */
class CameraBase
{

private:
  // The ID of the camera.
  std::string cameraId;
  // The name of the camera.
  std::string cameraName;

  // The position of the camera.
  glm::vec3 position;
  // The direction the camera is facing.
  glm::vec3 direction;
  // The direction that is the up direction for the camera.
  glm::vec3 up;

  // The view matrix of the camera.
  glm::mat4 viewMatrix;
  // The projection matrix of the camera.
  glm::mat4 projectionMatrix;

protected:
  CameraBase(std::string cameraId, std::string cameraName, glm::vec3 position, glm::vec3 direction, glm::vec3 up, glm::mat4 projectionMatrix)
      : cameraId(cameraId),
        cameraName(cameraName),
        position(position),
        direction(direction),
        up(up),
        projectionMatrix(projectionMatrix) {}

  virtual ~CameraBase() {}

public:
  /**
   * Get the ID of the camera.
   * 
   * @return The camera ID.
   */
  std::string getCameraId()
  {
    return cameraId;
  }

  /**
   * Get the name of the camera.
   * 
   * @return The camera name.
   */
  std::string getCameraName()
  {
    return cameraName;
  }

  /**
   * Get the position of the camera.
   * 
   * @return The camera position.
   */
  glm::vec3 &getCameraPosition()
  {
    return position;
  }

  /**
   * Get the direction the camera is facing.
   * 
   * @return The camera view direction.
   */
  glm::vec3 &getCameraDirection()
  {
    return direction;
  }

  /**
   * Get the up vector of the camera.
   * 
   * @return The camera up vector.
   */
  glm::vec3 &getCameraUp()
  {
    return up;
  }

  /**
   * Get the view matrix of the camera.
   * 
   * @return The camera view matrix.
   */
  glm::mat4 &getViewMatrix()
  {
    return viewMatrix;
  }

  /**
   * Get the projection matrix of the camera.
   * 
   * @return The camera projection matrix.
   */
  glm::mat4 &getProjectionMatrix()
  {
    return projectionMatrix;
  }

  /**
   * Set the position of the camera.
   * 
   * @param newPosition  The camera position.
   */
  void setCameraPosition(glm::vec3 newPosition)
  {
    position = newPosition;
  }

  /**
   * Set the direction the camera is facing.
   * 
   * @param newDirection  The camera view direction.
   */
  void setCameraDirection(glm::vec3 newDirection)
  {
    direction = newDirection;
  }

  /**
   * Set the up vector of the camera.
   * 
   * @param newUp  The camera up vector.
   */
  void setCameraUp(glm::vec3 newUp)
  {
    up = newUp;
  }

  /**
   * Initialize the camera once registered.
   */
  virtual void init() {}

  /**
   * De-initialize the camera once de-registered.
   */
  virtual void deinit() {}

  /**
   * Update the camera during the update step before starting rendering.
   */
  virtual void update()
  {
    // Calculate the view matrix of the camera.
    viewMatrix = glm::lookAt(position, position + direction, up);
    // Calculate the projection matrix of the camera.
    projectionMatrix = createProjectionMatrix();
  }

  /**
   * Calculates and returns a projection matrix. Has to be implemented by child classes.
   * 
   * @return The calculated projection matrix.
   */
  virtual glm::mat4 createProjectionMatrix() = 0;
};

#endif