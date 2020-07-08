#ifndef INCLUDE_SHADOWBUFFER_CPP
#define INCLUDE_SHADOWBUFFER_CPP

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <memory>

#include <GL/glew.h>

#include "constants.cpp"

/**
 * Enum of supported shadow buffer types.
 */
enum ShadowBufferType
{
  SIMPLE,
  CUBE
};

/**
 * Class for containing the details of the shadow buffer.
 */
class ShadowBufferDetails
{
  // Let the shadow buffer manager access private variables.
  friend class ShadowBufferManager;

private:
  // The name of the shadow buffer.
  std::string shadowBufferName;

  // The ID of the shadow buffer.
  GLuint shadowBufferId;
  // The ID of the texture the shadow buffer copies data to.
  GLuint shadowBufferTextureId;
  // The type of the shadow buffer.
  ShadowBufferType shadowBufferType;

public:
  ShadowBufferDetails(GLuint shadowBufferId, GLuint shadowBufferTextureId, std::string shadowBufferName, ShadowBufferType shadowBufferType)
      : shadowBufferId(shadowBufferId),
        shadowBufferTextureId(shadowBufferTextureId),
        shadowBufferName(shadowBufferName),
        shadowBufferType(shadowBufferType) {}

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
   * Get the ID of the texture the shadow buffer copies data to.
   * 
   * @return The shadow buffer texture ID.
   */
  GLuint getShadowBufferTextureId()
  {
    return shadowBufferTextureId;
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
  // The width of the framebuffer;
  static int frameBufferWidth;
  // The height of the framebuffer;
  static int frameBufferHeight;

  // Singleton instance of the shadow buffer manager.
  static ShadowBufferManager instance;

  // A map of created textures.
  std::map<std::string, std::shared_ptr<ShadowBufferDetails>> namedShadowBuffers;
  // A map counting the references to the created textures.
  std::map<std::string, int> namedShadowBufferReferences;

  /**
	 * Create a 2D depth texture, with a border color provided for the color to return if a coordinate outside the shadow map was requested.
	 * 
	 * @param outsideMapDepth  The color of the border.
	 * 
	 * @return The ID of the depth texture.
	 */
  GLuint createSimpleShadowBufferTexture(float outsideMapDepth[3])
  {
    // Define a variable for storing the texture ID.
    GLuint shadowBufferTextureId;
    // Create a new texture and store the texture ID.
    glGenTextures(1, &shadowBufferTextureId);
    // Bind the texture as a 2D texture.
    glBindTexture(GL_TEXTURE_2D, shadowBufferTextureId);
    // Create a 2D depth texture image on the bounded texture.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, frameBufferWidth, frameBufferHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    // Provide parameters for behaviour when reading coordinates that are out-of-bounds,
    //   the algorithms to use for maginifcation and minification, and the border color.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, outsideMapDepth);

    // Unbind the texture now that we're done.
    glBindTexture(GL_TEXTURE_2D, 0);

    // Return the ID of the created texture.
    return shadowBufferTextureId;
  }

  /**
	 * Create a cubemap depth texture.
	 * 
	 * @return The ID of the cubemap depth texture.
	 */
  GLuint createCubeShadowBufferTexture()
  {
    // Define a variable for storing the texture ID.
    GLuint shadowBufferTextureId;
    // Create a new texture and store the texture ID.
    glGenTextures(1, &shadowBufferTextureId);
    // Bind the texture as a cubemap texture.
    glBindTexture(GL_TEXTURE_CUBE_MAP, shadowBufferTextureId);

    // Iterate each face of the cube map
    for (auto face = 0; face < 6; face++)
    {
      // Create a 2D depth texture image on the bounded cubemap texture at the face.
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_DEPTH_COMPONENT, frameBufferWidth, frameBufferHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    }

    // Provide parameters for behaviour when reading coordinates that are out-of-bounds,
    //   as well as algorithms to use for maginifcation and minification.
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Unbind the texture now that we're done.
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    // Return the ID of the created texture.
    return shadowBufferTextureId;
  }

  /**
	 * Create a shadow framebuffer that can capture the depth values of objects drawn to it.
	 * 
	 * @param outsideMapDepth  The color of the border.
	 * 
	 * @return The ID of the depth texture.
	 */
  GLuint createShadowBuffer(std::string shadowBufferName, GLuint shadowBufferTextureId)
  {
    // Define a variable for storing the framebuffer ID.
    GLuint shadowBufferId;
    // Create a new texture and store the framebuffer ID.
    glGenFramebuffers(1, &shadowBufferId);
    // Bind the framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, shadowBufferId);
    // Attach the depth texture as the depth texture for the bounded framebuffer.
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowBufferTextureId, 0);

    // Tell OpenGL not to read or draw color data.
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // Check if the framebuffer is ready.
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
      // Framebuffer is not ready. Time to crash.
      std::cout << "Failed at shadow buffer" << std::endl;
      exit(1);
    }

