#ifndef INCLUDE_RENDER_CPP
#define INCLUDE_RENDER_CPP

#include <map>
#include <set>

#include <GL/glew.h>

#include "window.cpp"
#include "control.cpp"
#include "constants.cpp"
#include "../light/light_base.cpp"
#include "../camera/camera_base.cpp"
#include "../models/model_base.cpp"

/**
 * Structure for defining details of the light.
 */
struct LightDetails
{
  // The position of the light
  glm::vec3 lightPosition;
  // The projection-view matrix of the light.
  glm::mat4 lightVpMatrix;
  // The color of the light.
  glm::vec3 lightColor;
  // The intensity of the light.
  double lightIntensity;
  // The width of the shadowmap of the light.
  int mapWidth;
  // The height of the shadowmap of the light.
  int mapHeight;
  // The closest distance from which the shadowmap captures objects.
  double nearPlane;
  // The farthest distance till which the shadowmap captures objects.
  double farPlane;
  // The ID of the shadowmap texture.
  GLuint textureId;
};

/**
 * Class for storing the vertex attribute array information.
 */
class VertexAttributeArray
{
private:
  // The set of attribute IDs already in use.
  static std::set<GLuint> attributeIds;

  // The ID of the attribute.
  GLuint attributeId;
  // The name of the attribute.
  std::string attributeName;
  // The ID of the buffer the attribute is linked to.
  GLuint bufferId;
  // The size of the elements in the buffer.
  unsigned int bufferElementSize;

  /**
   * Creates a new attribute ID for the current attribute to use.
   * 
   * @return The attribute ID.
   */
  GLuint createAttributeId()
  {
    // Get the maximum possible attribute ID that can be used.
    GLuint maxId = std::numeric_limits<GLuint>::max();
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
    std::cout << "Failed at render" << std::endl;
    exit(1);
  }

public:
  // Preventing copying the vertex attribute array, making sure only one instance can exist.
  VertexAttributeArray(VertexAttributeArray &) = delete;

  VertexAttributeArray(std::string attributeName, GLuint bufferId, unsigned int bufferElementSize)
      : attributeName(attributeName),
        bufferId(bufferId),
        bufferElementSize(bufferElementSize)
  {
    // Create a new ID for the attribute.
    attributeId = createAttributeId();
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
    glVertexAttribPointer(attributeId, bufferElementSize, GL_FLOAT, GL_FALSE, 0, (void *)0);
  }
};

// Initialize the set of used attribute IDs to an empty set.
std::set<GLuint> VertexAttributeArray::attributeIds = std::set<GLuint>({});

/**
 * A manager class for managing rendering of models.
 */
class RenderManager
{
  // Let the debug renderer access private variables.
  friend class DebugRenderManager;

private:
  // The ambient lighting factor of the scene.
  static double ambientFactor;

  // The maximum number of simple and cube lights allowed.
  static unsigned long maxSimpleLights;
  static unsigned long maxCubeLights;

  // The flags for disabling shadows and lighting.
  static int DISABLE_SHADOW;
  static int DISABLE_LIGHT;

  // Singleton instance of the render manager.
  static RenderManager instance;

  // The window manager responsible for the window.
  WindowManager &windowManager;
  // The control manager responsible for managing controls and inputs of the window.
  ControlManager &controlManager;

  // The ID of the active camera to use to render the scene to the window.
  std::string activeCameraId;
  // A dead version of simple (2D texture) light (required due to a OpenGL bug on certain GPUs/drivers).
  std::shared_ptr<LightBase> deadSimpleLight;
  // A dead version of cube (cubemap texture) light (required due to a OpenGL bug on certain GPUs/drivers).
  std::shared_ptr<LightBase> deadCubeLight;
  // The map of registered lights.
  std::map<std::string, std::shared_ptr<LightBase>> registeredLights;
  // The map of registered models.
  std::map<std::string, std::shared_ptr<ModelBase>> registeredModels;
  // The map of registered cameras.
  std::map<std::string, std::shared_ptr<CameraBase>> registeredCameras;

