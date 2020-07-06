#ifndef INCLUDE_SHADOWBUFFER_CPP
#define INCLUDE_SHADOWBUFFER_CPP

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <memory>

#include <GL/glew.h>

#include "constants.cpp"

enum ShadowBufferType
{
  SIMPLE,
  CUBE
};

class ShadowBufferDetails
{
  friend class ShadowBufferManager;

private:
  GLuint shadowBufferId;
  GLuint shadowBufferTextureId;
  std::string shadowBufferName;
  ShadowBufferType shadowBufferType;

public:
  ShadowBufferDetails(ShadowBufferDetails &) = delete;

  ShadowBufferDetails(GLuint shadowBufferId, GLuint shadowBufferTextureId, std::string shadowBufferName, ShadowBufferType shadowBufferType)
      : shadowBufferId(shadowBufferId),
        shadowBufferTextureId(shadowBufferTextureId),
        shadowBufferName(shadowBufferName),
        shadowBufferType(shadowBufferType) {}

  GLuint getShadowBufferId()
  {
    return shadowBufferId;
  }

  GLuint getShadowBufferTextureId()
  {
    return shadowBufferTextureId;
  }

  std::string getShadowBufferName()
  {
    return shadowBufferName;
  }

  ShadowBufferType getShadowBufferType()
  {
    return shadowBufferType;
  }
};

class ShadowBufferManager
{
private:
  static ShadowBufferManager instance;

  std::map<std::string, std::shared_ptr<ShadowBufferDetails>> namedShadowBuffers;
  std::map<std::string, int> namedShadowBufferReferences;

  GLuint createSimpleShadowBufferTexture(float outsideMapDepth[3])
  {
    GLuint shadowBufferTextureId;
    glGenTextures(1, &shadowBufferTextureId);
    glBindTexture(GL_TEXTURE_2D, shadowBufferTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, FRAMEBUFFER_WIDTH, FRAMEBUFFER_WIDTH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, outsideMapDepth);

    glBindTexture(GL_TEXTURE_2D, 0);
    return shadowBufferTextureId;
  }

  GLuint createCubeShadowBufferTexture()
  {
    GLuint shadowBufferTextureId;
    glGenTextures(1, &shadowBufferTextureId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, shadowBufferTextureId);

    for (auto i = 0; i < 6; i++)
    {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, FRAMEBUFFER_WIDTH, FRAMEBUFFER_WIDTH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    return shadowBufferTextureId;
  }

  GLuint createShadowBuffer(std::string shadowBufferName, GLuint shadowBufferTextureId)
  {
    GLuint shadowBufferId;
    glGenFramebuffers(1, &shadowBufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowBufferId);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowBufferTextureId, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
      std::cout << "Failed at shadow buffer" << std::endl;
      exit(1);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return shadowBufferId;
  }

  ShadowBufferManager()
      : namedShadowBuffers({}),
        namedShadowBufferReferences({}) {}

public:
  ShadowBufferManager(ShadowBufferManager &) = delete;

  std::shared_ptr<ShadowBufferDetails> &createShadowBuffer(std::string shadowBufferName, ShadowBufferType shadowBufferType)
  {
    float defaultOutsideMapDepth[] = {0.0f, 0.0f, 0.0f};
    return createShadowBuffer(shadowBufferName, shadowBufferType, defaultOutsideMapDepth);
  }

  std::shared_ptr<ShadowBufferDetails> &createShadowBuffer(std::string shadowBufferName, ShadowBufferType shadowBufferType, float outsideMapDepth[3])
  {
    auto existingShadowBuffer = namedShadowBuffers.find(shadowBufferName);
    if (existingShadowBuffer != namedShadowBuffers.end())
    {
      namedShadowBufferReferences[shadowBufferName]++;
      return existingShadowBuffer->second;
    }

    GLuint shadowBufferTextureId;
    switch (shadowBufferType)
    {
    case CUBE:
      shadowBufferTextureId = createCubeShadowBufferTexture();
      break;
    default:
      shadowBufferTextureId = createSimpleShadowBufferTexture(outsideMapDepth);
    }

    GLuint shadowBufferId = createShadowBuffer(shadowBufferName, shadowBufferTextureId);
    auto newShadowBuffer = std::make_shared<ShadowBufferDetails>(shadowBufferId, shadowBufferTextureId, shadowBufferName, shadowBufferType);

    namedShadowBuffers.insert(std::pair<std::string, std::shared_ptr<ShadowBufferDetails>>(shadowBufferName, newShadowBuffer));
    namedShadowBufferReferences[shadowBufferName] = 1;

    return namedShadowBuffers[shadowBufferName];
  }

  std::shared_ptr<ShadowBufferDetails> &getShadowBufferDetails(std::string shadowBufferName)
  {
    return namedShadowBuffers[shadowBufferName];
  }

  void destroyShadowBuffer(std::shared_ptr<ShadowBufferDetails> &shadowBufferDetails)
  {
    namedShadowBufferReferences[shadowBufferDetails->getShadowBufferName()]--;
    if (namedShadowBufferReferences[shadowBufferDetails->getShadowBufferName()] <= 0)
    {
      namedShadowBufferReferences.erase(shadowBufferDetails->getShadowBufferName());
      namedShadowBuffers.erase(shadowBufferDetails->getShadowBufferName());
      glDeleteFramebuffers(1, &shadowBufferDetails->shadowBufferId);
      glDeleteTextures(1, &shadowBufferDetails->shadowBufferTextureId);
    }
  }

  static ShadowBufferManager &getInstance()
  {
    return instance;
  }
};

ShadowBufferManager ShadowBufferManager::instance;

#endif