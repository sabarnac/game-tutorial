#ifndef INCLUDE_SHADOWBUFFER_CPP
#define INCLUDE_SHADOWBUFFER_CPP

#include <string>
#include <map>
#include <set>
#include <memory>
#include <limits>

#include <GL/glew.h>

#include "constants.cpp"
#include "window.cpp"

/**
 * Enum of supported shadow buffer types.
 */
enum ShadowBufferType
{
  CONE,
  POINT
};

/**
 * Class for containing the details of the shadow buffer.
 */
class ShadowBufferDetails
{
  // Let the shadow buffer manager access private variables.
  friend class ShadowBufferManager;

private:
  // The ID of the shadow buffer.
  const GLuint shadowBufferId;
  // The ID of the texture array the shadow buffer copies data to in a layer.
  const GLuint shadowBufferTextureArrayId;
  // The ID of the layer of the texture array that the shadow buffer data is stored in.
  const uint32_t shadowBufferTextureArrayLayerId;
  // The type of the shadow buffer.
  const ShadowBufferType shadowBufferType;

  // The name of the shadow buffer.
  const std::string shadowBufferName;

public:
  ShadowBufferDetails(
      const GLuint &shadowBufferId,
      const GLuint &shadowBufferTextureArrayId,
      const uint32_t &shadowBufferTextureArrayLayerId,
      const std::string &shadowBufferName,
      const ShadowBufferType &shadowBufferType)
      : shadowBufferId(shadowBufferId),
        shadowBufferTextureArrayId(shadowBufferTextureArrayId),
        shadowBufferTextureArrayLayerId(shadowBufferTextureArrayLayerId),
        shadowBufferType(shadowBufferType),
        shadowBufferName(shadowBufferName) {}

  /**
   * Get the ID of the shadow buffer.
   * 
   * @return The shadow buffer ID.
   */
  const GLuint &getShadowBufferId() const
  {
    return shadowBufferId;
  }

  /**
   * Get the ID of the texture array the shadow buffer copies data to in a layer.
   * 
   * @return The shadow buffer texture array ID.
   */
  const GLuint &getShadowBufferTextureArrayId() const
  {
    return shadowBufferTextureArrayId;
  }

  /**
   * Get the ID of the layer of the texture array that the shadow buffer data is stored in.
   * 
   * @return The layer ID.
   */
  const GLuint &getShadowBufferTextureArrayLayerId() const
  {
    return shadowBufferTextureArrayLayerId;
  }

  /**
   * Get the name of the shadow buffer.
   * 
   * @return The shadow buffer name.
   */
  const std::string &getShadowBufferName() const
  {
    return shadowBufferName;
  }

  /**
   * Get the type of the shadow buffer.
   * 
   * @return The shadow buffer type.
   */
  const ShadowBufferType &getShadowBufferType() const
  {
    return shadowBufferType;
  }
};

/**
 * A manager class for managing shadow buffer used by lights.
 */
class ShadowBufferManager
{
private:
  // The number of faces in a cube map.
  const static unsigned short facesPerCubeMap;

  // Singleton instance of the shadow buffer manager.
  static ShadowBufferManager instance;

  // A map of created textures.
  std::map<const std::string, const std::shared_ptr<const ShadowBufferDetails>> namedShadowBuffers;
  // A map counting the references to the created textures.
  std::map<const std::string, int32_t> namedShadowBufferReferences;

  // The texture ID of the texture array for cone lights.
  const GLuint coneLightTextureArrayId;
  // The shadow framebuffer ID that the texture array for cone lights is attached to.
  const GLuint coneLightShadowBufferId;
  // The set of layer IDs being used in the texture array for cone lights.
  static std::set<uint32_t> assignedConeLightTextureArrayLayerIds;

  // The texture ID of the texture array for point lights.
  const GLuint pointLightTextureArrayId;
  // The shadow framebuffer ID that the texture array for point lights is attached to.
  const GLuint pointLightShadowBufferId;
  // The set of layer IDs being used in the texture array for point lights.
  static std::set<uint32_t> assignedPointLightTextureArrayLayerIds;

