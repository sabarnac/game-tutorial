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
  // The shadow buffer manager responsible for creating shader programs.
  ShadowBufferManager &shadowBufferManager;

  // The ID of the light.
  std::string lightId;
  // The name of the light.
  std::string lightName;

  // The color of the light.
  glm::vec3 lightColor;
  // The intensity of the light.
  double lightIntensity;

  // The position of the light.
  glm::vec3 position;
  // The closest distance the light can capture from.
  double nearPlane;
  // The farthest distance the light can capture till.
  double farPlane;

  // The list of view matrices of the light.
  std::vector<glm::mat4> viewMatrices;
  // The list of projection matrices of the light.
  std::vector<glm::mat4> projectionMatrices;
  // The shader program details of the light.
  std::shared_ptr<ShaderDetails> shaderDetails;
  // The shadow buffer details of the light.
  std::shared_ptr<ShadowBufferDetails> shadowBufferDetails;

protected:
  LightBase(
      std::string lightId,
      std::string lightName,
      glm::vec3 lightColor, double lightIntensity,
      std::string vertexShaderFilePath, std::string fragmentShaderFilePath,
      glm::vec3 position,
      double nearPlane, double farPlane,
      std::vector<glm::mat4> viewMatrices, std::vector<glm::mat4> projectionMatrices,
      ShadowBufferType shadowBufferType)
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
        projectionMatrices(projectionMatrices)
  {
    // Create the shader program for the light.
    shaderDetails = shaderManager.createShaderProgram(lightName + "::Shader", vertexShaderFilePath, fragmentShaderFilePath);
    // Create the shadow buffer for the light.
    shadowBufferDetails = shadowBufferManager.createShadowBuffer(lightName + "::ShadowMap", shadowBufferType);
  }

  LightBase(
      std::string lightId,
      std::string lightName,
      glm::vec3 lightColor, double lightIntensity,
      std::string vertexShaderFilePath, std::string geometryShaderFilePath, std::string fragmentShaderFilePath,
      glm::vec3 position,
      double nearPlane, double farPlane,
      std::vector<glm::mat4> viewMatrices, std::vector<glm::mat4> projectionMatrices,
      ShadowBufferType shadowBufferType)
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
        projectionMatrices(projectionMatrices)
  {
    // Create the shader program for the light.
    shaderDetails = shaderManager.createShaderProgram(lightName + "::Shader", vertexShaderFilePath, geometryShaderFilePath, fragmentShaderFilePath);
    // Create the shadow buffer for the light.
    shadowBufferDetails = shadowBufferManager.createShadowBuffer(lightName + "::ShadowMap", shadowBufferType);
  }

  LightBase(
      std::string lightId,
      std::string lightName,
      glm::vec3 lightColor, double lightIntensity,
      std::string vertexShaderFilePath, std::string fragmentShaderFilePath,
      glm::vec3 position,
      double nearPlane, double farPlane,
      std::vector<glm::mat4> viewMatrices, std::vector<glm::mat4> projectionMatrices,
      ShadowBufferType shadowBufferType, float outsideMapDepth[3])
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
        projectionMatrices(projectionMatrices)
  {
    // Create the shader program for the light.
    shaderDetails = shaderManager.createShaderProgram(lightName + "::Shader", vertexShaderFilePath, fragmentShaderFilePath);
    // Create the shadow buffer for the light.
    shadowBufferDetails = shadowBufferManager.createShadowBuffer(lightName + "::ShadowMap", shadowBufferType, outsideMapDepth);
  }

  LightBase(
      std::string lightId,
      std::string lightName,
      glm::vec3 lightColor, double lightIntensity,
      std::string vertexShaderFilePath, std::string geometryShaderFilePath, std::string fragmentShaderFilePath,
      glm::vec3 position,
      double nearPlane, double farPlane,
      std::vector<glm::mat4> viewMatrices, std::vector<glm::mat4> projectionMatrices,
      ShadowBufferType shadowBufferType, float outsideMapDepth[3])
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
        projectionMatrices(projectionMatrices)
  {
    // Create the shader program for the light.
    shaderDetails = shaderManager.createShaderProgram(lightName + "::Shader", vertexShaderFilePath, geometryShaderFilePath, fragmentShaderFilePath);
    // Create the shadow buffer for the light.
    shadowBufferDetails = shadowBufferManager.createShadowBuffer(lightName + "::ShadowMap", shadowBufferType, outsideMapDepth);
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
  std::string getLightId()
  {
    return lightId;
  }

  /**
   * Get the name of the light.
   * 
   * @return The light name.
   */
  std::string getLightName()
  {
    return lightName;
  }

  /**
   * Get the position of the light.
   * 
   * @return The light position.
   */
  glm::vec3 getLightPosition()
  {
    return position;
  }

  /**
   * Get the color of the light.
   * 
   * @return The light color.
   */
  glm::vec3 &getLightColor()
  {
    return lightColor;
  }

  /**
   * Get the intensity of the light.
   * 
   * @return The light intensity.
   */
  double getLightIntensity()
  {
    return lightIntensity;
  }

  /**
   * Get the near plane of the light.
   * 
   * @return The light near plane.
   */
  double getLightNearPlane()
  {
    return nearPlane;
  }

  /**
   * Get the far plane of the light.
   * 
   * @return The light far plane.
   */
  double getLightFarPlane()
  {
    return farPlane;
  }

  /**
   * Get the shader program details of the light.
   * 
   * @return The light shader program details.
   */
  std::shared_ptr<ShaderDetails> getShaderDetails()
  {
    return shaderDetails;
  }

  /**
   * Get the shadow buffer of the light.
   * 
   * @return The light shadow buffer.
   */
  std::shared_ptr<ShadowBufferDetails> getShadowBufferDetails()
  {
    return shadowBufferDetails;
  }

  /**
   * Get the view matrices of the light.
   * 
   * @return The light view matrices.
   */
  std::vector<glm::mat4> &getViewMatrices()
  {
    return viewMatrices;
  }

  /**
   * Get the projection matrices of the light.
   * 
   * @return The light projection matrices.
   */
  std::vector<glm::mat4> &getProjectionMatrices()
  {
    return projectionMatrices;
  }

  /**
   * Set the position of the light.
   * 
   * @param newPosition  The light position.
   */
  virtual void setLightPosition(glm::vec3 newPosition)
  {
    position = newPosition;
  }

  /**
   * Set the color of the light.
   * 
   * @param newLightColor  The light color.
   */
  virtual void setLightColor(glm::vec3 newLightColor)
  {
    lightColor = newLightColor;
  }

  /**
   * Set the intensity of the light.
   * 
   * @param newLightIntensity  The light intensity.
   */
  virtual void setLightIntensity(double newLightIntensity)
  {
    lightIntensity = newLightIntensity;
  }

  /**
   * Set the near plane of the light.
   * 
   * @param newNearPlane  The light near plane.
   */
  virtual void setLightNearPlane(double newNearPlane)
  {
    nearPlane = newNearPlane;
  }

  /**
   * Set the far plane of the light.
   * 
   * @param newFarPlane  The light far plane.
   */
  virtual void setLightFarPlane(double newFarPlane)
  {
    farPlane = newFarPlane;
  }

  /**
   * Set the view matrices of the light.
   * 
   * @param newViewMatrices  The light view matrices.
   */
  virtual void setViewMatrices(std::vector<glm::mat4> newViewMatrices)
  {
    viewMatrices = newViewMatrices;
  }

  /**
   * Set the projection matrices of the light.
   * 
   * @param newProjectionMatrices  The light projection matrices.
   */
  virtual void setProjectionMatrices(std::vector<glm::mat4> newProjectionMatrices)
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