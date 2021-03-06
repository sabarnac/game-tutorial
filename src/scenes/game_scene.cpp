#ifndef SCENES_GAME_SCENE_CPP
#define SCENES_GAME_SCENE_CPP

#include <string>
#include <optional>
#include <memory>
#include <vector>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "../include/window.cpp"
#include "../include/control.cpp"
#include "../include/camera.cpp"
#include "../include/light.cpp"
#include "../include/render.cpp"
#include "../include/debug_render.cpp"
#include "../include/text.cpp"

#include "../camera/perspective_camera.cpp"
#include "../models/enemy_model.cpp"
#include "../models/player_model.cpp"

#include "scene_base.cpp"

class GameScene : public SceneBase
{
private:
  ControlManager &controlManager;
  ModelManager &modelManager;
  LightManager &lightManager;
  CameraManager &cameraManager;
  RenderManager &renderManager;
  DebugRenderManager &debugRenderManager;

  std::vector<std::string> sceneCameraIds;
  std::vector<std::string> sceneModelIds;

  void initCameras()
  {
    // Create a perspective camera, and set its properties.
    const auto cameraId = "MainCamera";
    sceneCameraIds.push_back(cameraId);

    const auto perspectiveCamera = PerspectiveCamera::create(cameraId);
    perspectiveCamera->setCameraPosition(glm::vec3(0.0f, 20.0f, 40.0f));
    perspectiveCamera->setCameraAngles(glm::pi<float_t>(), -(glm::pi<float_t>() / 4.3f));

    cameraManager.registerCamera(perspectiveCamera);
    renderManager.registerActiveCamera(perspectiveCamera->getCameraId());
  }

  void deinitCameras()
  {
    for (const auto &cameraId : sceneCameraIds)
    {
      cameraManager.deregisterCamera(cameraId);
    }
  }

  void initEnemyModels()
  {
    // Create 45 enemy models stacked in a grid format (5 x 3 x 3), and set their properties.
    for (auto i = -2; i <= 2; i++)
    {
      for (auto j = -1; j <= 1; j++)
      {
        for (auto k = -2; k <= 0; k++)
        {
          const auto enemyModelId = "Enemy" + std::to_string((9 * (i + 2)) + (3 * (j + 1)) + (k + 2));
          sceneModelIds.push_back(enemyModelId);

          const auto enemyModel = EnemyModel::create(enemyModelId);
          enemyModel->setModelPosition(glm::vec3(i * 5, j * 5, k * 5));
          modelManager.registerModel(enemyModel);
        }
      }
    }
  }

  void initPlayerModels()
  {
    // Create a player model.
    const auto playerModelId = "MainPlayer";
    sceneModelIds.push_back(playerModelId);

    const auto playerModel = PlayerModel::create(playerModelId);
    modelManager.registerModel(playerModel);
  }

  void initModels()
  {
    EnemyModel::initModel();
    renderLoadingText("Loading (35%)", glm::vec2(1, 1), 1.0f);
    PlayerModel::initModel();
    renderLoadingText("Loading (60%)", glm::vec2(1, 1), 1.0f);
    ShotModel::initModel();
    renderLoadingText("Loading (85%)", glm::vec2(1, 1), 1.0f);

    initEnemyModels();
    renderLoadingText("Loading (90%)", glm::vec2(1, 1), 1.0f);
    initPlayerModels();
  }

  void deinitModels()
  {
    for (const auto &modelId : sceneModelIds)
    {
      modelManager.deregisterModel(modelId);
    }

    // Iterate over the list of registered models.
    for (const auto &model : modelManager.getAllModels())
    {
      // Check if the current model is a shot model.
      if (model->getModelName() != "Shot")
      {
        continue;
      }

      modelManager.deregisterModel(model->getModelId());
    }

    EnemyModel::deinitModel();
    PlayerModel::deinitModel();
    ShotModel::deinitModel();
  }

public:
  GameScene(const std::string &sceneId)
      : SceneBase(sceneId, "GameScene"),
        controlManager(ControlManager::getInstance()),
        modelManager(ModelManager::getInstance()),
        lightManager(LightManager::getInstance()),
        cameraManager(CameraManager::getInstance()),
        renderManager(RenderManager::getInstance()),
        debugRenderManager(DebugRenderManager::getInstance())
  {
    sceneModelIds = std::vector<std::string>({});
    sceneCameraIds = std::vector<std::string>({});
  }

  const static std::shared_ptr<GameScene> create(const std::string &sceneId)
  {
    return std::make_shared<GameScene>(sceneId);
  }

