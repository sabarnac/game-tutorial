#ifndef INCLUDE_LIGHT_CPP
#define INCLUDE_LIGHT_CPP

#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "render.cpp"
#include "../light/light_base.cpp"

class LightManager
{
private:
  static LightManager instance;

  RenderManager &renderManager;

  std::shared_ptr<LightBase> deadSimpleLight;
  std::shared_ptr<LightBase> deadCubeLight;

  std::map<std::string, std::shared_ptr<LightBase>> registeredLights;

  LightManager()
      : renderManager(RenderManager::getInstance()),
        registeredLights({}) {}

public:
  LightManager(LightManager &) = delete;

  void registerLight(std::shared_ptr<LightBase> light)
  {
    registeredLights.insert(std::pair<std::string, std::shared_ptr<LightBase>>(light->getLightId(), light));
    renderManager.registerLight(light);
    light->init();
  }

  void registerDeadSimpleLight(std::shared_ptr<LightBase> light)
  {
    deadSimpleLight = light;
    renderManager.registerDeadSimpleLight(deadSimpleLight);
  }

  void registerDeadCubeLight(std::shared_ptr<LightBase> light)
  {
    deadCubeLight = light;
    renderManager.registerDeadCubeLight(deadCubeLight);
  }

  void deregisterLight(std::shared_ptr<LightBase> light)
  {
    registeredLights.erase(light->getLightId());
    renderManager.deregisterLight(light);
    light->deinit();
  }

  void deregisterLight(std::string lightId)
  {
    auto light = registeredLights[lightId];
    registeredLights.erase(lightId);
    renderManager.deregisterLight(lightId);
    light->deinit();
  }

  std::shared_ptr<LightBase> getModel(std::string lightId)
  {
    return registeredLights[lightId];
  }

  std::vector<std::shared_ptr<LightBase>> getAllLights()
  {
    std::vector<std::shared_ptr<LightBase>> lights({});
    for (auto light = registeredLights.begin(); light != registeredLights.end(); light++)
    {
      lights.push_back(light->second);
    }
    return lights;
  }

  void updateAllLights()
  {
    std::vector<std::string> registeredLightIds({});
    for (auto light = registeredLights.begin(); light != registeredLights.end(); light++)
    {
      registeredLightIds.push_back(light->first);
    }

    for (auto lightId = registeredLightIds.begin(); lightId != registeredLightIds.end(); lightId++)
    {
      auto result = registeredLights.find(*(lightId));
      if (result != registeredLights.end())
      {
        result->second->update();
      }
    }
  }

  static LightManager &getInstance()
  {
    return instance;
  }
};

LightManager LightManager::instance;

#endif