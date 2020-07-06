#ifndef CAMERA_CAMERA_BASE_CPP
#define CAMERA_CAMERA_BASE_CPP

#include <iostream>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class CameraBase
{

private:
  std::string cameraId;
  std::string cameraName;

  glm::vec3 position;
  glm::vec3 direction;
  glm::vec3 up;

  glm::mat4 viewMatrix;
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
  std::string getCameraId()
  {
    return cameraId;
  }

  std::string getCameraName()
  {
    return cameraName;
  }

  glm::vec3 &getCameraPosition()
  {
    return position;
  }

  glm::vec3 &getDirection()
  {
    return direction;
  }

  glm::vec3 &getUp()
  {
    return up;
  }

  void setCameraPosition(glm::vec3 newPosition)
  {
    position = newPosition;
  }

  void setCameraDirection(glm::vec3 newDirection)
  {
    direction = newDirection;
  }

  void setCameraUp(glm::vec3 newUp)
  {
    up = newUp;
  }

  glm::mat4 &getViewMatrix()
  {
    return viewMatrix;
  }

  glm::mat4 &getProjectionMatrix()
  {
    return projectionMatrix;
  }

  virtual void init() {}

  virtual void deinit() {}

  virtual void update()
  {
    viewMatrix = glm::lookAt(position, position + direction, up);

    projectionMatrix = createProjectionMatrix();
  }

  virtual glm::mat4 createProjectionMatrix() = 0;
};

#endif