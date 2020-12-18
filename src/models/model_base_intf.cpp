#ifndef MODELS_MODEL_BASE_INTF_CPP
#define MODELS_MODEL_BASE_INTF_CPP

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
class ModelBaseIntf
{

protected:
  // The object manager responsible for creating objects.
  static ObjectManager &objectManager;
  // The texture manager responsible for creating textures.
  static TextureManager &textureManager;
  // The shader manager responsible for creating shader programs.
  static ShaderManager &shaderManager;

public:
  /**
   * Get the ID of the model.
   * 
   * @return The model ID.
   */
  virtual const std::string &getModelId() const = 0;

  /**
   * Get the name of the model.
   * 
   * @return The model name.
   */
  virtual std::string getModelName() const = 0;

  /**
   * Get the position of the model.
   * 
   * @return The model position.
   */
  virtual const glm::vec3 &getModelPosition() const = 0;

  /**
   * Get the rotation of the model.
   * 
   * @return The model rotation.
   */
  virtual const glm::vec3 &getModelRotation() const = 0;

  /**
   * Get the scale of the model.
   * 
   * @return The model scale.
   */
  virtual const glm::vec3 &getModelScale() const = 0;

  /**
   * Get the object details of the model.
   * 
   * @return The model object details.
   */
  virtual std::shared_ptr<const ObjectDetails> &getObjectDetails() = 0;

  /**
   * Get the texture details of the model.
   * 
   * @return The model texture details.
   */
  virtual std::shared_ptr<const TextureDetails> &getTextureDetails() = 0;

  /**
   * Get the shader program details of the model.
   * 
   * @return The model shader program details.
   */
  virtual std::shared_ptr<const ShaderDetails> &getShaderDetails() = 0;

  /**
   * Get the collider details of the model.
   * 
   * @return The model collider details.
   */
  virtual const std::shared_ptr<ColliderDetails> &getColliderDetails() const = 0;

  /**
   * Get the model matrix of the model.
   * 
   * @return The model matrix.
   */
  virtual const glm::mat4 &getModelMatrix() const = 0;

  /**
   * Set the position of the model.
   * 
   * @param newPosition  The model position.
   */
  virtual void setModelPosition(const glm::vec3 &newPosition) = 0;

  /**
   * Set the rotation of the model.
   * 
   * @param newRotation  The model rotation.
   */
  virtual void setModelRotation(const glm::vec3 &newRotation) = 0;

  /**
   * Set the scale of the model.
   * 
   * @param newScale  The model scale.
   */
  virtual void setModelScale(const glm::vec3 &newScale) = 0;

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

ObjectManager &ModelBaseIntf::objectManager = ObjectManager::getInstance();
TextureManager &ModelBaseIntf::textureManager = TextureManager::getInstance();
ShaderManager &ModelBaseIntf::shaderManager = ShaderManager::getInstance();

#endif