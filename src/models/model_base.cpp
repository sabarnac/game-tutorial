#ifndef MODELS_MODEL_BASE_CPP
#define MODELS_MODEL_BASE_CPP

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <algorithm>
#include <iterator>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

#include "../include/object.cpp"
#include "../include/texture.cpp"
#include "../include/shader.cpp"
#include "../include/collider.cpp"

class ModelBase
{

private:
  ObjectManager &objectManager;
  TextureManager &textureManager;
  ShaderManager &shaderManager;

  std::string modelId;
  std::string modelName;

  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale;

  glm::mat4 modelMatrix;
  std::shared_ptr<ObjectDetails> objectDetails;
  std::shared_ptr<TextureDetails> textureDetails;
  std::shared_ptr<ShaderDetails> shaderDetails;
  std::shared_ptr<ColliderDetails> colliderDetails;

  glm::mat4 createModelMatrix()
  {
    return glm::translate(position) * glm::toMat4(glm::quat(rotation)) * glm::scale(scale) * glm::mat4();
  }

protected:
  ModelBase(
      std::string modelId,
      std::string modelName,
      glm::vec3 position, glm::vec3 rotation, glm::vec3 scale,
      std::string modelObjectFilePath,
      std::string modelTextureFilePath, TextureType modelTextureType,
      std::string modelVertexShaderFilePath, std::string modelFragmentShaderFilePath,
      std::shared_ptr<ColliderShape> colliderShape)
      : objectManager(ObjectManager::getInstance()),
        textureManager(TextureManager::getInstance()),
        shaderManager(ShaderManager::getInstance()),
        modelId(modelId),
        modelName(modelName),
        position(position),
        rotation(rotation),
        scale(scale),
        modelMatrix(createModelMatrix())
  {
    objectDetails = objectManager.createObject(modelName + "::Object", modelObjectFilePath);
    textureDetails = textureManager.createTexture(modelName + "::Texture::BMP", modelTextureFilePath, modelTextureType);
    shaderDetails = shaderManager.createShaderProgram(modelName + "::Shader", modelVertexShaderFilePath, modelFragmentShaderFilePath);
    colliderDetails = std::make_shared<ColliderDetails>(modelName + "::Collider", colliderShape);
  }

  ModelBase(
      std::string modelId,
      std::string modelName,
      glm::vec3 position, glm::vec3 rotation, glm::vec3 scale,
      std::string modelObjectFilePath,
      std::string modelTextureFilePath, TextureType modelTextureType,
      std::string modelVertexShaderFilePath, std::string modelFragmentShaderFilePath,
      ColliderShapeType colliderShapeType)
      : objectManager(ObjectManager::getInstance()),
        textureManager(TextureManager::getInstance()),
        shaderManager(ShaderManager::getInstance()),
        modelId(modelId),
        modelName(modelName),
        position(position),
        rotation(rotation),
        scale(scale),
        modelMatrix(createModelMatrix())
  {
    objectDetails = objectManager.createObject(modelName + "::Object", modelObjectFilePath);
    textureDetails = textureManager.createTexture(modelName + "::Texture::BMP", modelTextureFilePath, modelTextureType);
    shaderDetails = shaderManager.createShaderProgram(modelName + "::Shader", modelVertexShaderFilePath, modelFragmentShaderFilePath);

    std::shared_ptr<ColliderShape> newColliderShape;
    auto modelVertices = objectDetails->getVertices();
    switch (colliderShapeType)
    {
    case BOX:
      newColliderShape = std::make_shared<BoxColliderShape>(position, rotation, scale, modelVertices);
      break;
    default:
      newColliderShape = std::make_shared<SphereColliderShape>(position, rotation, scale, modelVertices);
    }
    colliderDetails = std::make_shared<ColliderDetails>(modelName + "::Collider", newColliderShape);
  }

  virtual ~ModelBase()
  {
    objectManager.destroyObject(objectDetails);
    textureManager.destroyTexture(textureDetails);
    shaderManager.destroyShaderProgram(shaderDetails);
  }

public:
  std::string getModelName()
  {
    return modelName;
  }

  std::string getModelId()
  {
    return modelId;
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
    position = newPosition;
    colliderDetails->getColliderShape()->updateTransformations(newPosition, rotation, scale);
    modelMatrix = createModelMatrix();
  }

  void setModelRotation(glm::vec3 newRotation)
  {
    rotation = newRotation;
    colliderDetails->getColliderShape()->updateTransformations(position, newRotation, scale);
    modelMatrix = createModelMatrix();
  }

  void setModelScale(glm::vec3 newScale)
  {
    scale = newScale;
    colliderDetails->getColliderShape()->updateTransformations(position, rotation, newScale);
    modelMatrix = createModelMatrix();
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

  std::shared_ptr<ColliderDetails> &getColliderDetails()
  {
    return colliderDetails;
  }

  virtual void init(){};

  virtual void deinit(){};

  virtual void update(){};
};

#endif