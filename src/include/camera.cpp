#ifndef INCLUDE_CAMERA_CPP
#define INCLUDE_CAMERA_CPP

#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "../include/models.cpp"

class CameraBase
{

private:
  std::string cameraName;

  glm::vec3 position;
  glm::vec3 direction;
  glm::vec3 up;

  glm::mat4 viewMatrix;
  glm::mat4 projectionMatrix;

public:
  CameraBase(std::string name, glm::vec3 position, glm::vec3 direction, glm::vec3 up, glm::mat4 projectionMatrix)
      : cameraName(name),
        position(position),
        direction(direction),
        up(up),
        projectionMatrix(projectionMatrix)
  {
    std::cout << "[CameraBase] "
              << "Constructing CameraBase." << std::endl;
    std::cout << "[CameraBase] "
              << "Constructed CameraBase." << std::endl;
  }

  std::string getCameraName()
  {
    return cameraName;
  }

  glm::vec3 &getPosition()
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

  void setPosition(glm::vec3 newPosition)
  {
    std::cout << "[CameraBase] "
              << "Set position: " << glm::to_string(newPosition) << std::endl;
    position = newPosition;
  }

  void setDirection(glm::vec3 newDirection)
  {
    std::cout << "[CameraBase] "
              << "Set direction: " << glm::to_string(newDirection) << std::endl;
    direction = newDirection;
  }

  void setUp(glm::vec3 newUp)
  {
    std::cout << "[CameraBase] "
              << "Set up: " << glm::to_string(newUp) << std::endl;
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

  void update()
  {
    std::cout << "[CameraBase] "
              << "Updating view matrix." << std::endl
              << "[CameraBase] "
              << "\tPosition: " << glm::to_string(position) << std::endl
              << "[CameraBase] "
              << "\tDirection: " << glm::to_string(direction) << std::endl
              << "[CameraBase] "
              << "\tUp: " << glm::to_string(up) << std::endl;
    viewMatrix = glm::lookAt(position, position + direction, up);

    projectionMatrix = updateProjectionMatrix();
  }

  virtual glm::mat4 updateProjectionMatrix() = 0;
};

#endif