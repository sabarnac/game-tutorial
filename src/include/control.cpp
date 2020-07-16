#ifndef INCLUDE_CONTROL_CPP
#define INCLUDE_CONTROL_CPP

#include <iostream>
#include <string>
#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "constants.cpp"
#include "window.cpp"

/**
 * Class containing the normalized position of the cursor.
 */
class CursorPosition
{
private:
  // The x-coordinate of the cursor.
  double x;
  // The y-coordinate of the cursor.
  double y;

public:
  CursorPosition(double x, double y) : x(x), y(y) {}

  /**
   * Returns the x-coordinate of the vector.
   * 
   * @return The x-coordinate.
   */
  double getX()
  {
    return x;
  }

  /**
   * Returns the y-coordinate of the vector.
   * 
   * @return The y-coordinate.
   */
  double getY()
  {
    return y;
  }
};

/**
 * A class to manage controls and inputs of the window.
 */
class ControlManager
{
private:
  // The width of the window.
  static int width;
  // The height of the window.
  static int height;

  // Singleton instance of the control manager.
  static ControlManager instance;

  // The window manager responsible for managing the window and properties related to it.
  WindowManager &windowManager;

  ControlManager() : windowManager(WindowManager::getInstance()) {}

public:
  // Preventing copying the control manager, making sure only one instance can exist.
  ControlManager(ControlManager &) = delete;

  /**
   * Returns the position of the cursor on the window.
   * 
   * @return The cursor position.
   */
  std::shared_ptr<CursorPosition> getCursorPosition()
  {
    // Define variables for the x,y-coordinates of the cursor.
    double x, y;
    // Get the cursor position in the window from GLFW.
    glfwGetCursorPos(windowManager.getWindow(), &x, &y);
    // The cursor position is based on the size of the window, so normalize accordingly and return it.
    return std::make_shared<CursorPosition>(x / width, y / height);
  }

  /**
   * Set the position of the cursor on the window.
   * 
   * @param newPosition  The position that the cursor should be set to.
   */
  void setCursorPosition(CursorPosition newPosition)
  {
    // Tell GLFW to set the position of the cursor. GLFW requires the absolute position of the cursor on the window,
    //   but the input coordinates are normalized, so multiply them with the dimensions of the window.
    glfwSetCursorPos(windowManager.getWindow(), newPosition.getX() * width, newPosition.getY() * height);
  }

  /**
   * Checks whether a key has been pressed or not.
   * 
   * @param key  The key to check.
   * 
   * @return Whether the key has been pressed or not.
   */
  bool isKeyPressed(int key)
  {
    // Query GLFW to see the status of the key and return true if it is being pressed.
    return glfwGetKey(windowManager.getWindow(), key) == GLFW_PRESS;
  }

  /**
   * Poll for input/control events on the window.
   */
  void pollEvents()
  {
    glfwPollEvents();
  }

  /**
   * Returns the singleton instance of the control manager.
   * 
   * @return The control manager singleton instance.
   */
  static ControlManager &getInstance()
  {
    return instance;
  }
};

// Initialize the window width to the WINDOW_WIDTH constant.
int ControlManager::width = WINDOW_WIDTH;
// Initialize the window height to the WINDOW_HEIGHT constant.
int ControlManager::height = WINDOW_HEIGHT;
// Initialize the control manager singleton instance static variable.
ControlManager ControlManager::instance;

#endif