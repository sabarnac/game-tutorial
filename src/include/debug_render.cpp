#ifndef INCLUDE_DEBUG_RENDER_CPP
#define INCLUDE_DEBUG_RENDER_CPP

#include <map>
#include <set>

#include <GL/glew.h>

#include "window.cpp"
#include "shader.cpp"
#include "object.cpp"
#include "render.cpp"

class DebugVertexAttributeArray
{

private:
  static std::set<GLuint> attributeIds;

  const GLuint attributeId;
  const std::string attributeName;
  const GLuint bufferId;
  const unsigned int bufferElementSize;

  GLuint createAttributeId()
  {
    const GLuint maxId = std::numeric_limits<GLuint>::max();
    for (GLuint i = 0; i < maxId; i++)
    {
      if (attributeIds.find(i) == attributeIds.end())
      {
        return i;
      }
    }
    std::cout << "Failed at debug render" << std::endl;
    exit(1);
  }

public:
  DebugVertexAttributeArray(const DebugVertexAttributeArray &) = delete;

  DebugVertexAttributeArray(const std::string &attributeName, const GLuint &bufferId, const unsigned int &bufferElementSize)
      : attributeId(createAttributeId()),
        attributeName(attributeName),
        bufferId(bufferId),
        bufferElementSize(bufferElementSize)
  {
    attributeIds.insert(attributeId);
  }

  ~DebugVertexAttributeArray()
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

std::set<GLuint> DebugVertexAttributeArray::attributeIds = std::set<GLuint>({});

class DebugRenderManager
{
private:
  static DebugRenderManager instance;

  const static glm::vec4 debugColor1;
  const static glm::vec4 debugColor2;
  const static glm::vec4 debugColor3;

  const WindowManager &windowManager;
  ObjectManager &objectManager;
  ShaderManager &shaderManager;
  const RenderManager &renderManager;

  const std::shared_ptr<const ObjectDetails> sphereDetails;
  const std::shared_ptr<const ShaderDetails> debugAabbShader;
  const std::shared_ptr<const ShaderDetails> debugBoxShader;
  const std::shared_ptr<const ShaderDetails> debugSphereShader;
  const GLuint debugModelBufferId;

  GLuint createDebugModelBuffer()
  {
    GLuint bufferId;
    glGenBuffers(1, &bufferId);
    return bufferId;
  }

  DebugRenderManager()
      : windowManager(WindowManager::getInstance()),
        objectManager(ObjectManager::getInstance()),
        shaderManager(ShaderManager::getInstance()),
        renderManager(RenderManager::getInstance()),
        sphereDetails(objectManager.createObject("DebugSphere", "assets/objects/sphere.obj")),
        debugAabbShader(shaderManager.createShaderProgram("DebugAabbShader", "assets/shaders/vertex/debug_aabb.glsl", "assets/shaders/fragment/debug.glsl")),
        debugBoxShader(shaderManager.createShaderProgram("DebugBoxShader", "assets/shaders/vertex/debug_box.glsl", "assets/shaders/fragment/debug.glsl")),
        debugSphereShader(shaderManager.createShaderProgram("DebugSphereShader", "assets/shaders/vertex/debug_sphere.glsl", "assets/shaders/fragment/debug.glsl")),
        debugModelBufferId(createDebugModelBuffer())
  {
  }

public:
  DebugRenderManager(const DebugRenderManager &) = delete;

  ~DebugRenderManager()
  {
    objectManager.destroyObject(sphereDetails);
    shaderManager.destroyShaderProgram(debugAabbShader);
    shaderManager.destroyShaderProgram(debugBoxShader);
    shaderManager.destroyShaderProgram(debugSphereShader);
    glDeleteBuffers(1, &debugModelBufferId);
  }

  std::vector<glm::vec3> getLineVertices(const std::vector<glm::vec3> &boundingBoxVertices) const
  {
    std::vector<glm::vec3> lineVertices({});
    for (unsigned long i = 0; i < boundingBoxVertices.size(); i++)
    {
      for (unsigned long j = i + 1; j < boundingBoxVertices.size(); j++)
      {
        lineVertices.push_back(boundingBoxVertices[i]);
        lineVertices.push_back(boundingBoxVertices[j]);
      }
    }
    return lineVertices;
  }

