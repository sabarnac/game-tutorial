#ifndef INCLUDE_CONTROL_CPP
#define INCLUDE_CONTROL_CPP

#include <iostream>
#include <string>
#include <memory>

#include <GLFW/glfw3.h>
#include <GL/glew.h>

#include "constants.cpp"
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
    glfwSetCursorPos(windowManager.getWindow(), newPosition.getX() * width, newPosition.getY() * height);
  }

  bool isKeyPressed(int key)
  {
    return glfwGetKey(windowManager.getWindow(), key) == GLFW_PRESS;
  }
};

int ControlManager::width = WINDOW_WIDTH;
int ControlManager::height = WINDOW_HEIGHT;
ControlManager ControlManager::instance;

#endif