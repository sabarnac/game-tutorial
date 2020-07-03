#ifndef INCLUDE_VERTEX_ATTRIBUTE_ARRAY_CPP
#define INCLUDE_VERTEX_ATTRIBUTE_ARRAY_CPP

#include <string>
#include <set>
#include <iostream>

#include <GL/glew.h>

class VertexAttributeArray
{

private:
  static std::set<GLuint> attributeIds;

  GLuint attributeId;
  std::string attributeName;
  GLuint bufferId;
  uint bufferElementSize;

  GLuint createAttributeId()
  {
    GLuint maxId = std::numeric_limits<GLuint>::max();
    for (GLuint i = 0; i < maxId; i++)
    {
      if (attributeIds.find(i) == attributeIds.end())
      {
        return i;
      }
    }
    exit(1);
  }

public:
  VertexAttributeArray(VertexAttributeArray &) = delete;

  VertexAttributeArray(std::string attributeName, GLuint bufferId, uint bufferElementSize)
      : attributeName(attributeName),
        bufferId(bufferId),
        bufferElementSize(bufferElementSize)
  {
    std::cout << "[VertexAttributeArray] "
              << "Constructing VertexAttributeArray." << std::endl;
    attributeId = createAttributeId();
    attributeIds.insert(attributeId);
    std::cout << "[VertexAttributeArray] "
              << "Constructed VertexAttributeArray." << std::endl;
  }

  ~VertexAttributeArray()
  {
    std::cout << "[VertexAttributeArray] "
              << "Destroying VertexAttributeArray: " << attributeName << std::endl;
    attributeIds.erase(attributeId);
    glDisableVertexAttribArray(attributeId);
    std::cout << "[VertexAttributeArray] "
              << "Destroyed VertexAttributeArray." << std::endl;
  }

  void enableAttribute()
  {
    std::cout << "[VertexAttributeArray] "
              << "Enabling attribute: " << attributeName << std::endl
              << "[VertexAttributeArray] "
              << "\tAttribute ID: " << attributeId << std::endl;

    glEnableVertexAttribArray(attributeId);
    glBindBuffer(GL_ARRAY_BUFFER, bufferId);
    glVertexAttribPointer(attributeId, bufferElementSize, GL_FLOAT, GL_FALSE, 0, (void *)0);

    std::cout << "[VertexAttributeArray] "
              << "Enabled attribute." << std::endl;
  }
};

std::set<GLuint> VertexAttributeArray::attributeIds = std::set<GLuint>({});

#endif