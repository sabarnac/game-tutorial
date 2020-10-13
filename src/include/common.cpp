#ifndef INCLUDE_COMMON_CPP
#define INCLUDE_COMMON_CPP

#include <iostream>
#include <string>
#include <map>
#include <set>
#include <limits>

#include <GL/glew.h>

/**
 * Class for storing the vertex attribute array information.
 */
class VertexAttributeArray
{
private:
  // The set of attribute IDs already in use.
  static std::set<GLuint> attributeIds;

  // The ID of the attribute.
  const GLuint attributeId;
  // The name of the attribute.
  const std::string attributeName;
  // The ID of the buffer the attribute is linked to.
  const GLuint bufferId;
  // The size of the elements in the buffer.
  const unsigned int bufferElementSize;
  // The type of the attribute data
  const GLenum attributeType;

  /**
   * Creates a new attribute ID for the current attribute to use.
   * 
   * @return The attribute ID.
   */
  GLuint createAttributeId()
  {
    // Get the maximum possible attribute ID that can be used.
    const GLuint maxId = std::numeric_limits<GLuint>::max();
    // Iterate through all possible attribute IDs that can be used.
    for (GLuint i = 0; i < maxId; i++)
    {
      // Check if the atribute ID is being used.
      if (attributeIds.find(i) == attributeIds.end())
      {
        // The attribute ID is available. Return it for use.
        return i;
      }
    }

    // Could not get an available attribute ID. Time to crash.
    std::cout << "Failed at vertex attribute array" << std::endl;
    exit(1);
  }

public:
  // Preventing copying the vertex attribute array, making sure only one instance can exist.
  VertexAttributeArray(const VertexAttributeArray &) = delete;

  VertexAttributeArray(const std::string &attributeName, const GLuint &bufferId, const unsigned int &bufferElementSize, const GLenum &attributeType = GL_FLOAT)
      : attributeId(createAttributeId()),
        attributeName(attributeName),
        bufferId(bufferId),
        bufferElementSize(bufferElementSize),
        attributeType(attributeType)
  {
    // Insert it to the set of attribute IDs being used.
    attributeIds.insert(attributeId);
  }

  ~VertexAttributeArray()
  {
    // Delete the attribute ID from the set of used IDs.
    attributeIds.erase(attributeId);
    // Disable the vertex attribute array from being used by the GPU.
    glDisableVertexAttribArray(attributeId);
  }

  void enableAttribute()
  {
    // Enable the vertex attribute array for being used by the GPU.
    glEnableVertexAttribArray(attributeId);
    // Bind the buffer as the array buffer the vertex attribute will link with.
    glBindBuffer(GL_ARRAY_BUFFER, bufferId);
    // Define the details regarding the vertex attribute list stored in the array buffer.
    glVertexAttribPointer(attributeId, bufferElementSize, attributeType, GL_FALSE, 0, (void *)0);
  }
};

// Initialize the set of used attribute IDs to an empty set.
std::set<GLuint> VertexAttributeArray::attributeIds = std::set<GLuint>({});

#endif