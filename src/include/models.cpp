#ifndef INCLUDE_MODELS_CPP
#define INCLUDE_MODELS_CPP

#include <string>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

#include "objloader.cpp"
#include "texture.cpp"
#include "shader.cpp"

class ModelBase
{

private:
  ObjectManager &objectManager;
  TextureManager &textureManager;
  ShaderManager &shaderManager;

  std::string modelName;

  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale;
  glm::mat4 modelMatrix;

  std::shared_ptr<ObjectDetails> objectDetails;
  std::shared_ptr<TextureDetails> textureDetails;
  std::shared_ptr<ShaderDetails> shaderDetails;

  glm::mat4 updateModelMatrix(glm::vec3 &position, glm::vec3 &rotation, glm::vec3 &scale)
  {
    std::cout << "[ModelBase] "
              << "Updating model matrix." << std::endl
              << "[ModelBase] "
              << "\tPosition: " << glm::to_string(position)
              << "[ModelBase] "
              << "\tRotation: " << glm::to_string(rotation)
              << "[ModelBase] "
              << "\tScale: " << glm::to_string(scale) << std::endl;
    return glm::translate(position) * glm::toMat4(glm::quat(rotation)) * glm::scale(scale) * glm::mat4();
  }

protected:
  ModelBase(
      std::string modelName,
      glm::vec3 position,
      glm::vec3 rotation,
      glm::vec3 scale,
      std::string modelObjectFilePath,
      std::string modelTextureFilePath,
      TextureType modelTextureType,
      std::string modelVertexShaderFilePath,
      std::string modelFragmentShaderFilePath)
      : objectManager(ObjectManager::getInstance()),
        textureManager(TextureManager::getInstance()),
        shaderManager(ShaderManager::getInstance()),
        modelName(modelName),
        position(position),
        rotation(rotation),
        scale(scale),
        modelMatrix(updateModelMatrix(position, rotation, scale))
  {
    std::cout << "[ModelBase] "
              << "Constructing ModelBase." << std::endl;
    objectDetails = objectManager.createObject(modelName + "::Object", modelObjectFilePath);
    textureDetails = textureManager.createTexture(modelName + "::Texture::BMP", modelTextureFilePath, modelTextureType);
    shaderDetails = shaderManager.createShaderProgram(modelName + "::Shader", modelVertexShaderFilePath, modelFragmentShaderFilePath);
    std::cout << "[ModelBase] "
              << "Constructed ModelBase." << std::endl;
  }

  ~ModelBase()
  {
    std::cout << "[ModelBase] "
              << "Destroying ModelBase." << std::endl;
    objectManager.destroyObject(objectDetails);
    textureManager.destroyTexture(textureDetails);
    shaderManager.destroyShaderProgram(shaderDetails);
    std::cout << "[ModelBase] "
              << "Destroyed ModelBase." << std::endl;
  }

public:
  std::string getModelName()
  {
    return modelName;
  }

  glm::vec3 &getModelPosition()
  {
    return position;
  }

  glm::vec3 &getModelRotation()
  {
    return rotation;
  }

  glm::vec3 &getModelScale()
  {
    return scale;
  }

  glm::mat4 &getModelMatrix()
  {
    return modelMatrix;
  }

  void setModelPosition(glm::vec3 newPosition)
  {
    std::cout << "[ModelBase] "
              << "Set position: " << glm::to_string(newPosition) << std::endl;
    position = newPosition;
    modelMatrix = updateModelMatrix(newPosition, rotation, scale);
  }

  void setModelRotation(glm::vec3 newRotation)
  {
    std::cout << "[ModelBase] "
              << "Set rotation: " << glm::to_string(newRotation) << std::endl;
    rotation = newRotation;
    modelMatrix = updateModelMatrix(position, newRotation, scale);
  }

  void setModelScale(glm::vec3 newScale)
  {
    std::cout << "[ModelBase] "
              << "Set scale: " << glm::to_string(newScale) << std::endl;
    scale = newScale;
    modelMatrix = updateModelMatrix(position, rotation, newScale);
  }

  std::shared_ptr<ObjectDetails> &getObjectDetails()
  {
    return objectDetails;
  }

  std::shared_ptr<TextureDetails> &getTextureDetails()
  {
    return textureDetails;
  }

  std::shared_ptr<ShaderDetails> &getShaderDetails()
  {
    return shaderDetails;
  }

  virtual void update() = 0;
};

#endif