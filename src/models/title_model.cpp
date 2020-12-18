#ifndef MODELS_TITLE_MODEL_CPP
#define MODELS_TITLE_MODEL_CPP

#include <string>
#include <memory>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "model_base.cpp"

/**
 * Class that represents a title model.
 */
class TitleModel : public ModelBase<TitleModel>
{
public:
  TitleModel(const std::string &modelId)
      : ModelBase(
            modelId,
            glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.9f, 0.24f, 1.0f),
            ColliderShapeType::BOX) {}

  static void initModel()
  {
    ModelBase::initModelDeps(
        "Title",
        "assets/objects/title.obj",
        "assets/textures/title.bmp",
        "assets/shaders/vertex/unlit.glsl", "assets/shaders/fragment/unlit.glsl");
  }

  static void deinitModel()
  {
    ModelBase::deinitModelDeps();
  }

  /**
   * Creates a new instance of the title model.
   */
  const static std::shared_ptr<TitleModel> create(const std::string &modelId)
  {
    return std::make_shared<TitleModel>(modelId);
  }

  void update() override
  {
  }
};

#endif