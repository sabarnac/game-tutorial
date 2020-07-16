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
  GLuint shadowBufferId;
  // The ID of the texture array the shadow buffer copies data to in a layer.
  GLuint shadowBufferTextureArrayId;
  // The ID of the layer of the texture array that the shadow buffer data is stored in.
  unsigned int shadowBufferTextureArrayLayerId;
  // The type of the shadow buffer.
  ShadowBufferType shadowBufferType;

  // The name of the shadow buffer.
  std::string shadowBufferName;

public:
  ShadowBufferDetails(
      GLuint shadowBufferId,
      GLuint shadowBufferTextureArrayId,
      unsigned int shadowBufferTextureArrayLayerId,
      std::string shadowBufferName,
      ShadowBufferType shadowBufferType)
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
  GLuint getShadowBufferId()
  {
    return shadowBufferId;
  }

  /**
   * Get the ID of the texture array the shadow buffer copies data to in a layer.
   * 
   * @return The shadow buffer texture array ID.
   */
  GLuint getShadowBufferTextureArrayId()
  {
    return shadowBufferTextureArrayId;
  }

  /**
   * Get the ID of the layer of the texture array that the shadow buffer data is stored in.
   * 
   * @return The layer ID.
   */
  GLuint getShadowBufferTextureArrayLayerId()
  {
    return shadowBufferTextureArrayLayerId;
  }

  /**
   * Get the name of the shadow buffer.
   * 
   * @return The shadow buffer name.
   */
  std::string getShadowBufferName()
  {
    return shadowBufferName;
  }

  /**
   * Get the type of the shadow buffer.
   * 
   * @return The shadow buffer type.
   */
  ShadowBufferType getShadowBufferType()
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
  // The width of the framebuffer.
  static int frameBufferWidth;
  // The height of the framebuffer.
  static int frameBufferHeight;

  // The maximum number of cone lights supported.
  static unsigned int maxConeLights;
  // The maximum number of point lights supported.
  static unsigned int maxPointLights;
  // The number of faces in a cube map.
  static unsigned short facesPerCubeMap;

  // Singleton instance of the shadow buffer manager.
  static ShadowBufferManager instance;

  // The shadow framebuffer ID that the texture array for cone lights is attached to.
  static GLuint coneLightShadowBufferId;
  // The texture ID of the texture array for cone lights.
  static GLuint coneLightTextureArrayId;
  // The set of layer IDs being used in the texture array for cone lights.
  static std::set<unsigned int> assignedConeLightTextureArrayLayerIds;

  // The shadow framebuffer ID that the texture array for point lights is attached to.
  static GLuint pointLightShadowBufferId;
  // The texture ID of the texture array for point lights.
  static GLuint pointLightTextureArrayId;
  // The set of layer IDs being used in the texture array for point lights.
  static std::set<unsigned int> assignedPointLightTextureArrayLayerIds;

  // A map of created textures.
  std::map<std::string, std::shared_ptr<ShadowBufferDetails>> namedShadowBuffers;
  // A map counting the references to the created textures.
  std::map<std::string, int> namedShadowBufferReferences;

  /**
   * Finds a free layer ID in the shadow map texture array that can be assigned to a cone light and returns it.
   * 
   * @return Index of an available layer in the cone light shadow map texture array.
   */
  unsigned int createNewConeLightLayerId()
  {
    // Iterate through all possible indices for layers in the cone light shadow map texture array.
    for (unsigned int i = 0; i < maxConeLights; i++)
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
  unsigned int createNewPointLightLayerId()
  {
    // Iterate through all possible indices for layers in the point light shadow map texture array.
    for (unsigned int i = 0; i < maxPointLights; i++)
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
  GLuint createShadowBuffer(GLuint shadowBufferTextureArrayId)
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
  void initializeConeLightTextureArrays()
  {
    // Generate a new texture.
    glGenTextures(1, &coneLightTextureArrayId);

    // Bind the texture as a 2D image texture array.
    glBindTexture(GL_TEXTURE_2D_ARRAY, coneLightTextureArrayId);
    // Define the size of an image, number of images (layers), and the type of data
    //   being drawn to the texture array as a whole.
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, frameBufferWidth, frameBufferHeight, maxConeLights, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    // Iterate through all the layers in the texture array.
    for (auto layerId = 0; layerId < maxConeLights; layerId++)
    {
      // Define the size of an image and the type of data being drawn to the texture
      //   array at the specified layer.
      glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, layerId, frameBufferWidth, frameBufferHeight, 1, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    }

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
    coneLightShadowBufferId = createShadowBuffer(coneLightTextureArrayId);
  }

  void initializePointLightTextureArrays()
  {
    // Generate a new texture.
    glGenTextures(1, &pointLightTextureArrayId);

    // Bind the texture as a cube map texture array.
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY_ARB, pointLightTextureArrayId);
    // Define the size of a cube map, number of cube maps (face layers), and the type
    //   of data being drawn to the texture array as a whole.
    glTexImage3D(
        GL_TEXTURE_CUBE_MAP_ARRAY_ARB,
        0,
        GL_DEPTH_COMPONENT,
        frameBufferWidth,
        frameBufferHeight,
        facesPerCubeMap * maxPointLights,
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        nullptr);

    // Iterate through all the layers in the texture array.
    for (auto layerId = 0; layerId < maxPointLights; layerId++)
    {
      // Iterate through the number of faces in a cube map.
      for (auto faceId = 0; faceId < facesPerCubeMap; faceId++)
      {
        // Define the size of an image and the type of data being drawn to the texture
        //   array at the specified layer face.
        glTexSubImage3D(
            GL_TEXTURE_CUBE_MAP_ARRAY_ARB,
            0,
            0,
            0,
            (facesPerCubeMap * layerId) + faceId,
            frameBufferWidth,
            frameBufferHeight,
            1,
            GL_DEPTH_COMPONENT,
            GL_FLOAT,
            nullptr);
      }
    }

    // Provide parameters for behaviour when reading coordinates that are out-of-bounds,
    //   as well as algorithms to use for maginifcation and minification.
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY_ARB, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Unbind the texture now that we're done.
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY_ARB, 0);

    // Generate a shadow framebuffer for the point light texture array and save the ID.
    pointLightShadowBufferId = createShadowBuffer(pointLightTextureArrayId);
  }

  ShadowBufferManager()
      : namedShadowBuffers({}),
        namedShadowBufferReferences({})
  {
    // Check if the cone light shadow map texture array has already been created.
    if (coneLightTextureArrayId == 0)
    {
      // If not, create it.
      initializeConeLightTextureArrays();
    }
    // Check if the point light shadow map texture array has already been created.
    if (pointLightTextureArrayId == 0)
    {
      // If not, create it.
      initializePointLightTextureArrays();
    }
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
  ShadowBufferManager(ShadowBufferManager &) = delete;

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
  std::shared_ptr<ShadowBufferDetails> &createShadowBuffer(std::string shadowBufferName, ShadowBufferType shadowBufferType)
  {
    // Check if an shadow buffer with the name already exists.
    auto existingShadowBuffer = namedShadowBuffers.find(shadowBufferName);
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
    unsigned int shadowBufferTextureArrayLayerId;
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
    auto newShadowBuffer = std::make_shared<ShadowBufferDetails>(shadowBufferId, shadowBufferTextureArrayId, shadowBufferTextureArrayLayerId, shadowBufferName, shadowBufferType);

    // Insert the newly created shadow buffer into the map of created textures.
    namedShadowBuffers[shadowBufferName] = newShadowBuffer;
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
  std::shared_ptr<ShadowBufferDetails> &getShadowBufferDetails(std::string shadowBufferName)
  {
    return namedShadowBuffers[shadowBufferName];
  }

  /**
	 * Delete a reference to the shadow buffer, and destroy it if no more references are present.
	 * 
	 * @param shadowBufferDetails  The details of the shadow buffer to destroy.
	 */
  void destroyShadowBuffer(std::shared_ptr<ShadowBufferDetails> &shadowBufferDetails)
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
  GLuint getConeLightTextureArrayId()
  {
    return coneLightTextureArrayId;
  }

  /**
   * Get the ID of the texture array of the point light shadow textures.
   * 
   * @return The ID of the texture array.
   */
  GLuint getPointLightTextureArrayId()
  {
    return pointLightTextureArrayId;
  }

  /**
   * Get the ID of the shadow framebuffer of the cone light shadow textures.
   * 
   * @return The ID of the shadow buffer.
   */
  GLuint getConeLightShadowBufferId()
  {
    return coneLightShadowBufferId;
  }

  /**
   * Get the ID of the shadow framebuffer of the point light shadow textures.
   * 
   * @return The ID of the shadow buffer.
   */
  GLuint getPointLightShadowBufferId()
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

// Initialize the framebuffer width instance static variable.
int ShadowBufferManager::frameBufferWidth = FRAMEBUFFER_WIDTH;
// Initialize the framebuffer height instance static variable.
int ShadowBufferManager::frameBufferHeight = FRAMEBUFFER_HEIGHT;
// Initialize the max cone lights static variable.
unsigned int ShadowBufferManager::maxConeLights = MAX_CONE_LIGHTS;
// Initialize the max point lights static variable.
unsigned int ShadowBufferManager::maxPointLights = MAX_POINT_LIGHTS;
// Initialize the number of faces in a single cube map static variable.
unsigned short ShadowBufferManager::facesPerCubeMap = 6;
// Initialize the cone lights texture array shadow framebuffer ID static variable.
GLuint ShadowBufferManager::coneLightShadowBufferId = 0;
// Initialize the cone lights texture array ID static variable.
GLuint ShadowBufferManager::coneLightTextureArrayId = 0;
// Initialize the cone lights texture array assigned layer IDs set static variable.
std::set<unsigned int> ShadowBufferManager::assignedConeLightTextureArrayLayerIds = std::set<unsigned int>({});
// Initialize the point lights texture array shadow framebuffer ID static variable.
GLuint ShadowBufferManager::pointLightShadowBufferId = 0;
// Initialize the point lights texture array ID static variable.
GLuint ShadowBufferManager::pointLightTextureArrayId = 0;
// Initialize the point lights texture array assigned layer IDs set static variable.
std::set<unsigned int> ShadowBufferManager::assignedPointLightTextureArrayLayerIds = std::set<unsigned int>({});
// Initialize the shadow buffer manager singleton instance static variable.
ShadowBufferManager ShadowBufferManager::instance;

#endif