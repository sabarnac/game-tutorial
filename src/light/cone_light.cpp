#ifndef LIGHT_CONE_LIGHT_CPP
#define LIGHT_CONE_LIGHT_CPP

#include <iostream>
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "light_base.cpp"

/**
 * Class that represents a cone light.
 */
class ConeLight : public LightBase
{
private:
  float_t horizontalAngle;
  float_t verticalAngle;

  /**
   * Create the view matrices for the cone light.
   * 
   * @param horizontalAngle  The horizontal angle of the light.
   * @param verticalAngle  The vertical angle of the light.
   * 
   * @return The list of view matrices for the cone light.
   */
  std::vector<glm::mat4> createViewMatrices(const float_t &horizontalAngle, const float_t &verticalAngle)
  {
    // Calculate the direction of the light.
    const auto direction = glm::vec3(
        cos(verticalAngle) * sin(horizontalAngle),
        sin(verticalAngle),
        cos(verticalAngle) * cos(horizontalAngle));

    // Calculate the right vector of the light.
    const auto right = glm::vec3(
        sin(horizontalAngle - glm::pi<float_t>() / 2.0f),
        0,
        cos(horizontalAngle - glm::pi<float_t>() / 2.0f));

    // Calculate the up vector of the light.
    const auto up = glm::cross(right, direction);

    // Get the position of the light.
    const auto position = getLightPosition();

    // Calculate and return the view matrices of the light.
    return std::vector<glm::mat4>({glm::lookAt(position, position + direction, up)});
  }

  /**
   * Create the projection matrices for the cone light.
   * 
   * @param nearPlane  The closest distance the light can project from.
   * @param farPlane   The farthest distance the light can project till.
   * 
   * @return The list of projection matrices for the cone light.
   */
  std::vector<glm::mat4> createProjectionMatrices(const float_t &nearPlane, const float_t &farPlane)
  {
    return std::vector<glm::mat4>({glm::perspective(glm::radians(90.0f), 1.0f, nearPlane, farPlane)});
  }

public:
  ConeLight(const std::string &lightId)
      : LightBase(
            lightId,
            "Cone",
            glm::vec3(1.0f), 100.0f,
            "assets/shaders/vertex/light_base.glsl", "assets/shaders/geometry/cone_light.glsl", "assets/shaders/fragment/cone_light.glsl",
            glm::vec3(0.0f),
            0.1f, 100.0f,
            createViewMatrices(0.0f, 0.0f), createProjectionMatrices(0.1f, 100.0f),
            ShadowBufferType::CONE),
        horizontalAngle(0.0f),
        verticalAngle(0.0f) {}

  virtual ~ConeLight() {}

  void setLightPosition(const glm::vec3 &newPosition) override
  {
    // Update the light position.
    LightBase::setLightPosition(newPosition);
    // Update the view matrices.
    setViewMatrices(createViewMatrices(horizontalAngle, verticalAngle));
  }

  void setLightAngles(const float_t &newHorizontalAngle, const float_t &newVerticalAngle)
  {
    // Update the light angles.
    horizontalAngle = newHorizontalAngle;
    verticalAngle = newVerticalAngle;
    // Update the view matrices.
    setViewMatrices(createViewMatrices(newHorizontalAngle, newVerticalAngle));
  }

  void setLightNearPlane(const float_t &newNearPlane) override
  {
    // Update the light near plane.
    LightBase::setLightNearPlane(newNearPlane);
    // Update the projection matrices.
    setProjectionMatrices(createProjectionMatrices(newNearPlane, getLightFarPlane()));
  }

  void setLightFarPlane(const float_t &newFarPlane) override
  {
    // Update the light far plane.
    LightBase::setLightFarPlane(newFarPlane);
    // Update the projection matrices.
    setProjectionMatrices(createProjectionMatrices(getLightNearPlane(), newFarPlane));
  }

  /**
   * Creates a new instance of the cone light.
   */
  const static std::shared_ptr<ConeLight> create(const std::string &lightId)
  {
    return std::make_shared<ConeLight>(lightId);
  }
};

#endif