  /**
   * Finds a free layer ID in the shadow map texture array that can be assigned to a cone light and returns it.
   * 
   * @return Index of an available layer in the cone light shadow map texture array.
   */
  uint32_t createNewConeLightLayerId()
  {
    // Iterate through all possible indices for layers in the cone light shadow map texture array.
    for (uint32_t i = 0; i < MAX_CONE_LIGHTS; i++)
    {
      // Check if the index is already in use.
      if (assignedConeLightTextureArrayLayerIds.find(i) != assignedConeLightTextureArrayLayerIds.end())
      {
        // If it is, skip it.
        continue;
      }

      // The current index is available. Mark it as in-use and return it.
      assignedConeLightTextureArrayLayerIds.insert(i);
      return i;
    }

    // Could not find any available index. Time to crash.
    std::cout << "Failed at shadowbuffer 1" << std::endl;
    exit(1);
  }

  /**
   * Finds a free layer ID in the shadow map texture array that can be assigned to a point light and returns it.
   * 
   * @return Index of an available layer in the point light shadow map texture array.
   */
  uint32_t createNewPointLightLayerId()
  {
    // Iterate through all possible indices for layers in the point light shadow map texture array.
    for (uint32_t i = 0; i < MAX_POINT_LIGHTS; i++)
    {
      // Check if the index is already in use.
      if (assignedPointLightTextureArrayLayerIds.find(i) != assignedPointLightTextureArrayLayerIds.end())
      {
        // If it is, skip it.
        continue;
      }

      // The current index is available. Mark it as in-use and return it.
      assignedPointLightTextureArrayLayerIds.insert(i);
      return facesPerCubeMap * i;
    }

    // Could not find any available index. Time to crash.
    std::cout << "Failed at shadowbuffer 2" << std::endl;
    exit(1);
  }

  /**
	 * Create a shadow framebuffer that can capture the depth values of objects drawn to it.
	 * 
	 * @param shadowBufferTextureArrayId  The ID of the texture array to attach.
	 * 
	 * @return The ID of the depth texture.
	 */
  GLuint createShadowBuffer(const GLuint &shadowBufferTextureArrayId)
  {
    // Define a variable for storing the framebuffer ID.
    GLuint shadowBufferId;
    // Create a new texture and store the framebuffer ID.
    glGenFramebuffers(1, &shadowBufferId);
    // Bind the framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, shadowBufferId);
    // Attach the depth texture array as the depth texture for the bounded framebuffer.
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowBufferTextureArrayId, 0);

    // Tell OpenGL not to read or draw color data.
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // Check if the framebuffer is ready.
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
      // Framebuffer is not ready. Time to crash.
      std::cout << "Failed at shadow buffer 3" << std::endl;
      exit(1);
    }

    // Unbind the framebuffer now that we're done.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Return the ID of the created framebuffer.
    return shadowBufferId;
  }

  /**
   * Initialize the cone light shadow map texture array to which cone light
   *   shadow maps are drawn to.
   */
  GLuint initializeConeLightTextureArrays()
  {
    GLuint newTextureId;
    // Generate a new texture.
    glGenTextures(1, &newTextureId);

    // Bind the texture as a 2D image texture array.
    glBindTexture(GL_TEXTURE_2D_ARRAY, newTextureId);
    // Define the size of an image, number of images (layers), and the type of data
    //   being drawn to the texture array as a whole.
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT, MAX_CONE_LIGHTS, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    // Provide parameters for behaviour when reading coordinates that are out-of-bounds,
    //   as well as algorithms to use for maginifcation and minification.
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Set the color to black for the border of the layers in the texture array.
    // Since the values for coordinates outside the range of layers in the texture
    //   are clamped to the border, this will make sure that when the texture is
    //   queried for out-of-bounds coordinates, a black color is returned, which
    //   will mark those coordinates as always being in shadow.
    GLfloat outsideMapDepth[] = {0.0, 0.0, 0.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, outsideMapDepth);

    // Unbind the texture now that we're done.
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    // Generate a shadow framebuffer for the cone light texture array and save the ID.
    return newTextureId;
  }

  GLuint initializePointLightTextureArrays()
  {
    GLuint newTextureId;
    // Generate a new texture.
    glGenTextures(1, &newTextureId);

    // Bind the texture as a cube map texture array.
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, newTextureId);
    // Define the size of a cube map, number of cube maps (face layers), and the type
    //   of data being drawn to the texture array as a whole.
    glTexImage3D(
        GL_TEXTURE_CUBE_MAP_ARRAY,
        0,
        GL_DEPTH_COMPONENT,
        FRAMEBUFFER_WIDTH,
        FRAMEBUFFER_HEIGHT,
        facesPerCubeMap * MAX_POINT_LIGHTS,
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        nullptr);

    // Provide parameters for behaviour when reading coordinates that are out-of-bounds,
    //   as well as algorithms to use for maginifcation and minification.
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Unbind the texture now that we're done.
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);

    // Generate a shadow framebuffer for the point light texture array and save the ID.
    return newTextureId;
  }

  ShadowBufferManager()
      : namedShadowBuffers({}),
        namedShadowBufferReferences({}),
        coneLightTextureArrayId(initializeConeLightTextureArrays()),
        coneLightShadowBufferId(createShadowBuffer(coneLightTextureArrayId)),
        pointLightTextureArrayId(initializePointLightTextureArrays()),
        pointLightShadowBufferId(createShadowBuffer(pointLightTextureArrayId))
  {
  }

  ~ShadowBufferManager()
  {
    // Delete the texture array and framebuffer containing the shadow buffer data for cone lights.
    glDeleteTextures(1, &coneLightTextureArrayId);
    glDeleteFramebuffers(1, &coneLightShadowBufferId);

    // Delete the texture array and framebuffer containing the shadow buffer data for point lights.
    glDeleteTextures(1, &pointLightTextureArrayId);
    glDeleteFramebuffers(1, &pointLightShadowBufferId);
  }

