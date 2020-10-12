#ifndef INCLUDE_RENDER_CPP
#define INCLUDE_RENDER_CPP

#include <map>
#include <set>

#include <GL/glew.h>

#include "constants.cpp"
#include "window.cpp"
#include "control.cpp"
#include "shadowbuffer.cpp"
#include "../light/light_base.cpp"
#include "../camera/camera_base.cpp"
#include "../models/model_base.cpp"

/**
 * Structure for defining details of the light.
 */
struct LightDetails
{
  // The position of the light
  const glm::vec3 lightPosition;
  // The projection-view matrix of the light.
  const glm::mat4 lightVpMatrix;
  // The color of the light.
  const glm::vec3 lightColor;
  // The intensity of the light.
  const double lightIntensity;
  // The width of the shadowmap of the light.
  const int mapWidth;
  // The height of the shadowmap of the light.
  const int mapHeight;
  // The closest distance from which the shadowmap captures objects.
  const double nearPlane;
  // The farthest distance till which the shadowmap captures objects.
  const double farPlane;
  // The ID of the layer of the shadowmap texture array the shadowmap is stored in.
  const GLuint textureArrayLayerId;
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
  const GLuint attributeId;
  // The name of the attribute.
  const std::string attributeName;
  // The ID of the buffer the attribute is linked to.
  const GLuint bufferId;
  // The size of the elements in the buffer.
  const unsigned int bufferElementSize;

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
    std::cout << "Failed at render" << std::endl;
    exit(1);
  }

public:
  // Preventing copying the vertex attribute array, making sure only one instance can exist.
  VertexAttributeArray(const VertexAttributeArray &) = delete;

  VertexAttributeArray(const std::string &attributeName, const GLuint &bufferId, const unsigned int &bufferElementSize)
      : attributeId(createAttributeId()),
        attributeName(attributeName),
        bufferId(bufferId),
        bufferElementSize(bufferElementSize)
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
  const static double ambientFactor;

  // The flags for disabling shadows and lighting.
  const static int DISABLE_SHADOW;
  const static int DISABLE_LIGHT;

  // Singleton instance of the render manager.
  static RenderManager instance;

  // The window manager responsible for the window.
  WindowManager &windowManager;
  // The control manager responsible for managing controls and inputs of the window.
  const ControlManager &controlManager;
  // The shadow buffer manager responsible for creating shadow buffers for lights.
  const ShadowBufferManager &shadowBufferManager;

  // The ID of the active camera to use to render the scene to the window.
  std::string activeCameraId;
  // The map of registered lights.
  std::map<const std::string, const std::shared_ptr<const LightBase>> registeredLights;
  // The map of registered models.
  std::map<const std::string, const std::shared_ptr<const ModelBase>> registeredModels;
  // The map of registered cameras.
  std::map<const std::string, const std::shared_ptr<const CameraBase>> registeredCameras;

  // The timestamp when the render manager was loaded.
  const double startTime;
  // The timestamp of the start of the last render.
  double lastTime;

  // A mask defining what features to disable (shadows/lighting).
  int disableFeatureMask;
  // The timestamp of the last time the mask for disabling features was modifed.
  double lastDisableFeatureMaskChange;

  RenderManager()
      : windowManager(WindowManager::getInstance()),
        controlManager(ControlManager::getInstance()),
        shadowBufferManager(ShadowBufferManager::getInstance()),
        registeredLights({}),
        registeredModels({}),
        registeredCameras({}),
        startTime(glfwGetTime()),
        lastTime(glfwGetTime()),
        disableFeatureMask(0),
        lastDisableFeatureMaskChange(glfwGetTime() - 10) {}