    // Unbind the framebuffer now that we're done.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Return the ID of the created framebuffer.
    return shadowBufferId;
  }

  ShadowBufferManager()
      : namedShadowBuffers({}),
        namedShadowBufferReferences({}) {}

public:
  // Preventing copying the shadow buffer manager, making sure only one instance can exist.
  ShadowBufferManager(ShadowBufferManager &) = delete;

  /**
	 * Load and create an shadow buffer from the given shadow buffer file path. If an shadow buffer with the same name was already created,
	 * return the same shadow buffer.
	 * 
	 * @param shadowBufferName      The name of the shadow buffer.
	 * @param shadowBufferFilePath  The file path to the shadow buffer data.
	 * 
	 * @return The details of the loaded shadow buffer.
	 */
  std::shared_ptr<ShadowBufferDetails> &createShadowBuffer(std::string shadowBufferName, ShadowBufferType shadowBufferType)
  {
    // Define a default color value that is returned when coordinates outside the shadowbuffer is queried.
    float defaultOutsideMapDepth[] = {0.0f, 0.0f, 0.0f};
    // Create the shadow buffer and return the details.
    return createShadowBuffer(shadowBufferName, shadowBufferType, defaultOutsideMapDepth);
  }

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
  std::shared_ptr<ShadowBufferDetails> &createShadowBuffer(std::string shadowBufferName, ShadowBufferType shadowBufferType, float outsideMapDepth[3])
  {
    // Check if an shadow buffer with the name already exists.
    auto existingShadowBuffer = namedShadowBuffers.find(shadowBufferName);
    if (existingShadowBuffer != namedShadowBuffers.end())
    {
      // Shadow buffer already created. Increase its reference count and return it.
      namedShadowBufferReferences[shadowBufferName]++;
      return existingShadowBuffer->second;
    }

    // Define a variable for storing the shadow buffer texture ID.
    GLuint shadowBufferTextureId;
    // Check the type of shadow buffer requested.
    switch (shadowBufferType)
    {
    case CUBE:
      // Create a cubemap shadow buffer.
      shadowBufferTextureId = createCubeShadowBufferTexture();
      break;
    default:
      // Create a 2D shadow buffer.
      shadowBufferTextureId = createSimpleShadowBufferTexture(outsideMapDepth);
    }

    // Create the shadow buffer and store its details.
    GLuint shadowBufferId = createShadowBuffer(shadowBufferName, shadowBufferTextureId);

    // Create a new shadow buffer details with the captured data.
    auto newShadowBuffer = std::make_shared<ShadowBufferDetails>(shadowBufferId, shadowBufferTextureId, shadowBufferName, shadowBufferType);

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
      // Delete the framebuffer containing the shadow buffer data.
      glDeleteFramebuffers(1, &shadowBufferDetails->shadowBufferId);
      // Delete the texture containing the object depth data.
      glDeleteTextures(1, &shadowBufferDetails->shadowBufferTextureId);
    }
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
// Initialize the shadow buffer manager singleton instance static variable.
ShadowBufferManager ShadowBufferManager::instance;

#endif