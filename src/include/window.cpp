#ifndef INCLUDE_WINDOW_CPP
#define INCLUDE_WINDOW_CPP

#include <iostream>
#include <string>

#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "constants.cpp"

class CursorPosition
{
private:
  double x;
  double y;

public:
  CursorPosition(double x, double y) : x(x), y(y) {}

  double getX()
  {
    return x;
  }

  double getY()
  {
    return y;
  }
};

class WindowManager
{
private:
  static int width;
  static int height;
  static WindowManager instance;

  GLFWwindow *window;

  WindowManager()
  {
    if (!glfwInit())
    {
      std::cout << "Failed at window 1" << std::endl;
      exit(1);
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "OpenGL Tutorial", nullptr, nullptr);
    if (window == NULL)
    {
      std::cout << "Failed at window 2" << std::endl;
      exit(1);
    }

    glfwMakeContextCurrent(window);

    glfwGetFramebufferSize(window, &VIEWPORT_WIDTH, &VIEWPORT_HEIGHT);

    FRAMEBUFFER_WIDTH = VIEWPORT_WIDTH * 2;
    FRAMEBUFFER_HEIGHT = VIEWPORT_HEIGHT * 2;

    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK)
    {
      std::cout << "Failed at window 3" << std::endl;
      exit(1);
    }

    glViewport(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);

    glfwSwapInterval(0);
  }

public:
  WindowManager(WindowManager &) = delete;

  ~WindowManager() {}

  GLFWwindow *getWindow()
  {
    return window;
  }

  void switchToWindowViewport()
  {
    glViewport(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
  }

  void switchToFrameBufferViewport()
  {
    glViewport(0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_WIDTH);
  }

  void setClearColor(glm::vec4 color)
  {
    glClearColor(color.r, color.g, color.b, color.a);
  }

  void clearScreen(GLbitfield mask)
  {
    glClear(mask);
  }

  void pollEvents()
  {
    glfwPollEvents();
  }

  void swapBuffers()
  {
    glfwSwapBuffers(window);
  }

  bool isWindowCloseRequested()
  {
    return glfwWindowShouldClose(window) != 0;
  }

  static WindowManager &getInstance()
  {
    return instance;
  }
};

int WindowManager::width = WINDOW_WIDTH;
int WindowManager::height = WINDOW_HEIGHT;
WindowManager WindowManager::instance;

#endif