public:
  // Preventing copying the shadow buffer manager, making sure only one instance can exist.
  ShadowBufferManager(const ShadowBufferManager &) = delete;

  /**
	 * Load and create an shadow buffer from the given shadow buffer file path. If an shadow buffer with the same name was already created,
	 * return the same shadow buffer.
	 * 
	 * @param shadowBufferName      The name of the shadow buffer.
	 * @param shadowBufferFilePath  The file path to the shadow buffer data.
	 * @param outsideMapDepth       The color value returned when coordinates outside the shadowbuffer is queried.
	 * 
	 * @return The details of the loaded shadow buffer.
	 */
  const std::shared_ptr<const ShadowBufferDetails> &createShadowBuffer(const std::string &shadowBufferName, const ShadowBufferType &shadowBufferType)
  {
    // Check if an shadow buffer with the name already exists.
    const auto existingShadowBuffer = namedShadowBuffers.find(shadowBufferName);
    if (existingShadowBuffer != namedShadowBuffers.end())
    {
      // Shadow buffer already created. Increase its reference count and return it.
      namedShadowBufferReferences[shadowBufferName]++;
      return existingShadowBuffer->second;
    }

    // Define a variable for storing the ID of the shadow framebuffer the shadow is being rendered to.
    GLuint shadowBufferId;
    // Define a variable for storing the ID of the texture array a layer is being assigned from.
    GLuint shadowBufferTextureArrayId;
    // Define a variable for storing the layer in the texture array that the shadow buffer is bound to.
    uint32_t shadowBufferTextureArrayLayerId;
    // Check the type of shadow buffer requested.
    switch (shadowBufferType)
    {
    case POINT:
      // Set the shadow framebuffer ID for point lights.
      shadowBufferId = pointLightShadowBufferId;
      // Set the texture array ID as the one for point lights.
      shadowBufferTextureArrayId = pointLightTextureArrayId;
      // Get a layer assigned for the point light cube map texture.
      shadowBufferTextureArrayLayerId = createNewPointLightLayerId();
      break;
    default:
      // Set the shadow framebuffer ID for cone lights.
      shadowBufferId = coneLightShadowBufferId;
      // Set the texture array ID as the one for cone lights.
      shadowBufferTextureArrayId = coneLightTextureArrayId;
      // Get a layer assigned for the cone light 2D texture.
      shadowBufferTextureArrayLayerId = createNewConeLightLayerId();
    }

    // Create a new shadow buffer details with the captured data.
    const auto newShadowBuffer = std::make_shared<const ShadowBufferDetails>(shadowBufferId, shadowBufferTextureArrayId, shadowBufferTextureArrayLayerId, shadowBufferName, shadowBufferType);

    // Insert the newly created shadow buffer into the map of created textures.
    namedShadowBuffers.insert(std::make_pair(shadowBufferName, newShadowBuffer));
    // Set the reference count of the shadow buffer to 1.
    namedShadowBufferReferences[shadowBufferName] = 1;

    // Return the shadow buffer details.
    return namedShadowBuffers[shadowBufferName];
  }

  /**
   * Return the shadow buffer created with the given name.
   * 
   * @param shadowBufferName  The name of the shadow buffer to return.
   * 
   * @return The shadow buffer created with the given name.
   */
  const std::shared_ptr<const ShadowBufferDetails> &getShadowBufferDetails(const std::string &shadowBufferName) const
  {
    return namedShadowBuffers.at(shadowBufferName);
  }

  /**
	 * Delete a reference to the shadow buffer, and destroy it if no more references are present.
	 * 
	 * @param shadowBufferDetails  The details of the shadow buffer to destroy.
	 */
  void destroyShadowBuffer(const std::shared_ptr<const ShadowBufferDetails> &shadowBufferDetails)
  {
    // Reduce the reference count of the shadow buffer.
    namedShadowBufferReferences[shadowBufferDetails->getShadowBufferName()]--;
    // Check if there are no more references to the shadow buffer.
    if (namedShadowBufferReferences[shadowBufferDetails->getShadowBufferName()] <= 0)
    {
      // No more references left, so time to clean.
      // Remove the shadow buffer from the created textures references map.
      namedShadowBufferReferences.erase(shadowBufferDetails->getShadowBufferName());
      // Remove the shadow buffer from the created textures map.
      namedShadowBuffers.erase(shadowBufferDetails->getShadowBufferName());
      // Check the type of light the shadow map was used for.
      switch (shadowBufferDetails->getShadowBufferType())
      {
      case POINT:
        // Un-assign the layer that was reserved for the shadow buffer in the point light shadow map texture array.
        assignedPointLightTextureArrayLayerIds.erase(shadowBufferDetails->getShadowBufferTextureArrayLayerId() / facesPerCubeMap);
        break;
      default:
        // Un-assign the layer that was reserved for the shadow buffer in the cone light shadow map texture array.
        assignedConeLightTextureArrayLayerIds.erase(shadowBufferDetails->getShadowBufferTextureArrayLayerId());
      }
    }
  }

  /**
   * Get the ID of the texture array of the cone light shadow textures.
   * 
   * @return The ID of the texture array.
   */
  const GLuint &getConeLightTextureArrayId() const
  {
    return coneLightTextureArrayId;
  }

  /**
   * Get the ID of the texture array of the point light shadow textures.
   * 
   * @return The ID of the texture array.
   */
  const GLuint &getPointLightTextureArrayId() const
  {
    return pointLightTextureArrayId;
  }

  /**
   * Get the ID of the shadow framebuffer of the cone light shadow textures.
   * 
   * @return The ID of the shadow buffer.
   */
  const GLuint &getConeLightShadowBufferId() const
  {
    return coneLightShadowBufferId;
  }

  /**
   * Get the ID of the shadow framebuffer of the point light shadow textures.
   * 
   * @return The ID of the shadow buffer.
   */
  const GLuint &getPointLightShadowBufferId() const
  {
    return pointLightShadowBufferId;
  }

  /**
   * Returns the singleton instance of the shadow buffer manager.
   * 
   * @return The shadow buffer manager singleton instance.
   */
  static ShadowBufferManager &getInstance()
  {
    return instance;
  }
};

// Initialize the number of faces in a single cube map static variable.
const unsigned short ShadowBufferManager::facesPerCubeMap = 6;
// Initialize the cone lights texture array assigned layer IDs set static variable.
std::set<uint32_t> ShadowBufferManager::assignedConeLightTextureArrayLayerIds = std::set<uint32_t>({});
// Initialize the point lights texture array assigned layer IDs set static variable.
std::set<uint32_t> ShadowBufferManager::assignedPointLightTextureArrayLayerIds = std::set<uint32_t>({});
// Initialize the shadow buffer manager singleton instance static variable.
ShadowBufferManager ShadowBufferManager::instance;

#endif