  // The timestamp when the render manager was loaded.
  double startTime;
  // The timestamp of the start of the last render.
  double lastTime;

  // A mask defining what features to disable (shadows/lighting).
  int disableFeatureMask;
  // The timestamp of the last time the mask for disabling features was modifed.
  double lastDisableFeatureMaskChange;

  RenderManager()
      : windowManager(WindowManager::getInstance()),
        controlManager(ControlManager::getInstance()),
        registeredLights({}),
        registeredModels({}),
        registeredCameras({}),
        startTime(glfwGetTime()),
        lastTime(glfwGetTime()),
        disableFeatureMask(0),
        lastDisableFeatureMaskChange(glfwGetTime() - 10) {}

public:
  // Preventing copying the render manager, making sure only one instance can exist.
  RenderManager(RenderManager &) = delete;

  /**
   * Register a dead simple light into the light manager.
   * 
   * @param light  The light to register.
   */
  void registerDeadSimpleLight(std::shared_ptr<LightBase> light)
  {
    deadSimpleLight = light;
  }

  /**
   * Register a dead cube light into the light manager.
   * 
   * @param light  The light to register.
   */
  void registerDeadCubeLight(std::shared_ptr<LightBase> light)
  {
    deadCubeLight = light;
  }

  /**
   * Register a new light into the light manager.
   * 
   * @param light  The light to register.
   */
  void registerLight(std::shared_ptr<LightBase> light)
  {
    registeredLights.insert(std::pair<std::string, std::shared_ptr<LightBase>>(light->getLightId(), light));
  }

  /**
   * De-register an existing light from the light manager.
   * 
   * @param lightId  The ID of the light to de-register.
   */
  void deregisterLight(std::string lightId)
  {
    registeredLights.erase(lightId);
  }

  /**
   * De-register an existing light from the light manager.
   * 
   * @param light  The light to de-register.
   */
  void deregisterLight(std::shared_ptr<LightBase> light)
  {
    registeredLights.erase(light->getLightId());
  }

  /**
   * Register a new model into the model manager.
   * 
   * @param model  The model to register.
   */
  void registerModel(std::shared_ptr<ModelBase> model)
  {
    registeredModels.insert(std::pair<std::string, std::shared_ptr<ModelBase>>(model->getModelId(), model));
  }

  /**
   * De-register an existing model from the model manager.
   * 
   * @param model  The ID of the model to de-register.
   */
  void deregisterModel(std::string modelId)
  {
    registeredModels.erase(modelId);
  }

  /**
   * De-register an existing model from the model manager.
   * 
   * @param model  The model to de-register.
   */
  void deregisterModel(std::shared_ptr<ModelBase> model)
  {
    registeredModels.erase(model->getModelId());
  }

  /**
   * Register a new camera into the camera manager.
   * 
   * @param camera  The camera to register.
   */
  void registerCamera(std::shared_ptr<CameraBase> camera)
  {
    registeredCameras.insert(std::pair<std::string, std::shared_ptr<CameraBase>>(camera->getCameraId(), camera));
  }

  /**
   * De-register an existing camera from the camera manager.
   * 
   * @param cameraId  The ID of the camera to de-register.
   */
  void deregisterCamera(std::string cameraId)
  {
    registeredCameras.erase(cameraId);
  }

  /**
   * De-register an existing camera from the camera manager.
   * 
   * @param camera  The camera to de-register.
   */
  void deregisterCamera(std::shared_ptr<CameraBase> camera)
  {
    registeredCameras.erase(camera->getCameraId());
  }

  /**
   * Registers a camera to be used as the active camera.
   * 
   * @param cameraId  The ID of the camera to set as the active camera.
   */
  void registerActiveCamera(std::string cameraId)
  {
    activeCameraId = cameraId;
  }

