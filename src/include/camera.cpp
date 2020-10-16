#ifndef INCLUDE_CAMERA_CPP
#define INCLUDE_CAMERA_CPP

#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "text.cpp"
#include "../camera/camera_base.cpp"

/**
 * A manager class for managing cameras in a scene.
 */
class CameraManager
{
private:
  // Singleton instance of the camera manager.
  static CameraManager instance;

  // The text manager responsible for rendering text.
  TextManager &textManager;

  // The map of registered cameras.
  std::map<const std::string, const std::shared_ptr<CameraBase>> registeredCameras;

  CameraManager()
      : textManager(TextManager::getInstance()),
        registeredCameras({}) {}

public:
  // Preventing copying the camera manager, making sure only one instance can exist.
  CameraManager(const CameraManager &) = delete;

  /**
   * Register a new camera into the camera manager.
   * 
   * @param camera  The camera to register.
   */
  void registerCamera(const std::shared_ptr<CameraBase> &&camera)
  {
    // Let the camera initialize itself.
    camera->init();
    // Insert the camera to the map of registered cameras.
    registeredCameras.emplace(camera->getCameraId(), std::move(camera));
  }

  /**
   * De-register an existing camera from the camera manager.
   * 
   * @param camera  The ID of the camera to de-register.
   */
  void deregisterCamera(const std::string &cameraId)
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
  void deregisterCamera(const std::shared_ptr<CameraBase> &camera)
  {
    // Remove the camera from the map of registered cameras.
    registeredCameras.erase(camera->getCameraId());
    // Let the camera de-initialize itself.
    camera->deinit();
  }

  /**
   * Return the camera registered with the given camera ID.
   * 
   * @param cameraId  The ID of the camera to return.
   * 
   * @return The camera registered with the given camera ID.
   */
  const std::shared_ptr<CameraBase> &getCamera(const std::string &cameraId) const
  {
    return registeredCameras.at(cameraId);
  }

  /**
   * Return the list of all cameras registered with the camera manager.
   * 
   * @return The list of all registered cameras.
   */
  const std::vector<std::shared_ptr<CameraBase>> getAllCameras() const
  {
    // Define a vector to store the list of registered cameras.
    std::vector<std::shared_ptr<CameraBase>> cameras({});
    // Iterate through the map of registered cameras.
    for (const auto &camera : registeredCameras)
    {
      // Push each registered camera into the cameras list.
      cameras.push_back(camera.second);
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
    // iterate through the map of registered cameras.
    for (const auto &camera : registeredCameras)
    {
      // Push the ID of each registered camera into the cameras ID list.
      registeredCameraIds.push_back(camera.first);
    }

    auto cameraNamesCount = std::map<const std::string, int>({});
    auto cameraNamesProcessTime = std::map<const std::string, double>({});

    // Iterate through the list of camera IDs.
    for (const auto &cameraId : registeredCameraIds)
    {
      // Find the camera registered with the given camera ID.
      const auto result = registeredCameras.find(cameraId);
      // Check if the camera still exists in the registration map.
      if (result != registeredCameras.end())
      {
        if (cameraNamesCount.find(result->second->getCameraName()) != cameraNamesCount.end())
        {
          cameraNamesCount[result->second->getCameraName()]++;
        }
        else
        {
          cameraNamesCount[result->second->getCameraName()] = 1;
          cameraNamesProcessTime[result->second->getCameraName()] = 0.0;
        }

        // If it does, tell the camera to perform an update on itself.
        const auto startTime = glfwGetTime();
        result->second->update();
        const auto endTime = glfwGetTime();
        cameraNamesProcessTime[result->second->getCameraName()] += (endTime - startTime) * 1000;
      }
    }

    auto height = 13.5;
    for (const auto &cameraCounts : cameraNamesCount)
    {
      const auto avgRenderTime = cameraNamesProcessTime[cameraCounts.first] / cameraCounts.second;
      textManager.addText(cameraCounts.first + " Camera Object Instances: " + std::to_string(cameraCounts.second) + " | Update (avg): " + std::to_string(avgRenderTime) + "ms", glm::vec2(1, height), 0.5);
      height -= 0.5;
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