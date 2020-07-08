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

#include "../include/object.cpp"
#include "../include/texture.cpp"
#include "../include/shader.cpp"
#include "../include/collider.cpp"

/**
 * Base class for creating models.
 */
class ModelBase
{

private:
  // The object manager responsible for creating objects.
  ObjectManager &objectManager;
  // The texture manager responsible for creating textures.
  TextureManager &textureManager;
  // The shader manager responsible for creating shader programs.
  ShaderManager &shaderManager;

  // The ID of the model.
  std::string modelId;
  // The name of the model.
  std::string modelName;

  // The position of the model.
  glm::vec3 position;
  // The rotation of the model.
  glm::vec3 rotation;
  // The scale of the model.
  glm::vec3 scale;

  // The model matrix of the model.
  glm::mat4 modelMatrix;
  // The object details of the model.
  std::shared_ptr<ObjectDetails> objectDetails;
  // The texture details of the model.
  std::shared_ptr<TextureDetails> textureDetails;
  // The shader program details of the model.
  std::shared_ptr<ShaderDetails> shaderDetails;
  // The collider details of the model.
  std::shared_ptr<ColliderDetails> colliderDetails;

  /**
   * Create the model matrix of the madel.
   */
  glm::mat4 createModelMatrix()
  {
    // Calculate and return the model matrix using the position, rotation, and scaling values of the model.
    return glm::translate(position) * glm::toMat4(glm::quat(rotation)) * glm::scale(scale);
  }

protected:
  ModelBase(
      std::string modelId,
      std::string modelName,
      glm::vec3 position, glm::vec3 rotation, glm::vec3 scale,
      std::string modelObjectFilePath,
      std::string modelTextureFilePath,
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
    // Create the object for the model.
    objectDetails = objectManager.createObject(modelName + "::Object", modelObjectFilePath);
    // Create the texture for the model.
    textureDetails = textureManager.create2dTexture(modelName + "::Texture", modelTextureFilePath);
    // Create the shader program for the model.
    shaderDetails = shaderManager.createShaderProgram(modelName + "::Shader", modelVertexShaderFilePath, modelFragmentShaderFilePath);
    // Create the collider for the model.
    colliderDetails = std::make_shared<ColliderDetails>(modelName + "::Collider", colliderShape);
  }

  ModelBase(
      std::string modelId,
      std::string modelName,
      glm::vec3 position, glm::vec3 rotation, glm::vec3 scale,
      std::string modelObjectFilePath,
      std::string modelTextureFilePath,
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
    // Create the object for the model.
    objectDetails = objectManager.createObject(modelName + "::Object", modelObjectFilePath);
    // Create the texture for the model.
    textureDetails = textureManager.create2dTexture(modelName + "::Texture", modelTextureFilePath);
    // Create the shader program for the model.
    shaderDetails = shaderManager.createShaderProgram(modelName + "::Shader", modelVertexShaderFilePath, modelFragmentShaderFilePath);

    // Get the vertices of the model.
    auto modelVertices = objectDetails->getVertices();
    // Check what collider shape is required,
    switch (colliderShapeType)
    {
    case BOX:
      // Create a box collider for the model.
      colliderDetails = std::make_shared<ColliderDetails>(modelName + "::Collider", std::make_shared<BoxColliderShape>(position, rotation, scale, modelVertices));
      break;
    default:
      // Create a sphere collider for the model.
      colliderDetails = std::make_shared<ColliderDetails>(modelName + "::Collider", std::make_shared<SphereColliderShape>(position, rotation, scale, modelVertices));
    }
  }

  virtual ~ModelBase()
  {
    // Destroy the object for the model.
    objectManager.destroyObject(objectDetails);
    // Destroy the texture for the model.
    textureManager.destroyTexture(textureDetails);
    // Destroy the shader program for the model.
    shaderManager.destroyShaderProgram(shaderDetails);
  }

public:
  /**
   * Get the ID of the model.
   * 
   * @return The model ID.
   */
  std::string getModelId()
  {
    return modelId;
  }

  /**
   * Get the name of the model.
   * 
   * @return The model name.
   */
  std::string getModelName()
  {
    return modelName;
  }

  /**
   * Get the position of the model.
   * 
   * @return The model position.
   */
  glm::vec3 &getModelPosition()
  {
    return position;
  }

  /**
   * Get the rotation of the model.
   * 
   * @return The model rotation.
   */
  glm::vec3 &getModelRotation()
  {
    return rotation;
  }

  /**
   * Get the scale of the model.
   * 
   * @return The model scale.
   */
  glm::vec3 &getModelScale()
  {
    return scale;
  }

  /**
   * Get the object details of the model.
   * 
   * @return The model object details.
   */
  std::shared_ptr<ObjectDetails> &getObjectDetails()
  {
    return objectDetails;
  }

  /**
   * Get the texture details of the model.
   * 
   * @return The model texture details.
   */
  std::shared_ptr<TextureDetails> &getTextureDetails()
  {
    return textureDetails;
  }

  /**
   * Get the shader program details of the model.
   * 
   * @return The model shader program details.
   */
  std::shared_ptr<ShaderDetails> &getShaderDetails()
  {
    return shaderDetails;
  }

  /**
   * Get the collider details of the model.
   * 
   * @return The model collider details.
   */
  std::shared_ptr<ColliderDetails> &getColliderDetails()
  {
    return colliderDetails;
  }

  /**
   * Get the model matrix of the model.
   * 
   * @return The model matrix.
   */
  glm::mat4 &getModelMatrix()
  {
    return modelMatrix;
  }

  /**
   * Set the position of the model.
   * 
   * @param newPosition  The model position.
   */
  void setModelPosition(glm::vec3 newPosition)
  {
    // Set the new position.
    position = newPosition;
    // Update the collider with the new transformation details
    colliderDetails->getColliderShape()->updateTransformations(newPosition, rotation, scale);
    // Update the model matrix.
    modelMatrix = createModelMatrix();
  }

  /**
   * Set the rotation of the model.
   * 
   * @param newRotation  The model rotation.
   */
  void setModelRotation(glm::vec3 newRotation)
  {
    // Set the new rotation.
    rotation = newRotation;
    // Update the collider with the new transformation details
    colliderDetails->getColliderShape()->updateTransformations(position, newRotation, scale);
    // Update the model matrix.
    modelMatrix = createModelMatrix();
  }

  /**
   * Set the scale of the model.
   * 
   * @param newScale  The model scale.
   */
  void setModelScale(glm::vec3 newScale)
  {
    // Set the new scale.
    scale = newScale;
    // Update the collider with the new transformation details
    colliderDetails->getColliderShape()->updateTransformations(position, rotation, newScale);
    // Update the model matrix.
    modelMatrix = createModelMatrix();
  }

  /**
   * Initialize the model once registered.
   */
  virtual void init() {}

  /**
   * De-initialize the model once de-registered.
   */
  virtual void deinit() {}

  /**
   * Update the model during the update step before starting rendering.
   */
  virtual void update() {}
};

#endif