  /**
   * Registers a camera to be used as the active camera.
   * 
   * @param camera  The camera to set as the active camera.
   */
  void registerActiveCamera(std::shared_ptr<CameraBase> camera)
  {
    activeCameraId = camera->getCameraId();
  }

  /**
   * Render the shadow maps for all the lights in the scene, and return the map of lights categorized by their shadow map type.
   * 
   * @return The map of the lights in the scene categorized by their shadow map type.
   */
  std::map<ShadowBufferType, std::vector<LightDetails>> renderLights()
  {
    // Switch the viewport to the size of the render framebuffers.
    windowManager.switchToFrameBufferViewport();
    // Set the clear framebuffer color to pure white.
    windowManager.setClearColor(glm::vec4(1.0, 1.0, 1.0, 1.0));

    // Create a map of the categorized lights.
    std::map<ShadowBufferType, std::vector<LightDetails>> categorizedLights({{ShadowBufferType::SIMPLE, {}}, {ShadowBufferType::CUBE, {}}});

    // Set the current active shader ID to 0.
    GLuint currentShaderId = 0;

    // Iterate through all the lights in the scene.
    for (auto light = registeredLights.begin(); light != registeredLights.end(); light++)
    {
      // Get the type of the shadow.
      auto shadowType = light->second->getShadowBufferDetails()->getShadowBufferType();
      // Generate a structure detailing information about the light.
      LightDetails lightDetails = {
          light->second->getLightPosition(),
          light->second->getProjectionMatrices()[0] * light->second->getViewMatrices()[0] * glm::mat4(),
          light->second->getLightColor(),
          light->second->getLightIntensity(),
          FRAMEBUFFER_WIDTH,
          FRAMEBUFFER_WIDTH,
          light->second->getLightNearPlane(),
          light->second->getLightFarPlane(),
          light->second->getShadowBufferDetails()->getShadowBufferTextureId()};
      // Store the light details in the categorized map.
      categorizedLights[shadowType].push_back(lightDetails);

      // If shadows are disabled, skip the shadowmap render step.
      if (disableFeatureMask >= DISABLE_SHADOW)
      {
        continue;
      }

      // Bind the shadowmap framebuffer of the light as the active framebuffer.
      glBindFramebuffer(GL_FRAMEBUFFER, light->second->getShadowBufferDetails()->getShadowBufferId());

      // Clear the color buffer and depth buffer of the screen.
      windowManager.clearScreen(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // Check if the shader of the light is the same as the currently used shader.
      if (currentShaderId != light->second->getShaderDetails()->getShaderId())
      {
        // If not, set it as the currently used shader and use it.
        currentShaderId = light->second->getShaderDetails()->getShaderId();
        glUseProgram(currentShaderId);
      }

      // Get the view and projection matrices of the light.
      auto viewMatrices = light->second->getViewMatrices();
      auto projectionMatrices = light->second->getProjectionMatrices();

      // Get the uniform ID of the count of the projection-view matrix variable and set it.
      auto vpMatrixCountVertexId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), "lightDetails_vertex.vpMatrixCount");
      glUniform1i(vpMatrixCountVertexId, viewMatrices.size());
      auto vpMatrixCountGeometryId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), "lightDetails_geometry.vpMatrixCount");
      glUniform1i(vpMatrixCountGeometryId, viewMatrices.size());
      auto vpMatrixCountFragmentId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), "lightDetails_fragment.vpMatrixCount");
      glUniform1i(vpMatrixCountFragmentId, viewMatrices.size());

      // Get the uniform ID of the light position variable and set it.
      auto lightPositionVertexId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), "lightDetails_vertex.lightPosition");
      glUniform3f(lightPositionVertexId, lightDetails.lightPosition.x, lightDetails.lightPosition.y, lightDetails.lightPosition.z);
      auto lightPositionGeometryId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), "lightDetails_geometry.lightPosition");
      glUniform3f(lightPositionGeometryId, lightDetails.lightPosition.x, lightDetails.lightPosition.y, lightDetails.lightPosition.z);
      auto lightPositionFragmentId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), "lightDetails_fragment.lightPosition");
      glUniform3f(lightPositionFragmentId, lightDetails.lightPosition.x, lightDetails.lightPosition.y, lightDetails.lightPosition.z);

      // Get the uniform ID of the near plane of the light variable and set it.
      auto nearPlaneVertexId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), "projectionDetails_vertex.nearPlane");
      glUniform1f(nearPlaneVertexId, lightDetails.nearPlane);
      auto nearPlaneGeometryId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), "projectionDetails_geometry.nearPlane");
      glUniform1f(nearPlaneGeometryId, lightDetails.nearPlane);
      auto nearPlaneFragmentId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), "projectionDetails_fragment.nearPlane");
      glUniform1f(nearPlaneFragmentId, lightDetails.nearPlane);

      // Get the uniform ID of the far plane of the light variable and set it.
      auto farPlaneVertexId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), "projectionDetails_vertex.farPlane");
      glUniform1f(farPlaneVertexId, lightDetails.farPlane);
      auto farPlaneGeometryId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), "projectionDetails_geometry.farPlane");
      glUniform1f(farPlaneGeometryId, lightDetails.farPlane);
      auto farPlaneFragmentId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), "projectionDetails_fragment.farPlane");
      glUniform1f(farPlaneFragmentId, lightDetails.farPlane);

      // Iterate through the view matrices of the light.
      for (unsigned long i = 0; i < viewMatrices.size(); i++)
      {
        // Calculate the projection-view matrix.
        auto vpMatrix = projectionMatrices[i] * viewMatrices[i];
        // Get the uniform ID of the projection-view matrix of the light variable and set it.
        auto vpMatrixVertexId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), ("lightDetails_vertex.vpMatrices[" + std::to_string(i) + "]").c_str());
        glUniformMatrix4fv(vpMatrixVertexId, 1, GL_FALSE, &vpMatrix[0][0]);
        auto vpMatrixGeometryId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), ("lightDetails_geometry.vpMatrices[" + std::to_string(i) + "]").c_str());
        glUniformMatrix4fv(vpMatrixGeometryId, 1, GL_FALSE, &vpMatrix[0][0]);
        auto vpMatrixFragmentId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), ("lightDetails_fragment.vpMatrices[" + std::to_string(i) + "]").c_str());
        glUniformMatrix4fv(vpMatrixFragmentId, 1, GL_FALSE, &vpMatrix[0][0]);
      }

      // Iterate through the models in the scene.
      for (auto model = registeredModels.begin(); model != registeredModels.end(); model++)
      {
        // Get the model matrix of the model.
        auto modelMatrix = model->second->getModelMatrix();
        // Get the uniform ID of the model matrix variable and set it.
        auto modelMatrixId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), "modelMatrix");
        glUniformMatrix4fv(modelMatrixId, 1, GL_FALSE, &modelMatrix[0][0]);

        // Define a vertex attribute array that contains the vertex position data of the model.
        VertexAttributeArray vertexArray("VertexArray", model->second->getObjectDetails()->getVertexBufferId(), 3);

        // Enable it so that it can be used by the GPU.
        vertexArray.enableAttribute();

        // Draw the triangles of the model.
        glDrawArrays(GL_TRIANGLES, 0, model->second->getObjectDetails()->getBufferSize());
      }

      // Bind the window framebuffer as the active framebuffer.
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Return the map of the categorized lights.
    return categorizedLights;
  }

  /**
   * Render the shadow maps for all the models in the scene.
   * 
   * @param categorizedLights  The categorized map of lights in the scene.
   */
  void renderModels(std::map<ShadowBufferType, std::vector<LightDetails>> categorizedLights, double currentTime)
  {
    // Switch the viewport to the size of the window viewport.
    windowManager.switchToWindowViewport();
    // Set the clear screen color to pure white.
    windowManager.setClearColor(glm::vec4(0.0, 0.0, 0.0, 1.0));

    // Clear the color buffer and depth buffer of the screen.
    windowManager.clearScreen(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set the current active shader ID to 0.
    GLuint currentShaderId = 0;
    // Get the active camera to use to render the video.
    auto activeCamera = registeredCameras[activeCameraId];
    // Get the view matrix of the camera.
    auto viewMatrix = activeCamera->getViewMatrix();
    // Get the projection matrix of the camera.
    auto projectionMatrix = activeCamera->getProjectionMatrix();
    // Iterate through all the models in the scene.
    for (auto model = registeredModels.begin(); model != registeredModels.end(); model++)
    {
      // Check if the shader of the light is the same as the currently used shader.
      if (currentShaderId != model->second->getShaderDetails()->getShaderId())
      {
        // If not, set it as the currently used shader and use it.
        currentShaderId = model->second->getShaderDetails()->getShaderId();
        glUseProgram(currentShaderId);
      }

      // Get the model matrix of the model.
      auto modelMatrix = model->second->getModelMatrix();
      // Get the uniform ID of the model matrix variable and set it.
      auto modelMatrixId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), "modelDetails.modelMatrix");
      glUniformMatrix4fv(modelMatrixId, 1, GL_FALSE, &modelMatrix[0][0]);

      // Get the uniform ID of the diffuse texture of the model variable and set it.
      auto diffuseTextureId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), "diffuseTexture");
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, model->second->getTextureDetails()->getTextureId());
      glUniform1i(diffuseTextureId, 0);

      // Get the uniform ID of the view matrix of the camera variable and set it.
      auto viewMatrixId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), "modelDetails.viewMatrix");
      glUniformMatrix4fv(viewMatrixId, 1, GL_FALSE, &viewMatrix[0][0]);
      // Get the uniform ID of the projection matrix of the camera variable and set it.
      auto projectionMatrixId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), "modelDetails.projectionMatrix");
      glUniformMatrix4fv(projectionMatrixId, 1, GL_FALSE, &projectionMatrix[0][0]);

      // Get the uniform ID of the disable feature mask variable and set it.
      auto disableFeatureMaskId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), "disableFeatureMask");
      glUniform1i(disableFeatureMaskId, disableFeatureMask);
      // Get the uniform ID of the ambient lighting factor variable and set it.
      auto ambientFactorId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), "ambientFactor");
      glUniform1f(ambientFactorId, ambientFactor);
      // Get the uniform ID of the simple lights count in the scene and set it.
      auto simpleLightsCountId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), "simpleLightsCount");
      glUniform1i(simpleLightsCountId, categorizedLights[ShadowBufferType::SIMPLE].size());
      // Get the uniform ID of the cube lights count in the scene and set it.
      auto cubeLightsCountId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), "cubeLightsCount");
      glUniform1i(cubeLightsCountId, categorizedLights[ShadowBufferType::CUBE].size());

      // If lighting is not disabled, then setup the lighting information.
      if (disableFeatureMask < DISABLE_LIGHT)
      {
        // Iterate through the simple lights in the scene.
        for (unsigned long i = 0; i < categorizedLights[ShadowBufferType::SIMPLE].size(); i++)
        {
          // Get the details of the simple light.
          auto lightDetails = categorizedLights[ShadowBufferType::SIMPLE][i];

          // Get the uniform ID of the light position variable and set it.
          auto lightPositionVertexId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("simpleLightDetails_vertex[" + std::to_string(i) + "].lightPosition").c_str());
          glUniform3f(lightPositionVertexId, lightDetails.lightPosition.x, lightDetails.lightPosition.y, lightDetails.lightPosition.z);
          auto lightPositionFragmentId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("simpleLightDetails_fragment[" + std::to_string(i) + "].lightPosition").c_str());
          glUniform3f(lightPositionFragmentId, lightDetails.lightPosition.x, lightDetails.lightPosition.y, lightDetails.lightPosition.z);

          // Get the uniform ID of the count of the projection-view matrix variable and set it.
          auto lightVpMatrixVertexId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("simpleLightDetails_vertex[" + std::to_string(i) + "].lightVpMatrix").c_str());
          glUniformMatrix4fv(lightVpMatrixVertexId, 1, GL_FALSE, &lightDetails.lightVpMatrix[0][0]);
          auto lightVpMatrixFragmentId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("simpleLightDetails_fragment[" + std::to_string(i) + "].lightVpMatrix").c_str());
          glUniformMatrix4fv(lightVpMatrixFragmentId, 1, GL_FALSE, &lightDetails.lightVpMatrix[0][0]);

          // Get the uniform ID of the light color variable and set it.
          auto lightColorVertexId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("simpleLightDetails_vertex[" + std::to_string(i) + "].lightColor").c_str());
          glUniform3f(lightColorVertexId, lightDetails.lightColor.r, lightDetails.lightColor.g, lightDetails.lightColor.b);
          auto lightColorFragmentId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("simpleLightDetails_fragment[" + std::to_string(i) + "].lightColor").c_str());
          glUniform3f(lightColorFragmentId, lightDetails.lightColor.r, lightDetails.lightColor.g, lightDetails.lightColor.b);

          // Get the uniform ID of the light intensity variable and set it.
          auto lightIntensityVertexId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("simpleLightDetails_vertex[" + std::to_string(i) + "].lightIntensity").c_str());
          glUniform1f(lightIntensityVertexId, lightDetails.lightIntensity);
          auto lightIntensityFragmentId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("simpleLightDetails_fragment[" + std::to_string(i) + "].lightIntensity").c_str());
          glUniform1f(lightIntensityFragmentId, lightDetails.lightIntensity);

          // Get the uniform ID of the near plane of the light variable and set it.
          auto lightNearPlaneVertexId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("simpleLightDetails_vertex[" + std::to_string(i) + "].nearPlane").c_str());
          glUniform1f(lightNearPlaneVertexId, lightDetails.nearPlane);
          auto lightNearPlaneFragmentId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("simpleLightDetails_fragment[" + std::to_string(i) + "].nearPlane").c_str());
          glUniform1f(lightNearPlaneFragmentId, lightDetails.nearPlane);

          // Get the uniform ID of the far plane of the light variable and set it.
          auto lightFarPlaneVertexId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("simpleLightDetails_vertex[" + std::to_string(i) + "].farPlane").c_str());
          glUniform1f(lightFarPlaneVertexId, lightDetails.farPlane);
          auto lightFarPlaneFragmentId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("simpleLightDetails_fragment[" + std::to_string(i) + "].farPlane").c_str());
          glUniform1f(lightFarPlaneFragmentId, lightDetails.farPlane);

          // Get the uniform ID of the shadowmap texture of the light variable and set it.
          auto lightTextureId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("simpleLightTextures[" + std::to_string(i) + "]").c_str());
          glActiveTexture(GL_TEXTURE0 + (i + 1));
          glBindTexture(GL_TEXTURE_2D, lightDetails.textureId);
          glUniform1i(lightTextureId, i + 1);
        }
        // Iterate through the unassinged light texture variables.
        for (auto i = categorizedLights[ShadowBufferType::SIMPLE].size(); i < maxSimpleLights; i++)
        {
          // Get the uniform ID of the shadowmap texture of the light variable and set it to the dead light.
          auto lightTextureId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("simpleLightTextures[" + std::to_string(i) + "]").c_str());
          glActiveTexture(GL_TEXTURE0 + (i + 1));
          glBindTexture(GL_TEXTURE_2D, deadSimpleLight->getShadowBufferDetails()->getShadowBufferTextureId());
          glUniform1i(lightTextureId, i + 1);
        }

        // Iterate through the cube lights in the scene.
        for (unsigned long i = 0; i < categorizedLights[ShadowBufferType::CUBE].size(); i++)
        {
          // Get the details of the cube light.
          auto lightDetails = categorizedLights[ShadowBufferType::CUBE][i];

          // Get the uniform ID of the light position variable and set it.
          auto lightPositionVertexId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("cubeLightDetails_vertex[" + std::to_string(i) + "].lightPosition").c_str());
          glUniform3f(lightPositionVertexId, lightDetails.lightPosition.x, lightDetails.lightPosition.y, lightDetails.lightPosition.z);
          auto lightPositionFragmentId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("cubeLightDetails_fragment[" + std::to_string(i) + "].lightPosition").c_str());
          glUniform3f(lightPositionFragmentId, lightDetails.lightPosition.x, lightDetails.lightPosition.y, lightDetails.lightPosition.z);

          // Get the uniform ID of the count of the projection-view matrix variable and set it.
          auto lightVpMatrixVertexId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("cubeLightDetails_vertex[" + std::to_string(i) + "].lightVpMatrix").c_str());
          glUniformMatrix4fv(lightVpMatrixVertexId, 1, GL_FALSE, &lightDetails.lightVpMatrix[0][0]);
          auto lightVpMatrixFragmentId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("cubeLightDetails_fragment[" + std::to_string(i) + "].lightVpMatrix").c_str());
          glUniformMatrix4fv(lightVpMatrixFragmentId, 1, GL_FALSE, &lightDetails.lightVpMatrix[0][0]);

          // Get the uniform ID of the light color variable and set it.
          auto lightColorVertexId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("cubeLightDetails_vertex[" + std::to_string(i) + "].lightColor").c_str());
          glUniform3f(lightColorVertexId, lightDetails.lightColor.r, lightDetails.lightColor.g, lightDetails.lightColor.b);
          auto lightColorFragmentId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("cubeLightDetails_fragment[" + std::to_string(i) + "].lightColor").c_str());
          glUniform3f(lightColorFragmentId, lightDetails.lightColor.r, lightDetails.lightColor.g, lightDetails.lightColor.b);

          // Get the uniform ID of the light intensity variable and set it.
          auto lightIntensityVertexId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("cubeLightDetails_vertex[" + std::to_string(i) + "].lightIntensity").c_str());
          glUniform1f(lightIntensityVertexId, lightDetails.lightIntensity);
          auto lightIntensityFragmentId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("cubeLightDetails_fragment[" + std::to_string(i) + "].lightIntensity").c_str());
          glUniform1f(lightIntensityFragmentId, lightDetails.lightIntensity);

          // Get the uniform ID of the near plane of the light variable and set it.
          auto lightNearPlaneVertexId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("cubeLightDetails_vertex[" + std::to_string(i) + "].nearPlane").c_str());
          glUniform1f(lightNearPlaneVertexId, lightDetails.nearPlane);
          auto lightNearPlaneFragmentId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("cubeLightDetails_fragment[" + std::to_string(i) + "].nearPlane").c_str());
          glUniform1f(lightNearPlaneFragmentId, lightDetails.nearPlane);

          // Get the uniform ID of the far plane of the light variable and set it.
          auto lightFarPlaneVertexId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("cubeLightDetails_vertex[" + std::to_string(i) + "].farPlane").c_str());
          glUniform1f(lightFarPlaneVertexId, lightDetails.farPlane);
          auto lightFarPlaneFragmentId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("cubeLightDetails_fragment[" + std::to_string(i) + "].farPlane").c_str());
          glUniform1f(lightFarPlaneFragmentId, lightDetails.farPlane);

          // Get the uniform ID of the shadowmap texture of the light variable and set it.
          auto lightTextureId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("cubeLightTextures[" + std::to_string(i) + "]").c_str());
          glActiveTexture(GL_TEXTURE0 + (i + 1 + maxSimpleLights));
          glBindTexture(GL_TEXTURE_CUBE_MAP, lightDetails.textureId);
          glUniform1i(lightTextureId, i + 1 + maxSimpleLights);
        }
        // Iterate through the unassinged light texture variables.
        for (auto i = categorizedLights[ShadowBufferType::CUBE].size(); i < maxCubeLights; i++)
        {
          // Get the uniform ID of the shadowmap texture of the light variable and set it to the dead light.
          auto lightTextureId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("cubeLightTextures[" + std::to_string(i) + "]").c_str());
          glActiveTexture(GL_TEXTURE0 + (i + 1 + maxSimpleLights));
          glBindTexture(GL_TEXTURE_CUBE_MAP, deadCubeLight->getShadowBufferDetails()->getShadowBufferTextureId());
          glUniform1i(lightTextureId, i + 1 + maxSimpleLights);
        }
      }

      // Define vertex attribute arrays that contains the vertex position, UV coordinates, and normal vector data of the model.
      VertexAttributeArray vertexArray("VertexArray", model->second->getObjectDetails()->getVertexBufferId(), 3);
      VertexAttributeArray uvArray("UvArray", model->second->getObjectDetails()->getUvBufferId(), 2);
      VertexAttributeArray normalArray("NormalArray", model->second->getObjectDetails()->getNormalBufferId(), 3);

      // Enable them so that it can be used by the GPU.
      vertexArray.enableAttribute();
      uvArray.enableAttribute();
      normalArray.enableAttribute();

      // Draw the triangles of the model.
      glDrawArrays(GL_TRIANGLES, 0, model->second->getObjectDetails()->getBufferSize());
    }

    // Update the last start time of the latest rendered frame to the start time of the current frame.
    lastTime = currentTime;
  }

  /**
   * Render the scene with the light shadowmaps and the models.
   */
  void render()
  {
    // Get the time at the start of the frame.
    auto currentTime = glfwGetTime();

    // Check if the "L" has been pressed 500ms after the last time the disable feature mask was changed.
    if (controlManager.isKeyPressed(GLFW_KEY_L) && (currentTime - lastDisableFeatureMaskChange) > 0.5)
    {
      // "L" was pressed, meaning we need to start disabling render features.
      // Check which features have already been disabled.
      switch (disableFeatureMask)
      {
      case 0:
        // No features were disabled. Start disabling shadows.
        disableFeatureMask = DISABLE_SHADOW;
        break;
      case 1:
        // Shadows were disabled. Start disabling lighting.
        disableFeatureMask = DISABLE_LIGHT;
        break;
      case 2:
      default:
        // All features were disabled. Enable everything.
        disableFeatureMask = 0;
      }
      // Update the timestamp for when the disable feature mask was changed
      lastDisableFeatureMaskChange = currentTime;
    }

    // Render the light shadowmaps.
    auto categorizedLights = renderLights();
    // Render the models.
    renderModels(categorizedLights, currentTime);
  }

  /**
   * Returns the singleton instance of the render manager.
   * 
   * @return The render manager singleton instance.
   */
  static RenderManager &getInstance()
  {
    return instance;
  }
};

// Initialize the render manager singleton instance static variable.
RenderManager RenderManager::instance;
// Initialize the ambient lighting factor static variable.
double RenderManager::ambientFactor = 0.25;
// Initialize the maximum supported simple lights static variable.
unsigned long RenderManager::maxSimpleLights = 2;
// Initialize the maximum supported cube lights static variable.
unsigned long RenderManager::maxCubeLights = 8;
// Initialize the mask value for disabling shadows static variable.
int RenderManager::DISABLE_SHADOW = 1;
// Initialize the mask value for disabling lighting static variable.
int RenderManager::DISABLE_LIGHT = 2;

#endif