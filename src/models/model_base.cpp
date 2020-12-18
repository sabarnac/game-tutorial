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

#include "model_base_intf.cpp"

/**
 * Base class for creating models.
 */
template <typename T>
class ModelBase : public ModelBaseIntf
{

private:
  // The name of the model.
  inline static std::string modelName;

  // The object details of the model.
  inline static std::shared_ptr<const ObjectDetails> objectDetails;
  // The texture details of the model.
  inline static std::shared_ptr<const TextureDetails> textureDetails;
  // The shader program details of the model.
  inline static std::shared_ptr<const ShaderDetails> shaderDetails;

  // The ID of the model.
  const std::string modelId;

  // The position of the model.
  glm::vec3 position;
  // The rotation of the model.
  glm::vec3 rotation;
  // The scale of the model.
  glm::vec3 scale;

  // The model matrix of the model.
  glm::mat4 modelMatrix;

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

  const std::shared_ptr<ColliderDetails> createColliderDetails(const ColliderShapeType &colliderShapeType)
  {
    // Get the vertices of the model.
    auto modelVertices = objectDetails->getVertices();
    // Check what collider shape is required,
    switch (colliderShapeType)
    {
    case BOX:
      // Create a box collider for the model.
      return std::make_shared<ColliderDetails>(modelName + "::Collider", std::make_shared<BoxColliderShape>(position, rotation, scale, modelVertices));
      break;
    default:
      // Create a sphere collider for the model.
      return std::make_shared<ColliderDetails>(modelName + "::Collider", std::make_shared<SphereColliderShape>(position, rotation, scale, modelVertices));
    }
  }

protected:
  ModelBase(
      const std::string &modelId,
      const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale,
      const std::shared_ptr<ColliderShape> &colliderShape)
      : modelId(modelId),
        position(position),
        rotation(rotation),
        scale(scale),
        modelMatrix(createModelMatrix()),
        colliderDetails(std::make_shared<ColliderDetails>(modelName + "::Collider", colliderShape))
  {
  }

  ModelBase(
      const std::string &modelId,
      const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale,
      const ColliderShapeType &colliderShapeType)
      : modelId(modelId),
        position(position),
        rotation(rotation),
        scale(scale),
        modelMatrix(createModelMatrix()),
        colliderDetails(createColliderDetails(colliderShapeType))
  {
  }

  /**
   * Initialize the base model dependencies
   */
  static void initModelDeps(
      const std::string &modelName,
      const std::string &modelObjectFilePath,
      const std::string &modelTextureFilePath,
      const std::string &modelVertexShaderFilePath, const std::string &modelFragmentShaderFilePath)
  {
    objectDetails = objectManager.createObject(modelName + "::Object", modelObjectFilePath);
    textureDetails = textureManager.create2dTexture(modelName + "::Texture", modelTextureFilePath);
    shaderDetails = shaderManager.createShaderProgram(modelName + "::Shader", modelVertexShaderFilePath, modelFragmentShaderFilePath);

    ModelBase::modelName = modelName;
  }

  /**
   * De-initialize the model dependencies
   */
  static void deinitModelDeps()
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
  const std::string &getModelId() const
  {
    return modelId;
  }

  /**
   * Get the name of the model.
   * 
   * @return The model name.
   */
  std::string getModelName() const
  {
    return modelName;
  }

  /**
   * Get the position of the model.
   * 
   * @return The model position.
   */
  const glm::vec3 &getModelPosition() const
  {
    return position;
  }

  /**
   * Get the rotation of the model.
   * 
   * @return The model rotation.
   */
  const glm::vec3 &getModelRotation() const
  {
    return rotation;
  }

  /**
   * Get the scale of the model.
   * 
   * @return The model scale.
   */
  const glm::vec3 &getModelScale() const
  {
    return scale;
  }

  /**
   * Get the object details of the model.
   * 
   * @return The model object details.
   */
  std::shared_ptr<const ObjectDetails> &getObjectDetails()
  {
    return objectDetails;
  }

  /**
   * Get the texture details of the model.
   * 
   * @return The model texture details.
   */
  std::shared_ptr<const TextureDetails> &getTextureDetails()
  {
    return textureDetails;
  }

  /**
   * Get the shader program details of the model.
   * 
   * @return The model shader program details.
   */
  std::shared_ptr<const ShaderDetails> &getShaderDetails()
  {
    return shaderDetails;
  }

  /**
   * Get the collider details of the model.
   * 
   * @return The model collider details.
   */
  const std::shared_ptr<ColliderDetails> &getColliderDetails() const
  {
    return colliderDetails;
  }

  /**
   * Get the model matrix of the model.
   * 
   * @return The model matrix.
   */
  const glm::mat4 &getModelMatrix() const
  {
    return modelMatrix;
  }

  /**
   * Set the position of the model.
   * 
   * @param newPosition  The model position.
   */
  void setModelPosition(const glm::vec3 &newPosition)
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
  void setModelRotation(const glm::vec3 &newRotation)
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
  void setModelScale(const glm::vec3 &newScale)
  {
    // Set the new scale.
    scale = newScale;
    // Update the collider with the new transformation details
    colliderDetails->getColliderShape()->updateTransformations(position, rotation, newScale);
    // Update the model matrix.
    modelMatrix = createModelMatrix();
  }
};

template <typename T>
static std::string modelName;

template <typename T>
static std::shared_ptr<const ObjectDetails> objectDetails;
template <typename T>
static std::shared_ptr<const TextureDetails> textureDetails;
template <typename T>
static std::shared_ptr<const ShaderDetails> shaderDetails;

#endif