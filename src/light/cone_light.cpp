#ifndef LIGHT_CONE_LIGHT_CPP
#define LIGHT_CONE_LIGHT_CPP

#include <iostream>
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "light_base.cpp"

class ConeLight : public LightBase
{

private:
  std::string lightId;
  std::string lightName;

  double horizontalAngle;
  double verticalAngle;

  std::vector<glm::mat4> createViewMatrices(double horizontalAngle, double verticalAngle)
  {
    auto direction = glm::vec3(
        cos(verticalAngle) * sin(horizontalAngle),
        sin(verticalAngle),
        cos(verticalAngle) * cos(horizontalAngle));

    auto right = glm::vec3(
        sin(horizontalAngle - glm::pi<double>() / 2.0),
        0,
        cos(horizontalAngle - glm::pi<double>() / 2.0));

    auto up = glm::cross(right, direction);

    auto position = getLightPosition();
    return std::vector<glm::mat4>({glm::lookAt(position, position + direction, up)});
  }

  std::vector<glm::mat4> createProjectionMatrices(double nearPlane, double farPlane)
  {
    return std::vector<glm::mat4>({glm::perspective(glm::radians(90.0), 1.0, nearPlane, farPlane)});
  }

public:
  ConeLight(std::string lightId)
      : LightBase(
            lightId,
            "ConeLight",
            glm::vec3(1.0), 100.0,
            "assets/shaders/vertex/cone_light.glsl", "assets/shaders/fragment/cone_light.glsl",
            glm::vec3(0.0),
            0.1, 100.0,
            createViewMatrices(0.0, 0.0), createProjectionMatrices(0.1, 100.0),
            ShadowBufferType::SIMPLE),
        horizontalAngle(0.0),
        verticalAngle(0.0) {}

  virtual ~ConeLight() {}

  void setLightPosition(glm::vec3 newPosition) override
  {
    LightBase::setLightPosition(newPosition);
    setViewMatrices(createViewMatrices(horizontalAngle, verticalAngle));
  }

  void setLightAngles(double newHorizontalAngle, double newVerticalAngle)
  {
    horizontalAngle = newHorizontalAngle;
    verticalAngle = newVerticalAngle;
    setViewMatrices(createViewMatrices(newHorizontalAngle, newVerticalAngle));
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

  static std::shared_ptr<ConeLight> create(std::string lightId)
  {
    return std::make_shared<ConeLight>(lightId);
  };
};

#endif