#ifndef LIGHT_LIGHT_BASE_CPP
#define LIGHT_LIGHT_BASE_CPP

#include <iostream>
#include <vector>
#include <string>

#include <glm/glm.hpp>

#include "../include/shader.cpp"
#include "../include/shadowbuffer.cpp"

/**
 * Base class for creating lights.
 */
class LightBase
{

private:
  // The shader manager responsible for creating shader programs.
  ShaderManager &shaderManager;
  // The shadow buffer manager responsible for creating shadow buffers for lights.
  ShadowBufferManager &shadowBufferManager;

  // The ID of the light.
  const std::string lightId;
  // The name of the light.
  const std::string lightName;

  // The color of the light.
  glm::vec3 lightColor;
  // The intensity of the light.
  double_t lightIntensity;

  // The position of the light.
  glm::vec3 position;
  // The closest distance the light can capture from.
  double_t nearPlane;
  // The farthest distance the light can capture till.
  double_t farPlane;

  // The list of view matrices of the light.
  std::vector<glm::mat4> viewMatrices;
  // The list of projection matrices of the light.
  std::vector<glm::mat4> projectionMatrices;
  // The shader program details of the light.
  const std::shared_ptr<const ShaderDetails> shaderDetails;
  // The shadow buffer details of the light.
  const std::shared_ptr<const ShadowBufferDetails> shadowBufferDetails;

protected:
  LightBase(
      const std::string &lightId,
      const std::string &lightName,
      const glm::vec3 &lightColor, const double_t &lightIntensity,
      const std::string &vertexShaderFilePath, const std::string &fragmentShaderFilePath,
      const glm::vec3 &position,
      const double_t &nearPlane, const double_t &farPlane,
      const std::vector<glm::mat4> &viewMatrices, const std::vector<glm::mat4> &projectionMatrices,
      const ShadowBufferType &shadowBufferType)
      : shaderManager(ShaderManager::getInstance()),
        shadowBufferManager(ShadowBufferManager::getInstance()),
        lightId(lightId),
        lightName(lightName),
        lightColor(lightColor),
        lightIntensity(lightIntensity),
        position(position),
        nearPlane(nearPlane),
        farPlane(farPlane),
        viewMatrices(viewMatrices),
        projectionMatrices(projectionMatrices),
        shaderDetails(shaderManager.createShaderProgram(lightName + "::Shader", vertexShaderFilePath, fragmentShaderFilePath)),
        shadowBufferDetails(shadowBufferManager.createShadowBuffer(lightId + "::ShadowMap", shadowBufferType))
  {
  }

  LightBase(
      const std::string &lightId,
      const std::string &lightName,
      const glm::vec3 &lightColor, const double_t &lightIntensity,
      const std::string &vertexShaderFilePath, const std::string &geometryShaderFilePath, const std::string &fragmentShaderFilePath,
      const glm::vec3 &position,
      const double_t &nearPlane, const double_t &farPlane,
      const std::vector<glm::mat4> &viewMatrices, const std::vector<glm::mat4> &projectionMatrices,
      const ShadowBufferType &shadowBufferType)
      : shaderManager(ShaderManager::getInstance()),
        shadowBufferManager(ShadowBufferManager::getInstance()),
        lightId(lightId),
        lightName(lightName),
        lightColor(lightColor),
        lightIntensity(lightIntensity),
        position(position),
        nearPlane(nearPlane),
        farPlane(farPlane),
        viewMatrices(viewMatrices),
        projectionMatrices(projectionMatrices),
        shaderDetails(shaderManager.createShaderProgram(lightName + "::Shader", vertexShaderFilePath, geometryShaderFilePath, fragmentShaderFilePath)),
        shadowBufferDetails(shadowBufferManager.createShadowBuffer(lightId + "::ShadowMap", shadowBufferType))
  {
  }

