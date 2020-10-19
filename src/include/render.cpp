#ifndef INCLUDE_RENDER_CPP
#define INCLUDE_RENDER_CPP

#include <map>
#include <set>

#include <GL/glew.h>

#include "common.cpp"
#include "constants.cpp"
#include "window.cpp"
#include "control.cpp"
#include "shadowbuffer.cpp"
#include "camera.cpp"
#include "light.cpp"
#include "models.cpp"
#include "text.cpp"
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
  const double_t lightIntensity;
  // The width of the shadowmap of the light.
  const int32_t mapWidth;
  // The height of the shadowmap of the light.
  const int32_t mapHeight;
  // The closest distance from which the shadowmap captures objects.
  const double_t nearPlane;
  // The farthest distance till which the shadowmap captures objects.
  const double_t farPlane;
  // The ID of the layer of the shadowmap texture array the shadowmap is stored in.
  const GLuint textureArrayLayerId;
};

/**
 * A manager class for managing rendering of models.
 */
class RenderManager
{
  // Let the debug renderer access private variables.
  friend class DebugRenderManager;

private:
  // The ambient lighting factor of the scene.
  const static double_t ambientFactor;

  // The flags for disabling shadows and lighting.
  const static int32_t DISABLE_SHADOW;
  const static int32_t DISABLE_LIGHT;

  // Singleton instance of the render manager.
  static RenderManager instance;

  // The window manager responsible for the window.
  WindowManager &windowManager;
  // The camera manager responsible for managing all the cameras.
  CameraManager &cameraManager;
  // The light manager responsible for managing all the lights.
  LightManager &lightManager;
  // The model manager responsible for managing all the models.
  ModelManager &modelManager;
  // The text manager responsible for rendering text.
  TextManager &textManager;
  // The control manager responsible for managing controls and inputs of the window.
  const ControlManager &controlManager;
  // The shadow buffer manager responsible for creating shadow buffers for lights.
  const ShadowBufferManager &shadowBufferManager;

  // The ID of the active camera to use to render the scene to the window.
  std::string activeCameraId;

  // The timestamp when the render manager was loaded.
  const double_t startTime;
  // The timestamp of the start of the last render.
  double_t lastTime;

  // A mask defining what features to disable (shadows/lighting).
  int32_t disableFeatureMask;
  // The timestamp of the last time the mask for disabling features was modifed.
  double_t lastDisableFeatureMaskChange;

