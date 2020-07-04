#ifndef INCLUDE_RENDER_CPP
#define INCLUDE_RENDER_CPP

#include <map>
#include <set>

#include <GL/glew.h>

#include "window.cpp"
#include "../camera/camera_base.cpp"
#include "../models/model_base.cpp"

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
    attributeId = createAttributeId();
    attributeIds.insert(attributeId);
  }

  ~VertexAttributeArray()
  {
    attributeIds.erase(attributeId);
    glDisableVertexAttribArray(attributeId);
  }

  void enableAttribute()
  {
    glEnableVertexAttribArray(attributeId);
    glBindBuffer(GL_ARRAY_BUFFER, bufferId);
    glVertexAttribPointer(attributeId, bufferElementSize, GL_FLOAT, GL_FALSE, 0, (void *)0);
  }
};

std::set<GLuint> VertexAttributeArray::attributeIds = std::set<GLuint>({});

class RenderManager
{
  friend class DebugRenderManager;

private:
  static RenderManager instance;

  WindowManager &windowManager;

  std::string activeCameraId;
  std::map<std::string, std::shared_ptr<ModelBase>> registeredModels;
  std::map<std::string, std::shared_ptr<CameraBase>> registeredCameras;

  double startTime;
  double lastTime;

  RenderManager()
      : windowManager(WindowManager::getInstance()),
        registeredModels({}),
        registeredCameras({}),
        startTime(glfwGetTime()),
        lastTime(glfwGetTime()) {}

public:
  RenderManager(RenderManager &) = delete;

  void registerModel(std::shared_ptr<ModelBase> model)
  {
    registeredModels.insert(std::pair<std::string, std::shared_ptr<ModelBase>>(model->getModelId(), model));
  }

  void deregisterModel(std::shared_ptr<ModelBase> model)
  {
    registeredModels.erase(model->getModelId());
  }

  void deregisterModel(std::string modelId)
  {
    registeredModels.erase(modelId);
  }

  void registerCamera(std::shared_ptr<CameraBase> camera)
  {
    registeredCameras.insert(std::pair<std::string, std::shared_ptr<CameraBase>>(camera->getCameraId(), camera));
  }

  void deregisterCamera(std::shared_ptr<CameraBase> camera)
  {
    registeredCameras.erase(camera->getCameraId());
  }

  void deregisterCamera(std::string cameraId)
  {
    registeredCameras.erase(cameraId);
  }

  void registerActiveCamera(std::shared_ptr<CameraBase> camera)
  {
    activeCameraId = camera->getCameraId();
  }

  void registerActiveCamera(std::string cameraId)
  {
    activeCameraId = cameraId;
  }

  void renderModels()
  {
    auto currentTime = glfwGetTime();
    auto deltaTime = currentTime - lastTime;
    auto totalTime = currentTime - startTime;

    auto shaderId = -1;
    auto activeCamera = registeredCameras[activeCameraId];
    auto viewMatrix = activeCamera->getViewMatrix();
    auto projectionMatrix = activeCamera->getProjectionMatrix();
    for (auto model = registeredModels.begin(); model != registeredModels.end(); model++)
    {
      if (shaderId != model->second->getShaderDetails()->getShaderId())
      {
        shaderId = model->second->getShaderDetails()->getShaderId();
        glUseProgram(shaderId);
      }

      auto modelMatrixId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), "modelMatrix");
      auto viewMatrixId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), "viewMatrix");
      auto projectionMatrixId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), "projectionMatrix");
      auto mvpMatrixId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), "mvpMatrix");

      auto totalTimeId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), "totalTime");
      auto deltaTimeId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), "deltaTime");

      auto textureSamplerId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), "textureSampler");

      auto modelMatrix = model->second->getModelMatrix();
      glUniformMatrix4fv(modelMatrixId, 1, GL_FALSE, &modelMatrix[0][0]);
      glUniformMatrix4fv(viewMatrixId, 1, GL_FALSE, &viewMatrix[0][0]);
      glUniformMatrix4fv(projectionMatrixId, 1, GL_FALSE, &projectionMatrix[0][0]);
      auto mvpMatrix = projectionMatrix * viewMatrix * modelMatrix * glm::mat4();
      glUniformMatrix4fv(mvpMatrixId, 1, GL_FALSE, &mvpMatrix[0][0]);

      glUniform1f(totalTimeId, totalTime);
      glUniform1f(deltaTimeId, deltaTime);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, model->second->getTextureDetails()->getTextureId());
      glUniform1i(textureSamplerId, 0);

      VertexAttributeArray vertexArray("VertexArray", model->second->getObjectDetails()->getVertexBufferId(), 3);
      VertexAttributeArray uvArray("UvArray", model->second->getObjectDetails()->getUvBufferId(), 2);
      VertexAttributeArray normalArray("NormalArray", model->second->getObjectDetails()->getNormalBufferId(), 3);

      vertexArray.enableAttribute();
      uvArray.enableAttribute();
      normalArray.enableAttribute();

      glDrawArrays(GL_TRIANGLES, 0, model->second->getObjectDetails()->getBufferSize());
    }

    lastTime = currentTime;
  }

  static RenderManager &getInstance()
  {
    return instance;
  }
};

RenderManager RenderManager::instance;

#endif