#ifndef SCENES_MAIN_MENU_SCENE_CPP
#define SCENES_MAIN_MENU_SCENE_CPP

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
#include "../include/render.cpp"
#include "../include/debug_render.cpp"
#include "../include/text.cpp"

#include "../camera/orthographic_camera.cpp"
#include "../models/dummy_enemy_model.cpp"
#include "../models/dummy_player_model.cpp"
#include "../models/dummy_shot_model.cpp"

#include "../models/title_model.cpp"
#include "../models/start_model.cpp"
#include "../models/exit_model.cpp"
#include "../models/cursor_model.cpp"

#include "scene_base.cpp"

class MainMenuScene : public SceneBase
{
private:
  ControlManager &controlManager;
  ModelManager &modelManager;
  CameraManager &cameraManager;
  RenderManager &renderManager;
  DebugRenderManager &debugRenderManager;

  std::vector<std::string> sceneCameraIds;
  std::vector<std::string> sceneModelIds;

  std::shared_ptr<StartModel> startModel;
  std::shared_ptr<ExitModel> exitModel;

  void initCameras()
  {
    // Create a perspective camera, and set its properties.
    const auto cameraId = "MainCamera";
    sceneCameraIds.push_back(cameraId);

    const auto orthographicCamera = OrthographicCamera::create(cameraId);
    cameraManager.registerCamera(orthographicCamera);
    renderManager.registerActiveCamera(orthographicCamera->getCameraId());

    orthographicCamera->setCameraPosition(glm::vec3(0.0f, 0.0f, 5.0f));
    orthographicCamera->setCameraAngles(glm::pi<float_t>(), 0.0f);
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
    const auto enemyModelId = "Enemy";
    sceneModelIds.push_back(enemyModelId);

    const auto enemyModel = DummyEnemyModel::create(enemyModelId);
    modelManager.registerModel(enemyModel);
    enemyModel->setModelPosition(glm::vec3(-0.3f, 0.2f, 0.0f));
  }

  void initPlayerModels()
  {
    // Create a player model.
    const auto playerModelId = "MainPlayer";
    sceneModelIds.push_back(playerModelId);

    const auto playerModel = DummyPlayerModel::create(playerModelId);
    modelManager.registerModel(playerModel);
    playerModel->setModelPosition(glm::vec3(0.0f, 0.2f, 0.0f));
  }

  void initShotModels()
  {
    // Create a shot model.
    const auto shotModelId = "Shot";
    sceneModelIds.push_back(shotModelId);

    const auto shotModel = DummyShotModel::create(shotModelId);
    modelManager.registerModel(shotModel);
    shotModel->setModelPosition(glm::vec3(0.3f, 0.3f, 0.0f));
  }

  void initTitleAndButtonModels()
  {
    {
      // Create a title model.
      const auto titleModelId = "Title";
      sceneModelIds.push_back(titleModelId);

      const auto titleModel = TitleModel::create(titleModelId);
      modelManager.registerModel(titleModel);
      titleModel->setModelPosition(glm::vec3(0.0f, 0.7f, 0.0f));
    }
    {
      // Create a start button model.
      const auto startModelId = "Start";
      sceneModelIds.push_back(startModelId);

      startModel = StartModel::create(startModelId);
      modelManager.registerModel(startModel);
      startModel->setModelPosition(glm::vec3(0.0f, -0.15f, 0.0f));
    }
    {
      // Create a exit button model.
      const auto exitModelId = "Exit";
      sceneModelIds.push_back(exitModelId);

      exitModel = ExitModel::create(exitModelId);
      modelManager.registerModel(exitModel);
      exitModel->setModelPosition(glm::vec3(0.0f, -0.7f, 0.0f));
    }
    {
      // Create a cursor model.
      const auto cursorModelId = "Cursor";
      sceneModelIds.push_back(cursorModelId);

      const auto cursorModel = CursorModel::create(cursorModelId);
      modelManager.registerModel(cursorModel);
      cursorModel->setModelPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    }
  }

  void initModels()
  {
    TitleModel::initModel();
    renderLoadingText("Loading (15%)", glm::vec2(1, 1), 1.0f);
    StartModel::initModel();
    ExitModel::initModel();
    renderLoadingText("Loading (20%)", glm::vec2(1, 1), 1.0f);
    CursorModel::initModel();
    renderLoadingText("Loading (25%)", glm::vec2(1, 1), 1.0f);
    DummyEnemyModel::initModel();
    renderLoadingText("Loading (40%)", glm::vec2(1, 1), 1.0f);
    DummyPlayerModel::initModel();
    renderLoadingText("Loading (55%)", glm::vec2(1, 1), 1.0f);
    DummyShotModel::initModel();
    renderLoadingText("Loading (70%)", glm::vec2(1, 1), 1.0f);

    initEnemyModels();
    renderLoadingText("Loading (85%)", glm::vec2(1, 1), 1.0f);
    initPlayerModels();
    renderLoadingText("Loading (90%)", glm::vec2(1, 1), 1.0f);
    initShotModels();
    renderLoadingText("Loading (95%)", glm::vec2(1, 1), 1.0f);
    initTitleAndButtonModels();
  }

  void deinitModels()
  {
    for (const auto &modelId : sceneModelIds)
    {
      modelManager.deregisterModel(modelId);
    }

    TitleModel::deinitModel();
    StartModel::deinitModel();
    ExitModel::deinitModel();
    CursorModel::deinitModel();
    DummyEnemyModel::deinitModel();
    DummyPlayerModel::deinitModel();
    DummyShotModel::deinitModel();
  }

public:
  MainMenuScene(const std::string &sceneId)
      : SceneBase(sceneId, "MainMenuScene"),
        controlManager(ControlManager::getInstance()),
        modelManager(ModelManager::getInstance()),
        cameraManager(CameraManager::getInstance()),
        renderManager(RenderManager::getInstance()),
        debugRenderManager(DebugRenderManager::getInstance())
  {
    sceneModelIds = std::vector<std::string>({});
    sceneCameraIds = std::vector<std::string>({});
  }

  const static std::shared_ptr<MainMenuScene> create(const std::string &sceneId)
  {
    return std::make_shared<MainMenuScene>(sceneId);
  }

  const void init()
  {
    renderLoadingText("Loading (0%)", glm::vec2(1, 1), 1.0f);
    initCameras();
    renderLoadingText("Loading (10%)", glm::vec2(1, 1), 1.0f);
    initModels();
    renderLoadingText("Loading (99%)", glm::vec2(1, 1), 1.0f);

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

  const std::optional<std::string> execute()
  {
    cameraManager.initAllCameras();
    modelManager.initAllModels();

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
      textManager.addText("Text Dimensions: " + std::to_string(TEXT_WIDTH) + "x" + std::to_string(TEXT_HEIGHT) + "px", glm::vec2(1, 9.5f), 0.5f);
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

      if (startModel->isClicked())
      {
        return "GameScene";
      }
      if (exitModel->isClicked())
      {
        break;
      }

      // Render the scene.
      updateStartTime = glfwGetTime();
      windowManager.enableBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      renderManager.render();
      windowManager.disableBlending();
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
    } while (!controlManager.isKeyPressed(GLFW_KEY_ESCAPE) &&
             !windowManager.isWindowCloseRequested());

    cameraManager.deinitAllCameras();
    modelManager.deinitAllModels();

    return std::nullopt;
  }
};

#endif