#ifndef INCLUDE_FRAMEBUFFER_CPP
#define INCLUDE_FRAMEBUFFER_CPP

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <memory>

#include <GL/glew.h>

#include "constants.cpp"

enum FrameBufferType
{
  SIMPLE,
  SIMPLE_WITH_DEPTH,
  CUBE,
  CUBE_WITH_DEPTH
};

class FrameBufferDetails
{
  friend class FrameBufferManager;

private:
  GLuint frameBufferId;
  GLuint frameBufferTextureId;
  std::string frameBufferName;
  FrameBufferType frameBufferType;

public:
  FrameBufferDetails(FrameBufferDetails &) = delete;

  FrameBufferDetails(GLuint frameBufferId, GLuint frameBufferTextureId, std::string frameBufferName, FrameBufferType frameBufferType)
      : frameBufferId(frameBufferId),
        frameBufferTextureId(frameBufferTextureId),
        frameBufferName(frameBufferName),
        frameBufferType(frameBufferType) {}

  GLuint getFrameBufferId()
  {
    return frameBufferId;
  }

  GLuint getFrameBufferTextureId()
  {
    return frameBufferTextureId;
  }

  std::string getFrameBufferName()
  {
    return frameBufferName;
  }

  FrameBufferType getFrameBufferType()
  {
    return frameBufferType;
  }
};

class FrameBufferManager
{
private:
  static FrameBufferManager instance;

  std::map<std::string, std::shared_ptr<FrameBufferDetails>> namedFrameBuffers;
  std::map<std::string, int> namedFrameBufferReferences;

  GLuint createSimpleFrameBufferTexture()
  {
    GLuint frameBufferTextureId;
    glGenTextures(1, &frameBufferTextureId);
    glBindTexture(GL_TEXTURE_2D, frameBufferTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FRAMEBUFFER_WIDTH, FRAMEBUFFER_WIDTH, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    return frameBufferTextureId;
  }

  GLuint createCubeFrameBufferTexture()
  {
    GLuint frameBufferTextureId;
    glGenTextures(1, &frameBufferTextureId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, frameBufferTextureId);

    for (auto i = 0; i < 6; i++)
    {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, FRAMEBUFFER_WIDTH, FRAMEBUFFER_WIDTH, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    return frameBufferTextureId;
  }

  GLuint createDepthRenderBuffer(GLuint frameBufferId)
  {
    GLuint depthRenderBufferId;
    glGenRenderbuffers(1, &depthRenderBufferId);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBufferId);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, FRAMEBUFFER_WIDTH, FRAMEBUFFER_WIDTH);

    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return depthRenderBufferId;
  }

  GLuint createFrameBuffer(std::string frameBufferName, GLuint frameBufferTextureId, bool withDepth = false)
  {
    GLuint frameBufferId;
    glGenFramebuffers(1, &frameBufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, frameBufferTextureId, 0);

    if (withDepth)
    {
      auto depthBuffer = createDepthRenderBuffer(frameBufferId);
    }

    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawBuffers);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
      std::cout << "Failed at frame buffer" << std::endl;
      exit(1);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return frameBufferId;
  }

  FrameBufferManager()
      : namedFrameBuffers({}),
        namedFrameBufferReferences({}) {}

public:
  FrameBufferManager(FrameBufferManager &) = delete;

  std::shared_ptr<FrameBufferDetails> &createFrameBuffer(std::string frameBufferName, FrameBufferType frameBufferType)
  {
    auto existingFrameBuffer = namedFrameBuffers.find(frameBufferName);
    if (existingFrameBuffer != namedFrameBuffers.end())
    {
      namedFrameBufferReferences[frameBufferName]++;
      return existingFrameBuffer->second;
    }

    GLuint frameBufferTextureId;
    switch (frameBufferType)
    {
    case CUBE:
    case CUBE_WITH_DEPTH:
      frameBufferTextureId = createCubeFrameBufferTexture();
      break;
    default:
      frameBufferTextureId = createSimpleFrameBufferTexture();
    }

    GLuint frameBufferId = createFrameBuffer(frameBufferName, frameBufferTextureId, frameBufferType == SIMPLE_WITH_DEPTH || frameBufferType == CUBE_WITH_DEPTH);
    auto newFrameBuffer = std::make_shared<FrameBufferDetails>(frameBufferId, frameBufferTextureId, frameBufferName, frameBufferType);

    namedFrameBuffers.insert(std::pair<std::string, std::shared_ptr<FrameBufferDetails>>(frameBufferName, newFrameBuffer));
    namedFrameBufferReferences[frameBufferName] = 1;

    return namedFrameBuffers[frameBufferName];
  }

  std::shared_ptr<FrameBufferDetails> &getFrameBufferDetails(std::string frameBufferName)
  {
    return namedFrameBuffers[frameBufferName];
  }

  void destroyFrameBuffer(std::shared_ptr<FrameBufferDetails> &frameBufferDetails)
  {
    namedFrameBufferReferences[frameBufferDetails->getFrameBufferName()]--;
    if (namedFrameBufferReferences[frameBufferDetails->getFrameBufferName()] <= 0)
    {
      namedFrameBufferReferences.erase(frameBufferDetails->getFrameBufferName());
      namedFrameBuffers.erase(frameBufferDetails->getFrameBufferName());
      glDeleteFramebuffers(1, &frameBufferDetails->frameBufferId);
      glDeleteTextures(1, &frameBufferDetails->frameBufferTextureId);
    }
  }

  static FrameBufferManager &getInstance()
  {
    return instance;
  }
};

FrameBufferManager FrameBufferManager::instance;

#endif