  void renderLights() const
  {
    const auto activeCamera = renderManager.registeredCameras.at(renderManager.activeCameraId);
    const auto viewMatrix = activeCamera->getViewMatrix();
    const auto projectionMatrix = activeCamera->getProjectionMatrix();

    GLuint shaderId = -1;
    for (auto light = renderManager.registeredLights.begin(); light != renderManager.registeredLights.end(); light++)
    {
      if (shaderId != debugSphereShader->getShaderId())
      {
        shaderId = debugSphereShader->getShaderId();
        glUseProgram(shaderId);
      }

      const auto mvpMatrixId = glGetUniformLocation(debugSphereShader->getShaderId(), "mvpMatrix");
      const auto radiusId = glGetUniformLocation(debugSphereShader->getShaderId(), "radius");
      const auto lineColorId = glGetUniformLocation(debugSphereShader->getShaderId(), "lineColor");

      const auto mvpMatrix = projectionMatrix * viewMatrix * glm::translate(light->second->getLightPosition()) * glm::mat4();
      glUniformMatrix4fv(mvpMatrixId, 1, GL_FALSE, &mvpMatrix[0][0]);
      glUniform1f(radiusId, 0.5);
      glUniform4f(lineColorId, debugColor3.r, debugColor3.g, debugColor3.b, debugColor3.a);

      DebugVertexAttributeArray vertexArray("VertexArray", sphereDetails->getVertexBufferId(), 3);
      vertexArray.enableAttribute();

      glDrawArrays(GL_TRIANGLES, 0, sphereDetails->getBufferSize());
    }
  }