  const void init()
  {
    renderLoadingText("Loading (0%)", glm::vec2(1, 1), 1.0f);
    initCameras();
    renderLoadingText("Loading (10%)", glm::vec2(1, 1), 1.0f);
    initModels();
    renderLoadingText("Loading (95%)", glm::vec2(1, 1), 1.0f);

    // Poll for events and set the mouse to the center of the screen
    controlManager.disableCursor();
    controlManager.setCursorPosition(CursorPosition(0.5f, 0.5f));
    controlManager.pollEvents();
    renderLoadingText("Loading (100%)", glm::vec2(1, 1), 1.0f);
  }

  const void deinit()
  {
    renderLoadingText("Cleaning (0%)", glm::vec2(1, 1), 1.0f);
    deinitModels();
    renderLoadingText("Cleaning (50%)", glm::vec2(1, 1), 1.0f);
    deinitCameras();
    renderLoadingText("Cleaning (100%)", glm::vec2(1, 1), 1.0f);
  }

  const uint32_t getEnemyModelsCount()
  {
    uint32_t enemyModelsCount = 0;

    // Iterate over the list of registered models.
    for (const auto &model : modelManager.getAllModels())
    {
      if (model->getModelName() != "Enemy")
      {
        continue;
      }

      ++enemyModelsCount;
    }

    return enemyModelsCount;
  }