  RenderManager()
      : windowManager(WindowManager::getInstance()),
        cameraManager(CameraManager::getInstance()),
        lightManager(LightManager::getInstance()),
        modelManager(ModelManager::getInstance()),
        textManager(TextManager::getInstance()),
        controlManager(ControlManager::getInstance()),
        shadowBufferManager(ShadowBufferManager::getInstance()),
        startTime(glfwGetTime()),
        lastTime(glfwGetTime()),
        disableFeatureMask(0),
        lastDisableFeatureMaskChange(glfwGetTime() - 10) {}

public:
  // Preventing copying the render manager, making sure only one instance can exist.
  RenderManager(const RenderManager &) = delete;

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
   * Render the shadow maps for all the lights in the scene, and return the map of lights categorized by their shadow map type.
   * 
   * @return The map of the lights in the scene categorized by their shadow map type.
   */
  std::map<const ShadowBufferType, std::vector<LightDetails>> renderLights() const
  {
    // Switch the viewport to the size of the render framebuffers.
    windowManager.switchToFrameBufferViewport();

    // Create a map of the categorized lights.
    std::map<const ShadowBufferType, std::vector<LightDetails>> categorizedLightDetails({{ShadowBufferType::CONE, {}}, {ShadowBufferType::POINT, {}}});

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

    auto lightNamesCount = std::map<const std::string, int>({});
    auto lightNamesProcessTime = std::map<const std::string, double>({});

    std::map<const ShadowBufferType, std::vector<std::shared_ptr<LightBase>>> categorizedLights({{ShadowBufferType::CONE, {}}, {ShadowBufferType::POINT, {}}});
    for (const auto &light : lightManager.getAllLights())
    {
      categorizedLights.at(light->getShadowBufferDetails()->getShadowBufferType()).push_back(light);
    }

    for (const auto &lights : categorizedLights)
    {
      if (lights.second.empty())
      {
        continue;
      }

      const auto startTime = glfwGetTime();

      const auto firstLight = lights.second.front();

      // Bind the shadowmap framebuffer of the light as the active framebuffer.
      glBindFramebuffer(GL_FRAMEBUFFER, firstLight->getShadowBufferDetails()->getShadowBufferId());

      // Check if the shader of the light is the same as the currently used shader.
      if (currentShaderId != firstLight->getShaderDetails()->getShaderId())
      {
        // If not, set it as the currently used shader and use it.
        currentShaderId = firstLight->getShaderDetails()->getShaderId();
        glUseProgram(currentShaderId);
      }

      // Iterate through all the lights in the scene.
      for (unsigned long i = 0; i < lights.second.size(); i++)
      {
        const auto &light = lights.second.at(i);

        if (lightNamesCount.find(light->getLightName()) != lightNamesCount.end())
        {
          lightNamesCount[light->getLightName()]++;
        }
        else
        {
          lightNamesCount[light->getLightName()] = 1;
          lightNamesProcessTime[light->getLightName()] = 0.0;
        }

        // Get the type of the shadow.
        const auto shadowType = light->getShadowBufferDetails()->getShadowBufferType();
        // Generate a structure detailing information about the light.
        const LightDetails lightDetails = {
            light->getLightPosition(),
            light->getProjectionMatrices()[0] * light->getViewMatrices()[0] * glm::mat4(),
            light->getLightColor(),
            light->getLightIntensity(),
            FRAMEBUFFER_WIDTH,
            FRAMEBUFFER_WIDTH,
            light->getLightNearPlane(),
            light->getLightFarPlane(),
            light->getShadowBufferDetails()->getShadowBufferTextureArrayLayerId()};
        // Store the light details in the categorized map.
        categorizedLightDetails.at(shadowType).push_back(lightDetails);

        // If shadows are disabled, skip the shadowmap render step.
        if (disableFeatureMask >= DISABLE_SHADOW)
        {
          continue;
        }

        // Get the view and projection matrices of the light.
        const auto viewMatrices = light->getViewMatrices();
        const auto projectionMatrices = light->getProjectionMatrices();

        // Get the uniform ID of the count of the projection-view matrix variable and set it.
        const auto vpMatrixCountVertexId = glGetUniformLocation(light->getShaderDetails()->getShaderId(), ("lightDetails_vertex[" + std::to_string(i) + "].vpMatrixCount").c_str());
        glUniform1i(vpMatrixCountVertexId, viewMatrices.size());
        const auto vpMatrixCountGeometryId = glGetUniformLocation(light->getShaderDetails()->getShaderId(), ("lightDetails_geometry[" + std::to_string(i) + "].vpMatrixCount").c_str());
        glUniform1i(vpMatrixCountGeometryId, viewMatrices.size());
        const auto vpMatrixCountFragmentId = glGetUniformLocation(light->getShaderDetails()->getShaderId(), ("lightDetails_fragment[" + std::to_string(i) + "].vpMatrixCount").c_str());
        glUniform1i(vpMatrixCountFragmentId, viewMatrices.size());

        // Get the uniform ID of the light position variable and set it.
        const auto lightPositionVertexId = glGetUniformLocation(light->getShaderDetails()->getShaderId(), ("lightDetails_vertex[" + std::to_string(i) + "].lightPosition").c_str());
        glUniform3f(lightPositionVertexId, lightDetails.lightPosition.x, lightDetails.lightPosition.y, lightDetails.lightPosition.z);
        const auto lightPositionGeometryId = glGetUniformLocation(light->getShaderDetails()->getShaderId(), ("lightDetails_geometry[" + std::to_string(i) + "].lightPosition").c_str());
        glUniform3f(lightPositionGeometryId, lightDetails.lightPosition.x, lightDetails.lightPosition.y, lightDetails.lightPosition.z);
        const auto lightPositionFragmentId = glGetUniformLocation(light->getShaderDetails()->getShaderId(), ("lightDetails_fragment[" + std::to_string(i) + "].lightPosition").c_str());
        glUniform3f(lightPositionFragmentId, lightDetails.lightPosition.x, lightDetails.lightPosition.y, lightDetails.lightPosition.z);

        // Get the uniform ID of the lights' shadow map layer ID variable and set it.
        const auto layerIdVertexId = glGetUniformLocation(light->getShaderDetails()->getShaderId(), ("lightDetails_vertex[" + std::to_string(i) + "].layerId").c_str());
        glUniform1i(layerIdVertexId, lightDetails.textureArrayLayerId);
        const auto layerIdGeometryId = glGetUniformLocation(light->getShaderDetails()->getShaderId(), ("lightDetails_geometry[" + std::to_string(i) + "].layerId").c_str());
        glUniform1i(layerIdGeometryId, lightDetails.textureArrayLayerId);
        const auto layerIdFragmentId = glGetUniformLocation(light->getShaderDetails()->getShaderId(), ("lightDetails_fragment[" + std::to_string(i) + "].layerId").c_str());
        glUniform1i(layerIdFragmentId, lightDetails.textureArrayLayerId);

        // Get the uniform ID of the near plane of the light variable and set it.
        const auto nearPlaneVertexId = glGetUniformLocation(light->getShaderDetails()->getShaderId(), ("projectionDetails_vertex[" + std::to_string(i) + "].nearPlane").c_str());
        glUniform1f(nearPlaneVertexId, lightDetails.nearPlane);
        const auto nearPlaneGeometryId = glGetUniformLocation(light->getShaderDetails()->getShaderId(), ("projectionDetails_geometry[" + std::to_string(i) + "].nearPlane").c_str());
        glUniform1f(nearPlaneGeometryId, lightDetails.nearPlane);
        const auto nearPlaneFragmentId = glGetUniformLocation(light->getShaderDetails()->getShaderId(), ("projectionDetails_fragment[" + std::to_string(i) + "].nearPlane").c_str());
        glUniform1f(nearPlaneFragmentId, lightDetails.nearPlane);

        // Get the uniform ID of the far plane of the light variable and set it.
        const auto farPlaneVertexId = glGetUniformLocation(light->getShaderDetails()->getShaderId(), ("projectionDetails_vertex[" + std::to_string(i) + "].farPlane").c_str());
        glUniform1f(farPlaneVertexId, lightDetails.farPlane);
        const auto farPlaneGeometryId = glGetUniformLocation(light->getShaderDetails()->getShaderId(), ("projectionDetails_geometry[" + std::to_string(i) + "].farPlane").c_str());
        glUniform1f(farPlaneGeometryId, lightDetails.farPlane);
        const auto farPlaneFragmentId = glGetUniformLocation(light->getShaderDetails()->getShaderId(), ("projectionDetails_fragment[" + std::to_string(i) + "].farPlane").c_str());
        glUniform1f(farPlaneFragmentId, lightDetails.farPlane);

        // Iterate through the view matrices of the light.
        for (unsigned long j = 0; j < viewMatrices.size(); j++)
        {
          // Calculate the projection-view matrix.
          const auto vpMatrix = projectionMatrices[j] * viewMatrices[j];
          // Get the uniform ID of the projection-view matrix of the light variable and set it.
          const auto vpMatrixVertexId = glGetUniformLocation(light->getShaderDetails()->getShaderId(), ("lightDetails_vertex[" + std::to_string(i) + "].vpMatrices[" + std::to_string(j) + "]").c_str());
          glUniformMatrix4fv(vpMatrixVertexId, 1, GL_FALSE, &vpMatrix[0][0]);
          const auto vpMatrixGeometryId = glGetUniformLocation(light->getShaderDetails()->getShaderId(), ("lightDetails_geometry[" + std::to_string(i) + "].vpMatrices[" + std::to_string(j) + "]").c_str());
          glUniformMatrix4fv(vpMatrixGeometryId, 1, GL_FALSE, &vpMatrix[0][0]);
          const auto vpMatrixFragmentId = glGetUniformLocation(light->getShaderDetails()->getShaderId(), ("lightDetails_fragment[" + std::to_string(i) + "].vpMatrices[" + std::to_string(j) + "]").c_str());
          glUniformMatrix4fv(vpMatrixFragmentId, 1, GL_FALSE, &vpMatrix[0][0]);
        }
      }

      // Get the uniform ID of the lights count variable and set it.
      const auto lightsCountVertexId = glGetUniformLocation(firstLight->getShaderDetails()->getShaderId(), "lightsCount");
      glUniform1i(lightsCountVertexId, lights.second.size());

      // Iterate through the models in the scene.
      for (const auto &model : modelManager.getAllModels())
      {
        // Get the model matrix of the model.
        const auto modelMatrix = model->getModelMatrix();
        // Get the uniform ID of the model matrix variable and set it.
        const auto modelMatrixId = glGetUniformLocation(firstLight->getShaderDetails()->getShaderId(), "modelMatrix");
        glUniformMatrix4fv(modelMatrixId, 1, GL_FALSE, &modelMatrix[0][0]);

        // Define a vertex attribute array that contains the vertex position data of the model.
        VertexAttributeArray vertexArray("VertexArray", model->getObjectDetails()->getVertexBufferId(), 3);

        // Enable it so that it can be used by the GPU.
        vertexArray.enableAttribute();

        // Draw the triangles of the model.
        glDrawArrays(GL_TRIANGLES, 0, model->getObjectDetails()->getBufferSize());
      }

      // Bind the window framebuffer as the active framebuffer.
      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      const auto endTime = glfwGetTime();

      lightNamesProcessTime[firstLight->getLightName()] += (endTime - startTime) * 1000;
    }

    auto height = 21.5;
    for (const auto &lightCounts : lightNamesCount)
    {
      const auto avgRenderTime = lightNamesProcessTime[lightCounts.first] / lightCounts.second;
      textManager.addText(lightCounts.first + " Light Render Instances: " + std::to_string(lightCounts.second) + " | Render (avg): " + std::to_string(avgRenderTime) + "ms", glm::vec2(1, height), 0.5);
      height -= 0.5;
    }

    // Return the map of the categorized lights.
    return categorizedLightDetails;
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
    const auto activeCamera = cameraManager.getCamera(activeCameraId);
    // Get the view matrix of the camera.
    const auto viewMatrix = activeCamera->getViewMatrix();
    // Get the projection matrix of the camera.
    const auto projectionMatrix = activeCamera->getProjectionMatrix();

    auto modelNamesCount = std::map<const std::string, int>({});
    auto modelNamesProcessTime = std::map<const std::string, double>({});

    // Iterate through all the models in the scene.
    for (const auto &model : modelManager.getAllModels())
    {
      // Check if the shader of the light is the same as the currently used shader.
      if (currentShaderId != model->getShaderDetails()->getShaderId())
      {
        // If not, set it as the currently used shader and use it.
        currentShaderId = model->getShaderDetails()->getShaderId();
        glUseProgram(currentShaderId);
      }

      if (modelNamesCount.find(model->getModelName()) != modelNamesCount.end())
      {
        modelNamesCount[model->getModelName()]++;
      }
      else
      {
        modelNamesCount[model->getModelName()] = 1;
        modelNamesProcessTime[model->getModelName()] = 0.0;
      }

      const auto startTime = glfwGetTime();

      // Get the model matrix of the model.
      const auto modelMatrix = model->getModelMatrix();
      // Get the uniform ID of the model matrix variable and set it.
      const auto modelMatrixVertexId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), "modelDetails_vertex.modelMatrix");
      glUniformMatrix4fv(modelMatrixVertexId, 1, GL_FALSE, &modelMatrix[0][0]);
      const auto modelMatrixFragmentId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), "modelDetails_fragment.modelMatrix");
      glUniformMatrix4fv(modelMatrixFragmentId, 1, GL_FALSE, &modelMatrix[0][0]);

      // Get the uniform ID of the diffuse texture of the model variable and set it.
      const auto diffuseTextureId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), "diffuseTexture");
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, model->getTextureDetails()->getTextureId());
      glUniform1i(diffuseTextureId, 0);

      // Get the uniform ID of the view matrix of the camera variable and set it.
      const auto viewMatrixVertexId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), "modelDetails_vertex.viewMatrix");
      glUniformMatrix4fv(viewMatrixVertexId, 1, GL_FALSE, &viewMatrix[0][0]);
      const auto viewMatrixFragmentId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), "modelDetails_fragment.viewMatrix");
      glUniformMatrix4fv(viewMatrixFragmentId, 1, GL_FALSE, &viewMatrix[0][0]);

      // Get the uniform ID of the projection matrix of the camera variable and set it.
      const auto projectionMatrixVertexId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), "modelDetails_vertex.projectionMatrix");
      glUniformMatrix4fv(projectionMatrixVertexId, 1, GL_FALSE, &projectionMatrix[0][0]);
      const auto projectionMatrixFragmentId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), "modelDetails_fragment.projectionMatrix");
      glUniformMatrix4fv(projectionMatrixFragmentId, 1, GL_FALSE, &projectionMatrix[0][0]);

      // Get the uniform ID of the disable feature mask variable and set it.
      const auto disableFeatureMaskId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), "disableFeatureMask");
      glUniform1i(disableFeatureMaskId, disableFeatureMask);

      // Get the uniform ID of the ambient lighting factor variable and set it.
      const auto ambientFactorId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), "ambientFactor");
      glUniform1f(ambientFactorId, ambientFactor);
      // Get the uniform ID of the cone lights count in the scene and set it.
      const auto coneLightsCountId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), "coneLightsCount");
      glUniform1i(coneLightsCountId, categorizedLights.at(ShadowBufferType::CONE).size());
      // Get the uniform ID of the point lights count in the scene and set it.
      const auto pointLightsCountId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), "pointLightsCount");
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
          const auto lightPositionVertexId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), ("coneLightDetails_vertex[" + std::to_string(i) + "].lightPosition").c_str());
          glUniform3f(lightPositionVertexId, lightDetails.lightPosition.x, lightDetails.lightPosition.y, lightDetails.lightPosition.z);
          const auto lightPositionFragmentId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), ("coneLightDetails_fragment[" + std::to_string(i) + "].lightPosition").c_str());
          glUniform3f(lightPositionFragmentId, lightDetails.lightPosition.x, lightDetails.lightPosition.y, lightDetails.lightPosition.z);

          // Get the uniform ID of the count of the projection-view matrix variable and set it.
          const auto lightVpMatrixVertexId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), ("coneLightDetails_vertex[" + std::to_string(i) + "].lightVpMatrix").c_str());
          glUniformMatrix4fv(lightVpMatrixVertexId, 1, GL_FALSE, &lightDetails.lightVpMatrix[0][0]);
          const auto lightVpMatrixFragmentId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), ("coneLightDetails_fragment[" + std::to_string(i) + "].lightVpMatrix").c_str());
          glUniformMatrix4fv(lightVpMatrixFragmentId, 1, GL_FALSE, &lightDetails.lightVpMatrix[0][0]);

          // Get the uniform ID of the light color-intensity variable and set it.
          const auto lightColorIntensityVertexId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), ("coneLightDetails_vertex[" + std::to_string(i) + "].lightColorIntensity").c_str());
          glUniform3f(lightColorIntensityVertexId, lightDetails.lightColor.r * lightDetails.lightIntensity, lightDetails.lightColor.g * lightDetails.lightIntensity, lightDetails.lightColor.b * lightDetails.lightIntensity);
          const auto lightColorIntensityFragmentId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), ("coneLightDetails_fragment[" + std::to_string(i) + "].lightColorIntensity").c_str());
          glUniform3f(lightColorIntensityFragmentId, lightDetails.lightColor.r * lightDetails.lightIntensity, lightDetails.lightColor.g * lightDetails.lightIntensity, lightDetails.lightColor.b * lightDetails.lightIntensity);

          // Get the uniform ID of the near plane of the light variable and set it.
          const auto lightNearPlaneVertexId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), ("coneLightDetails_vertex[" + std::to_string(i) + "].nearPlane").c_str());
          glUniform1f(lightNearPlaneVertexId, lightDetails.nearPlane);
          const auto lightNearPlaneFragmentId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), ("coneLightDetails_fragment[" + std::to_string(i) + "].nearPlane").c_str());
          glUniform1f(lightNearPlaneFragmentId, lightDetails.nearPlane);

          // Get the uniform ID of the far plane of the light variable and set it.
          const auto lightFarPlaneVertexId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), ("coneLightDetails_vertex[" + std::to_string(i) + "].farPlane").c_str());
          glUniform1f(lightFarPlaneVertexId, lightDetails.farPlane);
          const auto lightFarPlaneFragmentId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), ("coneLightDetails_fragment[" + std::to_string(i) + "].farPlane").c_str());
          glUniform1f(lightFarPlaneFragmentId, lightDetails.farPlane);

          // Get the uniform ID of the lights' shadow map layer ID variable and set it.
          const auto lightLayerIdVertexId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), ("coneLightDetails_vertex[" + std::to_string(i) + "].layerId").c_str());
          glUniform1i(lightLayerIdVertexId, lightDetails.textureArrayLayerId);
          const auto lightLayerIdFragmentId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), ("coneLightDetails_fragment[" + std::to_string(i) + "].layerId").c_str());
          glUniform1i(lightLayerIdFragmentId, lightDetails.textureArrayLayerId);
        }

        // Iterate through the point lights in the scene.
        for (unsigned long i = 0; i < categorizedLights.at(ShadowBufferType::POINT).size(); i++)
        {
          // Get the details of the point light.
          const auto lightDetails = categorizedLights.at(ShadowBufferType::POINT)[i];

          // Get the uniform ID of the light position variable and set it.
          const auto lightPositionVertexId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), ("pointLightDetails_vertex[" + std::to_string(i) + "].lightPosition").c_str());
          glUniform3f(lightPositionVertexId, lightDetails.lightPosition.x, lightDetails.lightPosition.y, lightDetails.lightPosition.z);
          const auto lightPositionFragmentId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), ("pointLightDetails_fragment[" + std::to_string(i) + "].lightPosition").c_str());
          glUniform3f(lightPositionFragmentId, lightDetails.lightPosition.x, lightDetails.lightPosition.y, lightDetails.lightPosition.z);

          // Get the uniform ID of the count of the projection-view matrix variable and set it.
          const auto lightVpMatrixVertexId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), ("pointLightDetails_vertex[" + std::to_string(i) + "].lightVpMatrix").c_str());
          glUniformMatrix4fv(lightVpMatrixVertexId, 1, GL_FALSE, &lightDetails.lightVpMatrix[0][0]);
          const auto lightVpMatrixFragmentId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), ("pointLightDetails_fragment[" + std::to_string(i) + "].lightVpMatrix").c_str());
          glUniformMatrix4fv(lightVpMatrixFragmentId, 1, GL_FALSE, &lightDetails.lightVpMatrix[0][0]);

          // Get the uniform ID of the light color-intensity variable and set it.
          const auto lightColorIntensityVertexId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), ("pointLightDetails_vertex[" + std::to_string(i) + "].lightColorIntensity").c_str());
          glUniform3f(lightColorIntensityVertexId, lightDetails.lightColor.r * lightDetails.lightIntensity, lightDetails.lightColor.g * lightDetails.lightIntensity, lightDetails.lightColor.b * lightDetails.lightIntensity);
          const auto lightColorIntensityFragmentId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), ("pointLightDetails_fragment[" + std::to_string(i) + "].lightColorIntensity").c_str());
          glUniform3f(lightColorIntensityFragmentId, lightDetails.lightColor.r * lightDetails.lightIntensity, lightDetails.lightColor.g * lightDetails.lightIntensity, lightDetails.lightColor.b * lightDetails.lightIntensity);

          // Get the uniform ID of the near plane of the light variable and set it.
          const auto lightNearPlaneVertexId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), ("pointLightDetails_vertex[" + std::to_string(i) + "].nearPlane").c_str());
          glUniform1f(lightNearPlaneVertexId, lightDetails.nearPlane);
          const auto lightNearPlaneFragmentId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), ("pointLightDetails_fragment[" + std::to_string(i) + "].nearPlane").c_str());
          glUniform1f(lightNearPlaneFragmentId, lightDetails.nearPlane);

          // Get the uniform ID of the far plane of the light variable and set it.
          const auto lightFarPlaneVertexId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), ("pointLightDetails_vertex[" + std::to_string(i) + "].farPlane").c_str());
          glUniform1f(lightFarPlaneVertexId, lightDetails.farPlane);
          const auto lightFarPlaneFragmentId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), ("pointLightDetails_fragment[" + std::to_string(i) + "].farPlane").c_str());
          glUniform1f(lightFarPlaneFragmentId, lightDetails.farPlane);

          // Get the uniform ID of the lights' shadow map layer ID variable and set it.
          const auto lightLayerIdVertexId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), ("pointLightDetails_vertex[" + std::to_string(i) + "].layerId").c_str());
          glUniform1i(lightLayerIdVertexId, lightDetails.textureArrayLayerId / 6);
          const auto lightLayerIdFragmentId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), ("pointLightDetails_fragment[" + std::to_string(i) + "].layerId").c_str());
          glUniform1i(lightLayerIdFragmentId, lightDetails.textureArrayLayerId / 6);
        }
      }

      // Get the uniform ID of the cone light shadow map texture array and set it.
      const auto coneLightTextureId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), "coneLightTextures");
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D_ARRAY, shadowBufferManager.getConeLightTextureArrayId());
      glUniform1i(coneLightTextureId, 1);

      // Get the uniform ID of the point light shadow map texture array and set it.
      const auto pointLightTextureId = glGetUniformLocation(model->getShaderDetails()->getShaderId(), "pointLightTextures");
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, shadowBufferManager.getPointLightTextureArrayId());
      glUniform1i(pointLightTextureId, 2);

      // Define vertex attribute arrays that contains the vertex position, UV coordinates, and normal vector data of the model.
      VertexAttributeArray vertexArray("VertexArray", model->getObjectDetails()->getVertexBufferId(), 3);
      VertexAttributeArray uvArray("UvArray", model->getObjectDetails()->getUvBufferId(), 2);
      VertexAttributeArray normalArray("NormalArray", model->getObjectDetails()->getNormalBufferId(), 3);

      // Enable them so that it can be used by the GPU.
      vertexArray.enableAttribute();
      uvArray.enableAttribute();
      normalArray.enableAttribute();

      // Draw the triangles of the model.
      glDrawArrays(GL_TRIANGLES, 0, model->getObjectDetails()->getBufferSize());

      const auto endTime = glfwGetTime();

      modelNamesProcessTime[model->getModelName()] += (endTime - startTime) * 1000;
    }

    auto height = 23.0;
    for (const auto &modelCounts : modelNamesCount)
    {
      const auto avgRenderTime = modelNamesProcessTime[modelCounts.first] / modelCounts.second;
      textManager.addText(modelCounts.first + " Model Render Instances: " + std::to_string(modelCounts.second) + " | Render (avg): " + std::to_string(avgRenderTime) + "ms", glm::vec2(1, height), 0.5);
      height -= 0.5;
    }
  }

  /**
   * Render the scene with the light shadowmaps and the models.
   */
  void render()
  {
    // Get the time at the start of the frame.
    const auto currentTime = glfwGetTime();
    auto updateStartTime = currentTime, updateEndTime = currentTime;

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
    updateStartTime = glfwGetTime();
    const auto categorizedLights = renderLights();
    updateEndTime = glfwGetTime();
    textManager.addText("Light Render: " + std::to_string((updateEndTime - updateStartTime) * 1000) + "ms", glm::vec2(1, 25.5), 0.5);

    // Render the models.
    updateStartTime = glfwGetTime();
    renderModels(categorizedLights);
    updateEndTime = glfwGetTime();
    textManager.addText("Model Render: " + std::to_string((updateEndTime - updateStartTime) * 1000) + "ms", glm::vec2(1, 25), 0.5);

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
const double_t RenderManager::ambientFactor = 0.25;
// Initialize the mask value for disabling shadows static variable.
const int32_t RenderManager::DISABLE_SHADOW = 1;
// Initialize the mask value for disabling lighting static variable.
const int32_t RenderManager::DISABLE_LIGHT = 2;

#endif