  void renderModels() const
  {
    const auto activeCamera = renderManager.registeredCameras.at(renderManager.activeCameraId);
    const auto viewMatrix = activeCamera->getViewMatrix();
    const auto projectionMatrix = activeCamera->getProjectionMatrix();

    GLuint shaderId = -1;
    for (auto model = renderManager.registeredModels.begin(); model != renderManager.registeredModels.end(); model++)
    {
      if (model->second->getColliderDetails()->getColliderShape()->getType() == ColliderShapeType::SPHERE)
      {
        if (shaderId != debugSphereShader->getShaderId())
        {
          shaderId = debugSphereShader->getShaderId();
          glUseProgram(shaderId);
        }

        const auto mvpMatrixId = glGetUniformLocation(debugSphereShader->getShaderId(), "mvpMatrix");
        const auto radiusId = glGetUniformLocation(debugSphereShader->getShaderId(), "radius");
        const auto lineColorId = glGetUniformLocation(debugSphereShader->getShaderId(), "lineColor");

        const auto mvpMatrix = projectionMatrix * viewMatrix * model->second->getModelMatrix() * glm::mat4();
        glUniformMatrix4fv(mvpMatrixId, 1, GL_FALSE, &mvpMatrix[0][0]);
        glUniform1f(radiusId, std::dynamic_pointer_cast<SphereColliderShape>(model->second->getColliderDetails()->getColliderShape())->getRadius());
        glUniform4f(lineColorId, debugColor1.r, debugColor1.g, debugColor1.b, debugColor1.a);

        DebugVertexAttributeArray vertexArray("VertexArray", sphereDetails->getVertexBufferId(), 3);
        vertexArray.enableAttribute();

        glDrawArrays(GL_TRIANGLES, 0, sphereDetails->getBufferSize());
      }
      else if (model->second->getColliderDetails()->getColliderShape()->getType() == ColliderShapeType::BOX)
      {
        if (shaderId != debugBoxShader->getShaderId())
        {
          shaderId = debugBoxShader->getShaderId();
          glUseProgram(shaderId);
        }

        const auto mvpMatrixId = glGetUniformLocation(debugBoxShader->getShaderId(), "mvpMatrix");
        const auto lineColorId = glGetUniformLocation(debugBoxShader->getShaderId(), "lineColor");

        const auto mvpMatrix = projectionMatrix * viewMatrix * model->second->getModelMatrix() * glm::mat4();
        glUniformMatrix4fv(mvpMatrixId, 1, GL_FALSE, &mvpMatrix[0][0]);
        glUniform4f(lineColorId, debugColor1.r, debugColor1.g, debugColor1.b, debugColor1.a);

        const auto debugModelBuffer = getLineVertices(model->second->getColliderDetails()->getColliderShape()->getBaseBox()->getCorners());
        glBindBuffer(GL_ARRAY_BUFFER, debugModelBufferId);
        glBufferData(GL_ARRAY_BUFFER, debugModelBuffer.size() * sizeof(glm::vec3), &debugModelBuffer[0], GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        DebugVertexAttributeArray vertexArray("VertexArray", debugModelBufferId, 3);
        vertexArray.enableAttribute();

        glDrawArrays(GL_LINES, 0, debugModelBuffer.size());
      }

      {
        if (shaderId != debugBoxShader->getShaderId())
        {
          shaderId = debugBoxShader->getShaderId();
          glUseProgram(shaderId);
        }

        const auto mvpMatrixId = glGetUniformLocation(debugBoxShader->getShaderId(), "mvpMatrix");
        const auto lineColorId = glGetUniformLocation(debugBoxShader->getShaderId(), "lineColor");

        const auto mvpMatrix = projectionMatrix * viewMatrix * model->second->getModelMatrix() * glm::mat4();
        glUniformMatrix4fv(mvpMatrixId, 1, GL_FALSE, &mvpMatrix[0][0]);
        glUniform4f(lineColorId, debugColor2.r, debugColor2.g, debugColor2.b, debugColor2.a);

        DebugVertexAttributeArray vertexArray("VertexArray", model->second->getObjectDetails()->getVertexBufferId(), 3);
        vertexArray.enableAttribute();

        glDrawArrays(GL_TRIANGLES, 0, model->second->getObjectDetails()->getBufferSize());
      }

      {
        if (shaderId != debugAabbShader->getShaderId())
        {
          shaderId = debugAabbShader->getShaderId();
          glUseProgram(shaderId);
        }

        const auto viewMatrixId = glGetUniformLocation(debugAabbShader->getShaderId(), "viewMatrix");
        const auto projectionMatrixId = glGetUniformLocation(debugAabbShader->getShaderId(), "projectionMatrix");
        const auto lineColorId = glGetUniformLocation(debugAabbShader->getShaderId(), "lineColor");

        glUniformMatrix4fv(viewMatrixId, 1, GL_FALSE, &viewMatrix[0][0]);
        glUniformMatrix4fv(projectionMatrixId, 1, GL_FALSE, &projectionMatrix[0][0]);
        glUniform4f(lineColorId, debugColor1.r, debugColor1.g, debugColor1.b, debugColor1.a);

        const auto debugModelBuffer = getLineVertices(model->second->getColliderDetails()->getColliderShape()->getTransformedBox()->getCorners());
        glBindBuffer(GL_ARRAY_BUFFER, debugModelBufferId);
        glBufferData(GL_ARRAY_BUFFER, debugModelBuffer.size() * sizeof(glm::vec3), &debugModelBuffer[0], GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        DebugVertexAttributeArray vertexArray("VertexArray", debugModelBufferId, 3);
        vertexArray.enableAttribute();

        glDrawArrays(GL_LINES, 0, debugModelBuffer.size());
      }
    }
  }

  void render() const
  {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    renderLights();
    renderModels();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  static DebugRenderManager &getInstance()
  {
    return instance;
  }
};

DebugRenderManager DebugRenderManager::instance;
const glm::vec4 DebugRenderManager::debugColor1 = glm::vec4(1.0, 0.0, 0.0, 1.0);
const glm::vec4 DebugRenderManager::debugColor2 = glm::vec4(0.0, 0.0, 1.0, 1.0);
const glm::vec4 DebugRenderManager::debugColor3 = glm::vec4(0.0, 1.0, 0.0, 1.0);

#endif