  virtual ~LightBase()
  {
    // Destroy the shader program for the light.
    shaderManager.destroyShaderProgram(shaderDetails);
    // Destroy the shadow buffer for the light.
    shadowBufferManager.destroyShadowBuffer(shadowBufferDetails);
  }

public:
  /**
   * Get the ID of the light.
   * 
   * @return The light ID.
   */
  const std::string &getLightId() const
  {
    return lightId;
  }

  /**
   * Get the name of the light.
   * 
   * @return The light name.
   */
  const std::string &getLightName() const
  {
    return lightName;
  }

  /**
   * Get the position of the light.
   * 
   * @return The light position.
   */
  const glm::vec3 &getLightPosition() const
  {
    return position;
  }

  /**
   * Get the color of the light.
   * 
   * @return The light color.
   */
  const glm::vec3 &getLightColor() const
  {
    return lightColor;
  }

  /**
   * Get the intensity of the light.
   * 
   * @return The light intensity.
   */
  const double_t &getLightIntensity() const
  {
    return lightIntensity;
  }

  /**
   * Get the near plane of the light.
   * 
   * @return The light near plane.
   */
  const double_t &getLightNearPlane() const
  {
    return nearPlane;
  }

  /**
   * Get the far plane of the light.
   * 
   * @return The light far plane.
   */
  const double_t &getLightFarPlane() const
  {
    return farPlane;
  }

  /**
   * Get the shader program details of the light.
   * 
   * @return The light shader program details.
   */
  const std::shared_ptr<const ShaderDetails> &getShaderDetails() const
  {
    return shaderDetails;
  }

  /**
   * Get the shadow buffer of the light.
   * 
   * @return The light shadow buffer.
   */
  const std::shared_ptr<const ShadowBufferDetails> &getShadowBufferDetails() const
  {
    return shadowBufferDetails;
  }

  /**
   * Get the view matrices of the light.
   * 
   * @return The light view matrices.
   */
  const std::vector<glm::mat4> &getViewMatrices() const
  {
    return viewMatrices;
  }

  /**
   * Get the projection matrices of the light.
   * 
   * @return The light projection matrices.
   */
  const std::vector<glm::mat4> &getProjectionMatrices() const
  {
    return projectionMatrices;
  }

  /**
   * Set the position of the light.
   * 
   * @param newPosition  The light position.
   */
  virtual void setLightPosition(const glm::vec3 &newPosition)
  {
    position = newPosition;
  }

  /**
   * Set the color of the light.
   * 
   * @param newLightColor  The light color.
   */
  virtual void setLightColor(const glm::vec3 &newLightColor)
  {
    lightColor = newLightColor;
  }

  /**
   * Set the intensity of the light.
   * 
   * @param newLightIntensity  The light intensity.
   */
  virtual void setLightIntensity(const double_t &newLightIntensity)
  {
    lightIntensity = newLightIntensity;
  }

  /**
   * Set the near plane of the light.
   * 
   * @param newNearPlane  The light near plane.
   */
  virtual void setLightNearPlane(const double_t &newNearPlane)
  {
    nearPlane = newNearPlane;
  }

  /**
   * Set the far plane of the light.
   * 
   * @param newFarPlane  The light far plane.
   */
  virtual void setLightFarPlane(const double_t &newFarPlane)
  {
    farPlane = newFarPlane;
  }

  /**
   * Set the view matrices of the light.
   * 
   * @param newViewMatrices  The light view matrices.
   */
  virtual void setViewMatrices(const std::vector<glm::mat4> &newViewMatrices)
  {
    viewMatrices = newViewMatrices;
  }

  /**
   * Set the projection matrices of the light.
   * 
   * @param newProjectionMatrices  The light projection matrices.
   */
  virtual void setProjectionMatrices(const std::vector<glm::mat4> &newProjectionMatrices)
  {
    projectionMatrices = newProjectionMatrices;
  }

  /**
   * Initialize the light once registered.
   */
  virtual void init() {}

  /**
   * De-initialize the light once de-registered.
   */
  virtual void deinit() {}

  /**
   * Update the light during the update step before starting rendering.
   */
  virtual void update() {}
};

#endif