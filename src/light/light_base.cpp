#ifndef LIGHT_LIGHT_BASE_CPP
#define LIGHT_LIGHT_BASE_CPP

#include <iostream>
#include <vector>
#include <string>

#include <glm/glm.hpp>

#include "../include/shader.cpp"
#include "../include/shadowbuffer.cpp"

class LightBase
{

private:
  ShaderManager &shaderManager;
  ShadowBufferManager &shadowBufferManager;

  std::string lightId;
  std::string lightName;

  glm::vec3 lightColor;
  double lightIntensity;

  glm::vec3 position;
  double nearPlane;
  double farPlane;

  std::vector<glm::mat4> viewMatrices;
  std::vector<glm::mat4> projectionMatrices;
  std::shared_ptr<ShaderDetails> shaderDetails;
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
    shaderDetails = shaderManager.createShaderProgram(lightName + "::Shader", vertexShaderFilePath, fragmentShaderFilePath);
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
    shaderDetails = shaderManager.createShaderProgram(lightName + "::Shader", vertexShaderFilePath, geometryShaderFilePath, fragmentShaderFilePath);
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
    shaderDetails = shaderManager.createShaderProgram(lightName + "::Shader", vertexShaderFilePath, fragmentShaderFilePath);
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
    shaderDetails = shaderManager.createShaderProgram(lightName + "::Shader", vertexShaderFilePath, geometryShaderFilePath, fragmentShaderFilePath);
    shadowBufferDetails = shadowBufferManager.createShadowBuffer(lightName + "::ShadowMap", shadowBufferType, outsideMapDepth);
  }

  virtual ~LightBase() {}

public:
  std::string getLightId()
  {
    return lightId;
  }

  std::string getLightName()
  {
    return lightName;
  }

  glm::vec3 getLightPosition()
  {
    return position;
  }

  glm::vec3 &getLightColor()
  {
    return lightColor;
  }

  double getLightIntensity()
  {
    return lightIntensity;
  }

  double getLightNearPlane()
  {
    return nearPlane;
  }

  double getLightFarPlane()
  {
    return farPlane;
  }

  std::vector<glm::mat4> &getViewMatrices()
  {
    return viewMatrices;
  }

  std::vector<glm::mat4> &getProjectionMatrices()
  {
    return projectionMatrices;
  }

  virtual void setLightPosition(glm::vec3 newPosition)
  {
    position = newPosition;
  }

  virtual void setLightColor(glm::vec3 newLightColor)
  {
    lightColor = newLightColor;
  }

  virtual void setLightIntensity(double newLightIntensity)
  {
    lightIntensity = newLightIntensity;
  }

  virtual void setLightNearPlane(double newNearPlane)
  {
    nearPlane = newNearPlane;
  }

  virtual void setLightFarPlane(double newFarPlane)
  {
    farPlane = newFarPlane;
  }

  virtual void setViewMatrices(std::vector<glm::mat4> newViewMatrices)
  {
    viewMatrices = newViewMatrices;
  }

  virtual void setProjectionMatrices(std::vector<glm::mat4> newProjectionMatrices)
  {
    projectionMatrices = newProjectionMatrices;
  }

  double getNearPlane()
  {
    return nearPlane;
  }

  double getFarPlane()
  {
    return farPlane;
  }

  std::shared_ptr<ShaderDetails> getShaderDetails()
  {
    return shaderDetails;
  }

  std::shared_ptr<ShadowBufferDetails> getShadowBufferDetails()
  {
    return shadowBufferDetails;
  }

  virtual void init() {}

  virtual void deinit() {}

  virtual void update() {}
};

#endif