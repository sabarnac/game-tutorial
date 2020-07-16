#ifndef INCLUDE_WINDOW_CPP
#define INCLUDE_WINDOW_CPP

#include <iostream>
#include <set>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "constants.cpp"

/**
 * A class to manage the window.
 */
class WindowManager
{
private:
  // The width of the window.
  static int width;
  // The height of the window.
  static int height;

  // Singleton instance of the window manager.
  static WindowManager instance;

  // A pointer to the GLFW created window.
  GLFWwindow *window;

  WindowManager()
  {
    // Initialize GLFW library
    if (!glfwInit())
    {
      // Failed to initialize. Time to crash.
      std::cout << "Failed at window 1" << std::endl;
      exit(1);
    }

    // Set up OpenGL window hints for creating an OpenGL context.
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Because MacOS.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a window.
    window = glfwCreateWindow(width, height, "Game Tutorial", nullptr, nullptr);
    // Check if the window creation was successful.
    if (window == NULL)
    {
      // Failed to create. Time to crash.
      std::cout << "Failed at window 2" << std::endl;
      exit(1);
    }

    // Set the newly created window as the current context in GLFW.
    glfwMakeContextCurrent(window);

    // Get the size of the viewport of the window (this should be the same as the window width, but on MacOS it is double).
    glfwGetFramebufferSize(window, &VIEWPORT_WIDTH, &VIEWPORT_HEIGHT);

    // Set the option for sticky keys on the window to true. This means keys will remain in the pressed state until they're processed.
    // This allows us to catch key presses that may be missed if we didn't poll in time.
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Set the option to disable the cursor when the window is active, preventing the user from moving the cursor out of the window,
    //   and accidentally clicking something else.
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set what the interval is for swapping buffers. A value of zero means the swap should be immediate.
    // A value of 1 means that a single screen refresh should occur before swapping buffers.
    glfwSwapInterval(0);

    // Define the framebuffer width as double the viewport width.
    FRAMEBUFFER_WIDTH = VIEWPORT_WIDTH;
    // Define the framebuffer height as double the viewport width (so that framebuffer is a square).
    FRAMEBUFFER_HEIGHT = VIEWPORT_WIDTH;

    // Setup glew as experimental mode so that we can initialize the core OpenGL profile.
    glewExperimental = true; // Needed for core profile
    // Initialize GLEW.
    if (glewInit() != GLEW_OK)
    {
      // Failed to initialize. Time to crash.
      std::cout << "Failed at window 3" << std::endl;
      exit(1);
    }

    int numberOfExtensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numberOfExtensions);
    std::set<std::string> supportedExtensions;
    for (int i = 0; i < numberOfExtensions; i++)
    {
      auto extensionName = (const char *)glGetStringi(GL_EXTENSIONS, i);
      supportedExtensions.insert(std::string(extensionName));
    }

    // Check if cube map array textures are supported
    if (supportedExtensions.find("GL_ARB_texture_cube_map_array") == supportedExtensions.end())
    {
      // Not supported. Time to crash.
      std::cout << "Failed at window 4" << std::endl;
      exit(1);
    }

    // Set the viewport width to the values we got from GLFW.
    glViewport(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    // Set the color to use when clearing the screen/framebuffer.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // Enable depth testing of the fragments, allowing the GPU to drop fragments from a single object and not process them if a condition is satisfied.
    glEnable(GL_DEPTH_TEST);
    // Use the less than function for depth testing fragments. This means any fragment of an object that is behind another fragment of the same object is dropped.
    glDepthFunc(GL_LESS);

    // Enable culling of faces/polygons. This means that any face/polygon that is behind another polygon within the same object is dropped.
    glEnable(GL_CULL_FACE);
  }

public:
  // Preventing copying the window manager, making sure only one instance can exist.
  WindowManager(WindowManager &) = delete;

  ~WindowManager()
  {
    // Destroy the GLFW window on application termination.
    glfwDestroyWindow(window);
  }

  /**
   * Return the window pointer.
   * 
   * @return The window pointer.
   */
  GLFWwindow *getWindow()
  {
    return window;
  }

  /**
   * Set the viewport to the size of the window viewport.
   */
  void switchToWindowViewport()
  {
    glViewport(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
  }

  /**
   * Set the viewport to the size of the framebuffer.
   */
  void switchToFrameBufferViewport()
  {
    glViewport(0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
  }

  /**
   * Set the color to be used to clear the screen/framebuffer.
   * 
   * @param color  The new color.
   */
  void setClearColor(glm::vec4 color)
  {
    glClearColor(color.r, color.g, color.b, color.a);
  }

  /**
   * Clear the screen/framebuffer
   * 
   * @param mask  What buffers to clear (color/depth/stencil).
   */
  void clearScreen(GLbitfield mask)
  {
    glClear(mask);
  }

  /**
   * Swap the active framebuffer of the window to the one on which was drawn.
   */
  void swapBuffers()
  {
    glfwSwapBuffers(window);
  }

  /**
   * Check if a window termination was requested.
   */
  bool isWindowCloseRequested()
  {
    return glfwWindowShouldClose(window) != 0;
  }

  /**
   * Returns the singleton instance of the window manager.
   * 
   * @return The window manager singleton instance.
   */
  static WindowManager &getInstance()
  {
    return instance;
  }
};

// Initialize the window width to the WINDOW_WIDTH constant.
int WindowManager::width = WINDOW_WIDTH;
// Initialize the window height to the WINDOW_HEIGHT constant.
int WindowManager::height = WINDOW_HEIGHT;
// Initialize the window manager singleton instance static variable.
WindowManager WindowManager::instance;

#endif