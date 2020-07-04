#ifndef INCLUDE_WINDOW_CPP
#define INCLUDE_WINDOW_CPP

#include <iostream>
#include <string>

#include <GLFW/glfw3.h>
#include <GL/glew.h>

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
      exit(1);
    }

    glfwMakeContextCurrent(window);

    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK)
    {
      exit(1);
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
  }

public:
  WindowManager(WindowManager &) = delete;

  ~WindowManager() {}

  GLFWwindow *getWindow()
  {
    return window;
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

int WindowManager::width = 1024;
int WindowManager::height = 768;
WindowManager WindowManager::instance;

#endif