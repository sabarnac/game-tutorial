#ifndef LIGHT_POINT_LIGHT_CPP
#define LIGHT_POINT_LIGHT_CPP

#include <iostream>
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "light_base.cpp"

class PointLight : public LightBase
{

private:
  std::string lightId;
  std::string lightName;

  std::vector<glm::mat4> createViewMatrices()
  {
    auto position = getLightPosition();
    return std::vector<glm::mat4>({glm::lookAt(position, position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)),
                                   glm::lookAt(position, position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)),
                                   glm::lookAt(position, position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)),
                                   glm::lookAt(position, position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)),
                                   glm::lookAt(position, position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)),
                                   glm::lookAt(position, position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0))});
  }

  std::vector<glm::mat4> createProjectionMatrices(double nearPlane, double farPlane)
  {
    return std::vector<glm::mat4>({glm::perspective(glm::radians(90.0), 1.0, nearPlane, farPlane),
                                   glm::perspective(glm::radians(90.0), 1.0, nearPlane, farPlane),
                                   glm::perspective(glm::radians(90.0), 1.0, nearPlane, farPlane),
                                   glm::perspective(glm::radians(90.0), 1.0, nearPlane, farPlane),
                                   glm::perspective(glm::radians(90.0), 1.0, nearPlane, farPlane),
                                   glm::perspective(glm::radians(90.0), 1.0, nearPlane, farPlane)});
  }

public:
  PointLight(std::string lightId)
      : LightBase(
            lightId,
            "PointLight",
            glm::vec3(1.0), 100.0,
            "assets/shaders/vertex/point_light.glsl", "assets/shaders/geometry/point_light.glsl", "assets/shaders/fragment/point_light.glsl",
            glm::vec3(0.0),
            0.1, 100.0,
            createViewMatrices(), createProjectionMatrices(0.1, 100.0),
            ShadowBufferType::CUBE) {}

  virtual ~PointLight() {}

  void setLightPosition(glm::vec3 newPosition) override
  {
    LightBase::setLightPosition(newPosition);
    setViewMatrices(createViewMatrices());
  }

  void setLightNearPlane(double newNearPlane) override
  {
    LightBase::setLightNearPlane(newNearPlane);
    setProjectionMatrices(createProjectionMatrices(newNearPlane, getFarPlane()));
  }

  void setLightFarPlane(double newFarPlane) override
  {
    LightBase::setLightFarPlane(newFarPlane);
    setProjectionMatrices(createProjectionMatrices(getNearPlane(), newFarPlane));
  }

  static std::shared_ptr<PointLight> create(std::string lightId)
  {
    return std::make_shared<PointLight>(lightId);
  };
};

#endif