  const std::optional<std::string> execute()
  {
    // consume events we don't want passed on
    controlManager.isKeyPressed(GLFW_KEY_SPACE);
    controlManager.isKeyPressed(GLFW_KEY_ESCAPE);
    windowManager.isWindowCloseRequested();

    modelManager.initAllModels();
    cameraManager.initAllCameras();
    lightManager.initAllLights();

    // Set debug mode to initially false.
    auto debugEnabled = false;
    // Set the timestamp for when debug mode toggle was changed to 10 seconds in the past.
    auto lastDebugEnabledChange = glfwGetTime() - 10;

    // Set debug text to initially false.
    auto textEnabled = false;
    // Set the timestamp for when debug text toggle was changed to 10 seconds in the past.
    auto lastTextEnabledChange = glfwGetTime() - 10;

    // Set the timestamp for when debug text toggle was changed to 10 seconds in the past.
    auto lastVsyncToggledChange = glfwGetTime() - 10;

    // Start the game loop.
    auto textRenderTimeLast = 0.0f;
    auto frameTimeLast = 0.0f;
    auto processTimeLast = 0.0f;
    uint32_t textCharsRenderedLast = 0;
    do
    {
      textManager.addText("Window Dimensions: " + std::to_string(WINDOW_WIDTH) + "x" + std::to_string(WINDOW_HEIGHT) + "px", glm::vec2(1, 11), 0.5f);
      textManager.addText("Viewport Dimensions: " + std::to_string(VIEWPORT_WIDTH) + "x" + std::to_string(VIEWPORT_HEIGHT) + "px", glm::vec2(1, 10.5f), 0.5f);
      textManager.addText("Framebuffer Dimensions: " + std::to_string(FRAMEBUFFER_WIDTH) + "x" + std::to_string(FRAMEBUFFER_HEIGHT) + "px", glm::vec2(1, 10), 0.5f);
      textManager.addText("Text Dimensions: " + std::to_string(TEXT_WIDTH) + "x" + std::to_string(TEXT_HEIGHT) + "px", glm::vec2(1, 9.5f), 0.5f);
      textManager.addText("Max Lights:", glm::vec2(1, 9), 0.5f);
      textManager.addText(std::to_string(MAX_CONE_LIGHTS) + " Cone Lights", glm::vec2(3, 8.5f), 0.5f);
      textManager.addText(std::to_string(MAX_POINT_LIGHTS) + " Point Lights", glm::vec2(3, 8), 0.5f);
      textManager.addText("Max Text Characters: " + std::to_string(MAX_TEXT_CHARS) + " chars", glm::vec2(1, 7.5f), 0.5f);

      auto isSwapEnabledStr = SWAP_INTERVAL == 0 ? std::string("False") : SWAP_INTERVAL == 1 ? std::string("True (Single-Sync)") : std::string("True (Double-Sync)");
      textManager.addText("VSync Enabled: " + isSwapEnabledStr, glm::vec2(1, 7), 0.5f);

      // Get the time at the start of the loop.
      const auto currentTime = glfwGetTime();
      auto updateStartTime = currentTime, updateEndTime = currentTime;

      // Check if "B" key was pressed beyond 500ms since the last debug mode toggle.
      if (controlManager.isKeyPressed(GLFW_KEY_B) && (currentTime - lastDebugEnabledChange) > 0.5f)
      {
        // "B" key was pressed. Toggle debug mode and update the last change timestamp.
        debugEnabled = !debugEnabled;
        lastDebugEnabledChange = currentTime;
      }

      // Check if "T" key was pressed beyond 500ms since the last debug text toggle.
      if (controlManager.isKeyPressed(GLFW_KEY_T) && (currentTime - lastTextEnabledChange) > 0.5f)
      {
        // "T" key was pressed. Toggle debug text and update the last change timestamp.
        textEnabled = !textEnabled;
        lastTextEnabledChange = currentTime;
      }

      // Check if "V" key was pressed beyond 500ms since the last vsync toggle.
      if (controlManager.isKeyPressed(GLFW_KEY_V) && (currentTime - lastVsyncToggledChange) > 0.5f)
      {
        // "V" key was pressed. Toggle vsync and update the last change timestamp.
        windowManager.toggleVsync();
        lastVsyncToggledChange = currentTime;
      }

      // Update the lights.
      updateStartTime = glfwGetTime();
      lightManager.updateAllLights();
      updateEndTime = glfwGetTime();
      textManager.addText("Light Update: " + std::to_string((updateEndTime - updateStartTime) * 1000) + "ms", glm::vec2(1, 0.5f), 0.5f);

      // Update the cameras.
      updateStartTime = glfwGetTime();
      modelManager.updateAllModels();
      updateEndTime = glfwGetTime();
      textManager.addText("Model Update: " + std::to_string((updateEndTime - updateStartTime) * 1000) + "ms", glm::vec2(1, 1), 0.5f);

      // Update the models.
      updateStartTime = glfwGetTime();
      cameraManager.updateAllCameras();
      updateEndTime = glfwGetTime();
      textManager.addText("Camera Update: " + std::to_string((updateEndTime - updateStartTime) * 1000) + "ms", glm::vec2(1, 1.5f), 0.5f);

      // Render the scene.
      updateStartTime = glfwGetTime();
      renderManager.render();
      updateEndTime = glfwGetTime();
      textManager.addText("Render: " + std::to_string((updateEndTime - updateStartTime) * 1000) + "ms", glm::vec2(1, 2), 0.5f);

      // Check if debug mode is enabled.
      if (debugEnabled)
      {
        // Render the debug models fo the main models and lights.
        updateStartTime = glfwGetTime();
        debugRenderManager.render();
        updateEndTime = glfwGetTime();
        textManager.addText("Debug Render: " + std::to_string((updateEndTime - updateStartTime) * 1000) + "ms", glm::vec2(1, 2.5f), 0.5f);
      }

      // Render text
      textManager.addText("Text Render (Last Frame): " + std::to_string(textRenderTimeLast) + "ms", glm::vec2(1, 3), 0.5f);
      textManager.addText("Text Characters Rendered (Last Frame): " + std::to_string(textCharsRenderedLast) + " chars", glm::vec2(1, 3.5f), 0.5f);

      textManager.addText("Process Time (Last Frame): " + std::to_string(processTimeLast) + "ms", glm::vec2(1, 4.5f), 0.5f);
      textManager.addText("Process Rate (Last Frame): " + std::to_string(1000 / processTimeLast) + "fps", glm::vec2(1, 5), 0.5f);
      textManager.addText("Frame Time (Last Frame): " + std::to_string(frameTimeLast) + "ms", glm::vec2(1, 5.5f), 0.5f);
      textManager.addText("Frame Rate (Last Frame): " + std::to_string(1000 / frameTimeLast) + "fps", glm::vec2(1, 6), 0.5f);

      const auto dividerPositions = std::vector<double>({23.5f, 20.5f, 17.5f, 15.5f, 14, 13, 11.5f, 6.5f, 4});
      for (const auto &yPosition : dividerPositions)
      {
        textManager.addText("---------------", glm::vec2(1, yPosition), 0.5f);
      }

      // Check if debug text is enabled.
      updateStartTime = glfwGetTime();
      if (textEnabled)
      {
        textCharsRenderedLast = textManager.render();
      }
      updateEndTime = glfwGetTime();
      textRenderTimeLast = (updateEndTime - updateStartTime) * 1000;
      processTimeLast = (updateEndTime - currentTime) * 1000;

      // Swap the window framebuffers.
      windowManager.swapBuffers();

      updateEndTime = glfwGetTime();
      frameTimeLast = (updateEndTime - currentTime) * 1000;

      // Poll for window events.
      controlManager.pollEvents();

      // Continue loop as long as escape key isn't pressed or the window close is not requested.
    } while (
        getEnemyModelsCount() > 0 &&
        !controlManager.isKeyPressed(GLFW_KEY_ESCAPE) &&
        !windowManager.isWindowCloseRequested());

    modelManager.deinitAllModels();
    lightManager.deinitAllLights();
    cameraManager.deinitAllCameras();

    return "EndScene";
  }
};

#endif