public:
  // Preventing copying the render manager, making sure only one instance can exist.
  RenderManager(const RenderManager &) = delete;

  /**
   * Register a new light into the light manager.
   * 
   * @param light  The light to register.
   */
  void registerLight(const std::shared_ptr<const LightBase> &light)
  {
    registeredLights.insert(std::pair<const std::string, const std::shared_ptr<const LightBase>>(light->getLightId(), light));
  }

  /**
   * De-register an existing light from the light manager.
   * 
   * @param lightId  The ID of the light to de-register.
   */
  void deregisterLight(const std::string &lightId)
  {
    registeredLights.erase(lightId);
  }

  /**
   * De-register an existing light from the light manager.
   * 
   * @param light  The light to de-register.
   */
  void deregisterLight(const std::shared_ptr<const LightBase> &light)
  {
    registeredLights.erase(light->getLightId());
  }

  /**
   * Register a new model into the model manager.
   * 
   * @param model  The model to register.
   */
  void registerModel(const std::shared_ptr<const ModelBase> &model)
  {
    registeredModels.insert(std::pair<const std::string, const std::shared_ptr<const ModelBase>>(model->getModelId(), model));
  }

  /**
   * De-register an existing model from the model manager.
   * 
   * @param model  The ID of the model to de-register.
   */
  void deregisterModel(const std::string &modelId)
  {
    registeredModels.erase(modelId);
  }

  /**
   * De-register an existing model from the model manager.
   * 
   * @param model  The model to de-register.
   */
  void deregisterModel(const std::shared_ptr<const ModelBase> &model)
  {
    registeredModels.erase(model->getModelId());
  }

  /**
   * Register a new camera into the camera manager.
   * 
   * @param camera  The camera to register.
   */
  void registerCamera(const std::shared_ptr<const CameraBase> &camera)
  {
    registeredCameras.insert(std::pair<const std::string, const std::shared_ptr<const CameraBase>>(camera->getCameraId(), camera));
  }

  /**
   * De-register an existing camera from the camera manager.
   * 
   * @param cameraId  The ID of the camera to de-register.
   */
  void deregisterCamera(const std::string &cameraId)
  {
    registeredCameras.erase(cameraId);
  }

  /**
   * De-register an existing camera from the camera manager.
   * 
   * @param camera  The camera to de-register.
   */
  void deregisterCamera(const std::shared_ptr<const CameraBase> &camera)
  {
    registeredCameras.erase(camera->getCameraId());
  }

  /**
   * Registers a camera to be used as the active camera.
   * 
   * @param cameraId  The ID of the camera to set as the active camera.
   */
  void registerActiveCamera(const std::string &cameraId)
  {
    activeCameraId = cameraId;
  }

  /**
   * Registers a camera to be used as the active camera.
   * 
   * @param camera  The camera to set as the active camera.
   */
  void registerActiveCamera(const std::shared_ptr<const CameraBase> &camera)
  {
    activeCameraId = camera->getCameraId();
  }

  /**
   * Render the shadow maps for all the lights in the scene, and return the map of lights categorized by their shadow map type.
   * 
   * @return The map of the lights in the scene categorized by their shadow map type.
   */
  std::map<const ShadowBufferType, std::vector<LightDetails>> renderLights() const
  {
    // Switch the viewport to the size of the render framebuffers.
    windowManager.switchToFrameBufferViewport();

    // Create a map of the categorized lights.
    std::map<const ShadowBufferType, std::vector<LightDetails>> categorizedLights({{ShadowBufferType::CONE, {}}, {ShadowBufferType::POINT, {}}});

    // Set the current active shader ID to 0.
    GLuint currentShaderId = 0;

    {
      // Bind the cone light shadow framebuffer as the active framebuffer and clear it.
      glBindFramebuffer(GL_FRAMEBUFFER, shadowBufferManager.getConeLightShadowBufferId());
      glClear(GL_DEPTH_BUFFER_BIT);

      // Bind the point light shadow framebuffer as the active framebuffer and clear it.
      glBindFramebuffer(GL_FRAMEBUFFER, shadowBufferManager.getPointLightShadowBufferId());
      glClear(GL_DEPTH_BUFFER_BIT);

      // Bind the window framebuffer as the active framebuffer.
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Iterate through all the lights in the scene.
    for (auto light = registeredLights.begin(); light != registeredLights.end(); light++)
    {
      // Get the type of the shadow.
      const auto shadowType = light->second->getShadowBufferDetails()->getShadowBufferType();
      // Generate a structure detailing information about the light.
      const LightDetails lightDetails = {
          light->second->getLightPosition(),
          light->second->getProjectionMatrices()[0] * light->second->getViewMatrices()[0] * glm::mat4(),
          light->second->getLightColor(),
          light->second->getLightIntensity(),
          FRAMEBUFFER_WIDTH,
          FRAMEBUFFER_WIDTH,
          light->second->getLightNearPlane(),
          light->second->getLightFarPlane(),
          light->second->getShadowBufferDetails()->getShadowBufferTextureArrayLayerId()};
      // Store the light details in the categorized map.
      categorizedLights.at(shadowType).push_back(lightDetails);

      // If shadows are disabled, skip the shadowmap render step.
      if (disableFeatureMask >= DISABLE_SHADOW)
      {
        continue;
      }

      // Bind the shadowmap framebuffer of the light as the active framebuffer.
      glBindFramebuffer(GL_FRAMEBUFFER, light->second->getShadowBufferDetails()->getShadowBufferId());

      // Check if the shader of the light is the same as the currently used shader.
      if (currentShaderId != light->second->getShaderDetails()->getShaderId())
      {
        // If not, set it as the currently used shader and use it.
        currentShaderId = light->second->getShaderDetails()->getShaderId();
        glUseProgram(currentShaderId);
      }

      // Get the view and projection matrices of the light.
      const auto viewMatrices = light->second->getViewMatrices();
      const auto projectionMatrices = light->second->getProjectionMatrices();

      // Get the uniform ID of the count of the projection-view matrix variable and set it.
      const auto vpMatrixCountVertexId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), "lightDetails_vertex.vpMatrixCount");
      glUniform1i(vpMatrixCountVertexId, viewMatrices.size());
      const auto vpMatrixCountGeometryId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), "lightDetails_geometry.vpMatrixCount");
      glUniform1i(vpMatrixCountGeometryId, viewMatrices.size());
      const auto vpMatrixCountFragmentId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), "lightDetails_fragment.vpMatrixCount");
      glUniform1i(vpMatrixCountFragmentId, viewMatrices.size());

      // Get the uniform ID of the light position variable and set it.
      const auto lightPositionVertexId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), "lightDetails_vertex.lightPosition");
      glUniform3f(lightPositionVertexId, lightDetails.lightPosition.x, lightDetails.lightPosition.y, lightDetails.lightPosition.z);
      const auto lightPositionGeometryId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), "lightDetails_geometry.lightPosition");
      glUniform3f(lightPositionGeometryId, lightDetails.lightPosition.x, lightDetails.lightPosition.y, lightDetails.lightPosition.z);
      const auto lightPositionFragmentId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), "lightDetails_fragment.lightPosition");
      glUniform3f(lightPositionFragmentId, lightDetails.lightPosition.x, lightDetails.lightPosition.y, lightDetails.lightPosition.z);

      // Get the uniform ID of the lights' shadow map layer ID variable and set it.
      const auto layerIdVertexId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), "lightDetails_vertex.layerId");
      glUniform1i(layerIdVertexId, lightDetails.textureArrayLayerId);
      const auto layerIdGeometryId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), "lightDetails_geometry.layerId");
      glUniform1i(layerIdGeometryId, lightDetails.textureArrayLayerId);
      const auto layerIdFragmentId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), "lightDetails_fragment.layerId");
      glUniform1i(layerIdFragmentId, lightDetails.textureArrayLayerId);

      // Get the uniform ID of the near plane of the light variable and set it.
      const auto nearPlaneVertexId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), "projectionDetails_vertex.nearPlane");
      glUniform1f(nearPlaneVertexId, lightDetails.nearPlane);
      const auto nearPlaneGeometryId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), "projectionDetails_geometry.nearPlane");
      glUniform1f(nearPlaneGeometryId, lightDetails.nearPlane);
      const auto nearPlaneFragmentId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), "projectionDetails_fragment.nearPlane");
      glUniform1f(nearPlaneFragmentId, lightDetails.nearPlane);

      // Get the uniform ID of the far plane of the light variable and set it.
      const auto farPlaneVertexId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), "projectionDetails_vertex.farPlane");
      glUniform1f(farPlaneVertexId, lightDetails.farPlane);
      const auto farPlaneGeometryId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), "projectionDetails_geometry.farPlane");
      glUniform1f(farPlaneGeometryId, lightDetails.farPlane);
      const auto farPlaneFragmentId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), "projectionDetails_fragment.farPlane");
      glUniform1f(farPlaneFragmentId, lightDetails.farPlane);

      // Iterate through the view matrices of the light.
      for (unsigned long i = 0; i < viewMatrices.size(); i++)
      {
        // Calculate the projection-view matrix.
        const auto vpMatrix = projectionMatrices[i] * viewMatrices[i];
        // Get the uniform ID of the projection-view matrix of the light variable and set it.
        const auto vpMatrixVertexId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), ("lightDetails_vertex.vpMatrices[" + std::to_string(i) + "]").c_str());
        glUniformMatrix4fv(vpMatrixVertexId, 1, GL_FALSE, &vpMatrix[0][0]);
        const auto vpMatrixGeometryId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), ("lightDetails_geometry.vpMatrices[" + std::to_string(i) + "]").c_str());
        glUniformMatrix4fv(vpMatrixGeometryId, 1, GL_FALSE, &vpMatrix[0][0]);
        const auto vpMatrixFragmentId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), ("lightDetails_fragment.vpMatrices[" + std::to_string(i) + "]").c_str());
        glUniformMatrix4fv(vpMatrixFragmentId, 1, GL_FALSE, &vpMatrix[0][0]);
      }

      // Iterate through the models in the scene.
      for (auto model = registeredModels.begin(); model != registeredModels.end(); model++)
      {
        // Get the model matrix of the model.
        const auto modelMatrix = model->second->getModelMatrix();
        // Get the uniform ID of the model matrix variable and set it.
        const auto modelMatrixId = glGetUniformLocation(light->second->getShaderDetails()->getShaderId(), "modelMatrix");
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
  void renderModels(const std::map<const ShadowBufferType, std::vector<LightDetails>> &categorizedLights) const
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
    const auto activeCamera = registeredCameras.at(activeCameraId);
    // Get the view matrix of the camera.
    const auto viewMatrix = activeCamera->getViewMatrix();
    // Get the projection matrix of the camera.
    const auto projectionMatrix = activeCamera->getProjectionMatrix();

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
      const auto modelMatrix = model->second->getModelMatrix();
      // Get the uniform ID of the model matrix variable and set it.
      const auto modelMatrixVertexId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), "modelDetails_vertex.modelMatrix");
      glUniformMatrix4fv(modelMatrixVertexId, 1, GL_FALSE, &modelMatrix[0][0]);
      const auto modelMatrixFragmentId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), "modelDetails_fragment.modelMatrix");
      glUniformMatrix4fv(modelMatrixFragmentId, 1, GL_FALSE, &modelMatrix[0][0]);

      // Get the uniform ID of the diffuse texture of the model variable and set it.
      const auto diffuseTextureId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), "diffuseTexture");
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, model->second->getTextureDetails()->getTextureId());
      glUniform1i(diffuseTextureId, 0);

      // Get the uniform ID of the view matrix of the camera variable and set it.
      const auto viewMatrixVertexId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), "modelDetails_vertex.viewMatrix");
      glUniformMatrix4fv(viewMatrixVertexId, 1, GL_FALSE, &viewMatrix[0][0]);
      const auto viewMatrixFragmentId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), "modelDetails_fragment.viewMatrix");
      glUniformMatrix4fv(viewMatrixFragmentId, 1, GL_FALSE, &viewMatrix[0][0]);

      // Get the uniform ID of the projection matrix of the camera variable and set it.
      const auto projectionMatrixVertexId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), "modelDetails_vertex.projectionMatrix");
      glUniformMatrix4fv(projectionMatrixVertexId, 1, GL_FALSE, &projectionMatrix[0][0]);
      const auto projectionMatrixFragmentId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), "modelDetails_fragment.projectionMatrix");
      glUniformMatrix4fv(projectionMatrixFragmentId, 1, GL_FALSE, &projectionMatrix[0][0]);

      // Get the uniform ID of the disable feature mask variable and set it.
      const auto disableFeatureMaskId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), "disableFeatureMask");
      glUniform1i(disableFeatureMaskId, disableFeatureMask);

      // Get the uniform ID of the ambient lighting factor variable and set it.
      const auto ambientFactorId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), "ambientFactor");
      glUniform1f(ambientFactorId, ambientFactor);
      // Get the uniform ID of the cone lights count in the scene and set it.
      const auto coneLightsCountId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), "coneLightsCount");
      glUniform1i(coneLightsCountId, categorizedLights.at(ShadowBufferType::CONE).size());
      // Get the uniform ID of the point lights count in the scene and set it.
      const auto pointLightsCountId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), "pointLightsCount");
      glUniform1i(pointLightsCountId, categorizedLights.at(ShadowBufferType::POINT).size());

      // If lighting is not disabled, then setup the lighting information.
      if (disableFeatureMask < DISABLE_LIGHT)
      {
        // Iterate through the cone lights in the scene.
        for (unsigned long i = 0; i < categorizedLights.at(ShadowBufferType::CONE).size(); i++)
        {
          // Get the details of the cone light.
          const auto lightDetails = categorizedLights.at(ShadowBufferType::CONE)[i];

          // Get the uniform ID of the light position variable and set it.
          const auto lightPositionVertexId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("coneLightDetails_vertex[" + std::to_string(i) + "].lightPosition").c_str());
          glUniform3f(lightPositionVertexId, lightDetails.lightPosition.x, lightDetails.lightPosition.y, lightDetails.lightPosition.z);
          const auto lightPositionFragmentId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("coneLightDetails_fragment[" + std::to_string(i) + "].lightPosition").c_str());
          glUniform3f(lightPositionFragmentId, lightDetails.lightPosition.x, lightDetails.lightPosition.y, lightDetails.lightPosition.z);

          // Get the uniform ID of the count of the projection-view matrix variable and set it.
          const auto lightVpMatrixVertexId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("coneLightDetails_vertex[" + std::to_string(i) + "].lightVpMatrix").c_str());
          glUniformMatrix4fv(lightVpMatrixVertexId, 1, GL_FALSE, &lightDetails.lightVpMatrix[0][0]);
          const auto lightVpMatrixFragmentId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("coneLightDetails_fragment[" + std::to_string(i) + "].lightVpMatrix").c_str());
          glUniformMatrix4fv(lightVpMatrixFragmentId, 1, GL_FALSE, &lightDetails.lightVpMatrix[0][0]);

          // Get the uniform ID of the light color variable and set it.
          const auto lightColorVertexId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("coneLightDetails_vertex[" + std::to_string(i) + "].lightColor").c_str());
          glUniform3f(lightColorVertexId, lightDetails.lightColor.r, lightDetails.lightColor.g, lightDetails.lightColor.b);
          const auto lightColorFragmentId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("coneLightDetails_fragment[" + std::to_string(i) + "].lightColor").c_str());
          glUniform3f(lightColorFragmentId, lightDetails.lightColor.r, lightDetails.lightColor.g, lightDetails.lightColor.b);

          // Get the uniform ID of the light intensity variable and set it.
          const auto lightIntensityVertexId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("coneLightDetails_vertex[" + std::to_string(i) + "].lightIntensity").c_str());
          glUniform1f(lightIntensityVertexId, lightDetails.lightIntensity);
          const auto lightIntensityFragmentId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("coneLightDetails_fragment[" + std::to_string(i) + "].lightIntensity").c_str());
          glUniform1f(lightIntensityFragmentId, lightDetails.lightIntensity);

          // Get the uniform ID of the near plane of the light variable and set it.
          const auto lightNearPlaneVertexId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("coneLightDetails_vertex[" + std::to_string(i) + "].nearPlane").c_str());
          glUniform1f(lightNearPlaneVertexId, lightDetails.nearPlane);
          const auto lightNearPlaneFragmentId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("coneLightDetails_fragment[" + std::to_string(i) + "].nearPlane").c_str());
          glUniform1f(lightNearPlaneFragmentId, lightDetails.nearPlane);

          // Get the uniform ID of the far plane of the light variable and set it.
          const auto lightFarPlaneVertexId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("coneLightDetails_vertex[" + std::to_string(i) + "].farPlane").c_str());
          glUniform1f(lightFarPlaneVertexId, lightDetails.farPlane);
          const auto lightFarPlaneFragmentId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("coneLightDetails_fragment[" + std::to_string(i) + "].farPlane").c_str());
          glUniform1f(lightFarPlaneFragmentId, lightDetails.farPlane);

          // Get the uniform ID of the lights' shadow map layer ID variable and set it.
          const auto lightLayerIdVertexId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("coneLightDetails_vertex[" + std::to_string(i) + "].layerId").c_str());
          glUniform1i(lightLayerIdVertexId, lightDetails.textureArrayLayerId);
          const auto lightLayerIdFragmentId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("coneLightDetails_fragment[" + std::to_string(i) + "].layerId").c_str());
          glUniform1i(lightLayerIdFragmentId, lightDetails.textureArrayLayerId);
        }

        // Iterate through the point lights in the scene.
        for (unsigned long i = 0; i < categorizedLights.at(ShadowBufferType::POINT).size(); i++)
        {
          // Get the details of the point light.
          const auto lightDetails = categorizedLights.at(ShadowBufferType::POINT)[i];

          // Get the uniform ID of the light position variable and set it.
          const auto lightPositionVertexId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("pointLightDetails_vertex[" + std::to_string(i) + "].lightPosition").c_str());
          glUniform3f(lightPositionVertexId, lightDetails.lightPosition.x, lightDetails.lightPosition.y, lightDetails.lightPosition.z);
          const auto lightPositionFragmentId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("pointLightDetails_fragment[" + std::to_string(i) + "].lightPosition").c_str());
          glUniform3f(lightPositionFragmentId, lightDetails.lightPosition.x, lightDetails.lightPosition.y, lightDetails.lightPosition.z);

          // Get the uniform ID of the count of the projection-view matrix variable and set it.
          const auto lightVpMatrixVertexId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("pointLightDetails_vertex[" + std::to_string(i) + "].lightVpMatrix").c_str());
          glUniformMatrix4fv(lightVpMatrixVertexId, 1, GL_FALSE, &lightDetails.lightVpMatrix[0][0]);
          const auto lightVpMatrixFragmentId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("pointLightDetails_fragment[" + std::to_string(i) + "].lightVpMatrix").c_str());
          glUniformMatrix4fv(lightVpMatrixFragmentId, 1, GL_FALSE, &lightDetails.lightVpMatrix[0][0]);

          // Get the uniform ID of the light color variable and set it.
          const auto lightColorVertexId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("pointLightDetails_vertex[" + std::to_string(i) + "].lightColor").c_str());
          glUniform3f(lightColorVertexId, lightDetails.lightColor.r, lightDetails.lightColor.g, lightDetails.lightColor.b);
          const auto lightColorFragmentId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("pointLightDetails_fragment[" + std::to_string(i) + "].lightColor").c_str());
          glUniform3f(lightColorFragmentId, lightDetails.lightColor.r, lightDetails.lightColor.g, lightDetails.lightColor.b);

          // Get the uniform ID of the light intensity variable and set it.
          const auto lightIntensityVertexId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("pointLightDetails_vertex[" + std::to_string(i) + "].lightIntensity").c_str());
          glUniform1f(lightIntensityVertexId, lightDetails.lightIntensity);
          const auto lightIntensityFragmentId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("pointLightDetails_fragment[" + std::to_string(i) + "].lightIntensity").c_str());
          glUniform1f(lightIntensityFragmentId, lightDetails.lightIntensity);

          // Get the uniform ID of the near plane of the light variable and set it.
          const auto lightNearPlaneVertexId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("pointLightDetails_vertex[" + std::to_string(i) + "].nearPlane").c_str());
          glUniform1f(lightNearPlaneVertexId, lightDetails.nearPlane);
          const auto lightNearPlaneFragmentId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("pointLightDetails_fragment[" + std::to_string(i) + "].nearPlane").c_str());
          glUniform1f(lightNearPlaneFragmentId, lightDetails.nearPlane);

          // Get the uniform ID of the far plane of the light variable and set it.
          const auto lightFarPlaneVertexId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("pointLightDetails_vertex[" + std::to_string(i) + "].farPlane").c_str());
          glUniform1f(lightFarPlaneVertexId, lightDetails.farPlane);
          const auto lightFarPlaneFragmentId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("pointLightDetails_fragment[" + std::to_string(i) + "].farPlane").c_str());
          glUniform1f(lightFarPlaneFragmentId, lightDetails.farPlane);

          // Get the uniform ID of the lights' shadow map layer ID variable and set it.
          const auto lightLayerIdVertexId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("pointLightDetails_vertex[" + std::to_string(i) + "].layerId").c_str());
          glUniform1i(lightLayerIdVertexId, lightDetails.textureArrayLayerId / 6);
          const auto lightLayerIdFragmentId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), ("pointLightDetails_fragment[" + std::to_string(i) + "].layerId").c_str());
          glUniform1i(lightLayerIdFragmentId, lightDetails.textureArrayLayerId / 6);
        }
      }

      // Get the uniform ID of the cone light shadow map texture array and set it.
      const auto coneLightTextureId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), "coneLightTextures");
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D_ARRAY, shadowBufferManager.getConeLightTextureArrayId());
      glUniform1i(coneLightTextureId, 1);

      // Get the uniform ID of the point light shadow map texture array and set it.
      const auto pointLightTextureId = glGetUniformLocation(model->second->getShaderDetails()->getShaderId(), "pointLightTextures");
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, shadowBufferManager.getPointLightTextureArrayId());
      glUniform1i(pointLightTextureId, 2);

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
  }

  /**
   * Render the scene with the light shadowmaps and the models.
   */
  void render()
  {
    // Get the time at the start of the frame.
    const auto currentTime = glfwGetTime();

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
    const auto categorizedLights = renderLights();
    // Render the models.
    renderModels(categorizedLights);

    // Update the last start time of the latest rendered frame to the start time of the current frame.
    lastTime = currentTime;
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
const double RenderManager::ambientFactor = 0.25;
// Initialize the mask value for disabling shadows static variable.
const int RenderManager::DISABLE_SHADOW = 1;
// Initialize the mask value for disabling lighting static variable.
const int RenderManager::DISABLE_LIGHT = 2;

#endif