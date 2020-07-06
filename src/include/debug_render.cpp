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
    std::cout << "Failed at debug render" << std::endl;
    exit(1);
  }

public:
  DebugVertexAttributeArray(DebugVertexAttributeArray &) = delete;

  DebugVertexAttributeArray(std::string attributeName, GLuint bufferId, uint bufferElementSize)
      : attributeName(attributeName),
        bufferId(bufferId),
        bufferElementSize(bufferElementSize)
  {
    attributeId = createAttributeId();
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

  static glm::vec4 debugColor1;
  static glm::vec4 debugColor2;
  static glm::vec4 debugColor3;

  WindowManager &windowManager;
  ObjectManager &objectManager;
  ShaderManager &shaderManager;
  RenderManager &renderManager;

  std::shared_ptr<ObjectDetails> sphereDetails;
  std::shared_ptr<ShaderDetails> debugAabbShader;
  std::shared_ptr<ShaderDetails> debugBoxShader;
  std::shared_ptr<ShaderDetails> debugSphereShader;
  GLuint debugModelBufferId;

  DebugRenderManager()
      : windowManager(WindowManager::getInstance()),
        objectManager(ObjectManager::getInstance()),
        shaderManager(ShaderManager::getInstance()),
        renderManager(RenderManager::getInstance())
  {
    sphereDetails = objectManager.createObject("DebugSphere", "assets/objects/sphere.obj");
    debugAabbShader = shaderManager.createShaderProgram("DebugAabbShader", "assets/shaders/vertex/debug_aabb.glsl", "assets/shaders/fragment/debug.glsl");
    debugBoxShader = shaderManager.createShaderProgram("DebugBoxShader", "assets/shaders/vertex/debug_box.glsl", "assets/shaders/fragment/debug.glsl");
    debugSphereShader = shaderManager.createShaderProgram("DebugSphereShader", "assets/shaders/vertex/debug_sphere.glsl", "assets/shaders/fragment/debug.glsl");
    glGenBuffers(1, &debugModelBufferId);
  }

public:
  DebugRenderManager(DebugRenderManager &) = delete;

  ~DebugRenderManager()
  {
    objectManager.destroyObject(sphereDetails);
    shaderManager.destroyShaderProgram(debugAabbShader);
    shaderManager.destroyShaderProgram(debugBoxShader);
    shaderManager.destroyShaderProgram(debugSphereShader);
    glDeleteBuffers(1, &debugModelBufferId);
  }

  std::vector<glm::vec3> getLineVertices(std::vector<glm::vec3> &boundingBoxVertices)
  {
    std::vector<glm::vec3> lineVertices({});
    for (auto i = 0; i < boundingBoxVertices.size(); i++)
    {
      for (auto j = i + 1; j < boundingBoxVertices.size(); j++)
      {
        lineVertices.push_back(boundingBoxVertices[i]);
        lineVertices.push_back(boundingBoxVertices[j]);
      }
    }
    return lineVertices;
  }

  void renderLights()
  {
    auto currentTime = glfwGetTime();

    auto activeCamera = renderManager.registeredCameras[renderManager.activeCameraId];
    auto viewMatrix = activeCamera->getViewMatrix();
    auto projectionMatrix = activeCamera->getProjectionMatrix();

    GLuint shaderId = -1;
    for (auto light = renderManager.registeredLights.begin(); light != renderManager.registeredLights.end(); light++)
    {
      if (shaderId != debugSphereShader->getShaderId())
      {
        shaderId = debugSphereShader->getShaderId();
        glUseProgram(shaderId);
      }

      auto mvpMatrixId = glGetUniformLocation(debugSphereShader->getShaderId(), "mvpMatrix");
      auto radiusId = glGetUniformLocation(debugSphereShader->getShaderId(), "radius");
      auto fragmentColorId = glGetUniformLocation(debugSphereShader->getShaderId(), "fragmentColor");

      auto mvpMatrix = projectionMatrix * viewMatrix * glm::translate(light->second->getLightPosition()) * glm::mat4();
      glUniformMatrix4fv(mvpMatrixId, 1, GL_FALSE, &mvpMatrix[0][0]);
      glUniform1f(radiusId, 0.5);
      glUniform4f(fragmentColorId, debugColor3.r, debugColor3.g, debugColor3.b, debugColor3.a);

      DebugVertexAttributeArray vertexArray("VertexArray", sphereDetails->getVertexBufferId(), 3);
      vertexArray.enableAttribute();

      glDrawArrays(GL_TRIANGLES, 0, sphereDetails->getBufferSize());
    }
  }

  void renderModels()
  {
    auto currentTime = glfwGetTime();

    auto activeCamera = renderManager.registeredCameras[renderManager.activeCameraId];
    auto viewMatrix = activeCamera->getViewMatrix();
    auto projectionMatrix = activeCamera->getProjectionMatrix();

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

        auto mvpMatrixId = glGetUniformLocation(debugSphereShader->getShaderId(), "mvpMatrix");
        auto radiusId = glGetUniformLocation(debugSphereShader->getShaderId(), "radius");
        auto fragmentColorId = glGetUniformLocation(debugSphereShader->getShaderId(), "fragmentColor");

        auto mvpMatrix = projectionMatrix * viewMatrix * model->second->getModelMatrix() * glm::mat4();
        glUniformMatrix4fv(mvpMatrixId, 1, GL_FALSE, &mvpMatrix[0][0]);
        glUniform1f(radiusId, std::dynamic_pointer_cast<SphereColliderShape>(model->second->getColliderDetails()->getColliderShape())->getRadius());
        glUniform4f(fragmentColorId, debugColor1.r, debugColor1.g, debugColor1.b, debugColor1.a);

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

        auto mvpMatrixId = glGetUniformLocation(debugBoxShader->getShaderId(), "mvpMatrix");
        auto fragmentColorId = glGetUniformLocation(debugBoxShader->getShaderId(), "fragmentColor");

        auto mvpMatrix = projectionMatrix * viewMatrix * model->second->getModelMatrix() * glm::mat4();
        glUniformMatrix4fv(mvpMatrixId, 1, GL_FALSE, &mvpMatrix[0][0]);
        glUniform4f(fragmentColorId, debugColor1.r, debugColor1.g, debugColor1.b, debugColor1.a);

        auto debugModelBuffer = getLineVertices(model->second->getColliderDetails()->getColliderShape()->getBaseBox()->getCorners());
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

        auto mvpMatrixId = glGetUniformLocation(debugBoxShader->getShaderId(), "mvpMatrix");
        auto fragmentColorId = glGetUniformLocation(debugBoxShader->getShaderId(), "fragmentColor");

        auto mvpMatrix = projectionMatrix * viewMatrix * model->second->getModelMatrix() * glm::mat4();
        glUniformMatrix4fv(mvpMatrixId, 1, GL_FALSE, &mvpMatrix[0][0]);
        glUniform4f(fragmentColorId, debugColor2.r, debugColor2.g, debugColor2.b, debugColor2.a);

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

        auto viewMatrixId = glGetUniformLocation(debugAabbShader->getShaderId(), "viewMatrix");
        auto projectionMatrixId = glGetUniformLocation(debugAabbShader->getShaderId(), "projectionMatrix");
        auto fragmentColorId = glGetUniformLocation(debugAabbShader->getShaderId(), "fragmentColor");

        glUniformMatrix4fv(viewMatrixId, 1, GL_FALSE, &viewMatrix[0][0]);
        glUniformMatrix4fv(projectionMatrixId, 1, GL_FALSE, &projectionMatrix[0][0]);
        glUniform4f(fragmentColorId, debugColor1.r, debugColor1.g, debugColor1.b, debugColor1.a);

        auto debugModelBuffer = getLineVertices(model->second->getColliderDetails()->getColliderShape()->getTransformedBox()->getCorners());
        glBindBuffer(GL_ARRAY_BUFFER, debugModelBufferId);
        glBufferData(GL_ARRAY_BUFFER, debugModelBuffer.size() * sizeof(glm::vec3), &debugModelBuffer[0], GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        DebugVertexAttributeArray vertexArray("VertexArray", debugModelBufferId, 3);
        vertexArray.enableAttribute();

        glDrawArrays(GL_LINES, 0, debugModelBuffer.size());
      }
    }
  }

  void render()
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
glm::vec4 DebugRenderManager::debugColor1 = glm::vec4(1.0, 0.0, 0.0, 1.0);
glm::vec4 DebugRenderManager::debugColor2 = glm::vec4(0.0, 0.0, 1.0, 1.0);
glm::vec4 DebugRenderManager::debugColor3 = glm::vec4(0.0, 1.0, 0.0, 1.0);

#endif