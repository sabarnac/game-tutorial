#ifndef INCLUDE_CAMERA_CPP
#define INCLUDE_CAMERA_CPP

#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "render.cpp"
#include "../camera/camera_base.cpp"

/**
 * A manager class for managing cameras in a scene.
 */
class CameraManager
{
private:
  // Singleton instance of the camera manager.
  static CameraManager instance;

  // The render manager responsible for rendering objects.
  RenderManager &renderManager;

  // The map of registered cameras.
  std::map<std::string, std::shared_ptr<CameraBase>> registeredCameras;

  CameraManager()
      : renderManager(RenderManager::getInstance()),
        registeredCameras({}) {}

public:
  // Preventing copying the camera manager, making sure only one instance can exist.
  CameraManager(CameraManager &) = delete;

  /**
   * Register a new camera into the camera manager.
   * 
   * @param camera  The camera to register.
   */
  void registerCamera(std::shared_ptr<CameraBase> camera)
  {
    // Let the camera initialize itself.
    camera->init();
    // Insert the camera to the map of registered cameras.
    registeredCameras.insert(std::pair<std::string, std::shared_ptr<CameraBase>>(camera->getCameraId(), camera));
    // Register the camera with the render manager as well so that it can be used for rendering.
    renderManager.registerCamera(camera);
    // Set the camera as the active camera so that it's used to render to the window.
    renderManager.registerActiveCamera(camera);
  }

  /**
   * De-register an existing camera from the camera manager.
   * 
   * @param camera  The ID of the camera to de-register.
   */
  void deregisterCamera(std::string cameraId)
  {
    // Get the camera that is registered with the given camera ID.
    auto camera = registeredCameras[cameraId];
    // Remove the camera from the map of registered cameras.
    deregisterCamera(camera);
  }

  /**
   * De-register an existing camera from the camera manager.
   * 
   * @param camera  The camera to de-register.
   */
  void deregisterCamera(std::shared_ptr<CameraBase> camera)
  {
    // Remove the camera from the map of registered cameras.
    registeredCameras.erase(camera->getCameraId());
    // De-register the camera from the render manager as well.
    renderManager.deregisterCamera(camera);
    // Let the camera de-initialize itself.
    camera->deinit();
  }

  /**
   * Return the camera registered with the given camera ID.
   * 
   * @param camera  The ID of the camera to return.
   * 
   * @return The camera registered with the given camera ID.
   */
  std::shared_ptr<CameraBase> getCamera(std::string cameraId)
  {
    return registeredCameras[cameraId];
  }

  /**
   * Return the list of all cameras registered with the camera manager.
   * 
   * @return The list of all registered cameras.
   */
  std::vector<std::shared_ptr<CameraBase>> getAllCameras()
  {
    // Define a vector to store the list of registered cameras.
    std::vector<std::shared_ptr<CameraBase>> cameras({});
    // Iterate through the map of registered cameras.
    for (auto camera = registeredCameras.begin(); camera != registeredCameras.end(); camera++)
    {
      // Push each registered camera into the cameras list.
      cameras.push_back(camera->second);
    }
    // Return the list of registered cameras.
    return cameras;
  }

  /**
   * Run the update operation on all the registered cameras.
   */
  void updateAllCameras()
  {
    // Define a vector to store the IDs of the registered cameras.
    std::vector<std::string> registeredCameraIds({});
    // ITerate through the map of registered cameras.
    for (auto camera = registeredCameras.begin(); camera != registeredCameras.end(); camera++)
    {
      // Push the ID of each registered camera into the cameras ID list.
      registeredCameraIds.push_back(camera->first);
    }

    // Iterate through the list of camera IDs.
    for (auto cameraId = registeredCameraIds.begin(); cameraId != registeredCameraIds.end(); cameraId++)
    {
      // Find the camera registered with the given camera ID.
      auto result = registeredCameras.find(*(cameraId));
      // Check if the camera still exists in the registration map.
      if (result != registeredCameras.end())
      {
        // If it does, tell the camera to perform an update on itself.
        result->second->update();
      }
    }
  }

  /**
   * Returns the singleton instance of the camera manager.
   * 
   * @return The camera manager singleton instance.
   */
  static CameraManager &getInstance()
  {
    return instance;
  }
};

// Initialize the camera manager singleton instance static variable.
CameraManager CameraManager::instance;

#endif