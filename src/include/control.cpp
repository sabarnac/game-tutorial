#ifndef INCLUDE_CONTROL_CPP
#define INCLUDE_CONTROL_CPP

#include <iostream>
#include <string>

#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <glm/gtx/string_cast.hpp>

#include "window.cpp"

class ControlManager
{
private:
  static int width;
  static int height;
  static ControlManager instance;

  WindowManager &windowManager;

public:
  ControlManager(ControlManager &) = delete;

  ControlManager() : windowManager(WindowManager::getInstance()) {}

  static ControlManager &getInstance()
  {
    return instance;
  }

  std::shared_ptr<CursorPosition> getCursorPosition()
  {
    double x, y;
    glfwGetCursorPos(windowManager.getWindow(), &x, &y);
    return std::make_shared<CursorPosition>(x / width, y / height);
  }

  void setCursorPosition(CursorPosition newPosition)
  {
    std::cout << "[ControlManager] "
              << "Updating cursor position (" << newPosition.getX() * width << " | " << newPosition.getY() * height << ")" << std::endl;
    glfwSetCursorPos(windowManager.getWindow(), newPosition.getX() * width, newPosition.getY() * height);
  }

  bool isKeyPressed(int key)
  {
    std::cout << "[ControlManager] "
              << "Checking if key is pressed: " << key << std::endl;
    return glfwGetKey(windowManager.getWindow(), key) == GLFW_PRESS;
  }
};

int ControlManager::width = 1024;
int ControlManager::height = 768;
ControlManager ControlManager::instance;

#endif