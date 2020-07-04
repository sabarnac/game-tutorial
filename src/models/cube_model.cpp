#ifndef MODELS_CUBE_MODEL_CPP
#define MODELS_CUBE_MODEL_CPP

#include <string>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "../include/models.cpp"

class CubeModel : public ModelBase
{

public:
  CubeModel()
      : ModelBase(
            "Cube",
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(1.0f, 1.0f, 1.0f),
            "assets/objects/cube.obj",
            "assets/textures/uvtemplate.bmp", TextureType::BMP,
            "assets/shaders/vertex/default.glsl", "assets/shaders/fragment/default.glsl")
  {
    std::cout << "[CubeModel] "
              << "Constructing CubeModel." << std::endl;
    std::cout << "[CubeModel] "
              << "Constructed CubeModel." << std::endl;
  }

  static std::shared_ptr<CubeModel> create()
  {
    std::cout << "[CubeModel] "
              << "Creating new cube model." << std::endl;

    return std::make_shared<CubeModel>();
  };

  void update()
  {
    std::cout << "[CubeModel] "
              << "Updating cube model attributes: " << getModelName() << std::endl;
    std::cout << "[CubeModel] "
              << "Updated cube model attributes." << std::endl;
  }
};

#endif