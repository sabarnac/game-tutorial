#ifndef LIGHT_POINT_LIGHT_CPP
#define LIGHT_POINT_LIGHT_CPP

#include <iostream>
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "light_base.cpp"

/**
 * Class that represents a point light.
 */
class PointLight : public LightBase
{
private:
  /**
   * Create the view matrices for the point light.
   * 
   * @return The list of view matrices for the point light.
   */
  std::vector<glm::mat4> createViewMatrices()
  {
    const auto position = getLightPosition();
    return std::vector<glm::mat4>({glm::lookAt(position, position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)),
                                   glm::lookAt(position, position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)),
                                   glm::lookAt(position, position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)),
                                   glm::lookAt(position, position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)),
                                   glm::lookAt(position, position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)),
                                   glm::lookAt(position, position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0))});
  }

  /**
   * Create the projection matrices for the point light.
   * 
   * @param nearPlane  The closest distance the light can project from.
   * @param farPlane   The farthest distance the light can project till.
   * 
   * @return The list of projection matrices for the point light.
   */
  std::vector<glm::mat4> createProjectionMatrices(const double_t &nearPlane, const double_t &farPlane)
  {
    return std::vector<glm::mat4>({glm::perspective(glm::radians(90.0), 1.0, nearPlane, farPlane),
                                   glm::perspective(glm::radians(90.0), 1.0, nearPlane, farPlane),
                                   glm::perspective(glm::radians(90.0), 1.0, nearPlane, farPlane),
                                   glm::perspective(glm::radians(90.0), 1.0, nearPlane, farPlane),
                                   glm::perspective(glm::radians(90.0), 1.0, nearPlane, farPlane),
                                   glm::perspective(glm::radians(90.0), 1.0, nearPlane, farPlane)});
  }

public:
  PointLight(const std::string &lightId)
      : LightBase(
            lightId,
            "PointLight",
            glm::vec3(1.0), 100.0,
            "assets/shaders/vertex/light_base.glsl", "assets/shaders/geometry/light_base.glsl", "assets/shaders/fragment/point_light.glsl",
            glm::vec3(0.0),
            0.1, 100.0,
            createViewMatrices(), createProjectionMatrices(0.1, 100.0),
            ShadowBufferType::POINT) {}

  virtual ~PointLight() {}

  void setLightPosition(const glm::vec3 &newPosition) override
  {
    // Update the light position.
    LightBase::setLightPosition(newPosition);
    // Update the view matrices.
    setViewMatrices(createViewMatrices());
  }

  void setLightNearPlane(const double_t &newNearPlane) override
  {
    // Update the light near plane.
    LightBase::setLightNearPlane(newNearPlane);
    // Update the projection matrices.
    setProjectionMatrices(createProjectionMatrices(newNearPlane, getLightFarPlane()));
  }

  void setLightFarPlane(const double_t &newFarPlane) override
  {
    // Update the light far plane.
    LightBase::setLightFarPlane(newFarPlane);
    // Update the projection matrices.
    setProjectionMatrices(createProjectionMatrices(getLightNearPlane(), newFarPlane));
  }

  /**
   * Creates a new instance of the point light.
   */
  const static std::shared_ptr<PointLight> create(const std::string &lightId)
  {
    return std::make_shared<PointLight>(lightId);
  };
};

#endif