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
  CursorPosition(double x, double y) : x(x), y(y)
  {
    std::cout << "[CursorPosition] "
              << "Constructing CursorPosition." << std::endl;
    std::cout << "[CursorPosition] "
              << "Constructed CursorPosition." << std::endl;
  }

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
    std::cout << "[WindowManager] "
              << "Constructing WindowManager." << std::endl;

    std::cout << "[WindowManager] "
              << "Initializing GLFW." << std::endl;
    if (!glfwInit())
    {
      std::cerr << "[WindowManager] "
                << "Failed to initialize GLFW." << std::endl;
      exit(1);
    }

    std::cout << "[WindowManager] "
              << "Initializing GLFW window hints." << std::endl;
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    std::cout << "[WindowManager] "
              << "Creating GLFW window." << std::endl;
    // Open a window and create its OpenGL context
    window = glfwCreateWindow(width, height, "OpenGL Tutorial", nullptr, nullptr);
    if (window == NULL)
    {
      std::cerr << "[WindowManager] "
                << "Failed to create window." << std::endl;
      exit(1);
    }

    std::cout << "[WindowManager] "
              << "Set created window as the current context." << std::endl;
    glfwMakeContextCurrent(window);

    std::cout << "[WindowManager] "
              << "Initializing GLEW." << std::endl;
    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK)
    {
      std::cerr << "[WindowManager] "
                << "Failed to initialize GLEW." << std::endl;
      exit(1);
    }

    std::cout << "[WindowManager] "
              << "Setting input modes." << std::endl;
    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    std::cout << "[WindowManager] "
              << "Setting clear color." << std::endl;
    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    std::cout << "[WindowManager] "
              << "Enabling GL features." << std::endl;
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);

    std::cout << "[WindowManager] "
              << "Constructed WindowManager." << std::endl;
  }

public:
  WindowManager(WindowManager &) = delete;

  ~WindowManager()
  {
    std::cout << "[WindowManager] "
              << "Destroying WindowManager." << std::endl;
    glfwTerminate();
    std::cout << "[WindowManager] "
              << "Destroyed WindowManager." << std::endl;
  }

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
    std::cout << "[WindowManager] "
              << "Polling for new window and input events." << std::endl;
    glfwPollEvents();
  }

  void swapBuffers()
  {
    std::cout << "[WindowManager] "
              << "Swapping window render buffers." << std::endl;
    glfwSwapBuffers(window);
  }

  bool isWindowCloseRequested()
  {
    std::cout << "[WindowManager] "
              << "Checking if window closure was requested." << std::endl;
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