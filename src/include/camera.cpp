#ifndef INCLUDE_CAMERA_CPP
#define INCLUDE_CAMERA_CPP

#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "render.cpp"
#include "../camera/camera_base.cpp"

class CameraManager
{
private:
  static CameraManager instance;

  RenderManager &renderManager;

  std::map<std::string, std::shared_ptr<CameraBase>> registeredCameras;

  CameraManager()
      : renderManager(RenderManager::getInstance()),
        registeredCameras({}) {}

public:
  CameraManager(CameraManager &) = delete;

  void registerCamera(std::shared_ptr<CameraBase> camera)
  {
    registeredCameras.insert(std::pair<std::string, std::shared_ptr<CameraBase>>(camera->getCameraId(), camera));
    renderManager.registerCamera(camera);
    renderManager.registerActiveCamera(camera);
    camera->init();
  }

  void deregisterCamera(std::shared_ptr<CameraBase> camera)
  {
    registeredCameras.erase(camera->getCameraId());
    renderManager.deregisterCamera(camera);
    camera->deinit();
  }

  void deregisterCamera(std::string cameraId)
  {
    auto camera = registeredCameras[cameraId];
    registeredCameras.erase(cameraId);
    renderManager.deregisterCamera(cameraId);
    camera->deinit();
  }

  std::shared_ptr<CameraBase> getModel(std::string cameraId)
  {
    return registeredCameras[cameraId];
  }

  std::vector<std::shared_ptr<CameraBase>> getAllCameras()
  {
    std::vector<std::shared_ptr<CameraBase>> cameras({});
    for (auto camera = registeredCameras.begin(); camera != registeredCameras.end(); camera++)
    {
      cameras.push_back(camera->second);
    }
    return cameras;
  }

  void updateAllCameras()
  {
    std::vector<std::string> registeredCameraIds({});
    for (auto camera = registeredCameras.begin(); camera != registeredCameras.end(); camera++)
    {
      registeredCameraIds.push_back(camera->first);
    }

    for (auto cameraId = registeredCameraIds.begin(); cameraId != registeredCameraIds.end(); cameraId++)
    {
      auto result = registeredCameras.find(*(cameraId));
      if (result != registeredCameras.end())
      {
        result->second->update();
      }
    }
  }

  static CameraManager &getInstance()
  {
    return instance;
  }
};

CameraManager CameraManager::instance;

#endif