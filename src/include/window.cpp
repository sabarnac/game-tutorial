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
  // Singleton instance of the window manager.
  static WindowManager instance;

  // Is GLFW initialized.
  const bool isGlfwInitialized;
  // A pointer to the GLFW created window.
  GLFWwindow *const window;
  // Is GLEW initialized.
  const bool isGlewInitialized;

  /**
   * Initialize GLFW library.
   * 
   * @return Whether the GLFW was successfully initialized or not.
   */
  bool initializeGlfw()
  {
    // Initialize GLFW library.
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

    return true;
  }

  /**
   * Create a new GLFW window.
   * 
   * @return Pointer to the newly created window.
   */
  GLFWwindow *createWindow()
  {
    // Create a new window.
    const auto newWindow = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Game Tutorial", nullptr, nullptr);

    // Check if the window creation was successful.
    if (newWindow == NULL)
    {
      // Failed to create. Time to crash.
      std::cout << "Failed at window 2" << std::endl;
      exit(1);
    }

    // Set the newly created window as the current context in GLFW.
    glfwMakeContextCurrent(newWindow);

    // Get the size of the viewport of the window (this should be the same as the window width, but on MacOS it is float_t).
    glfwGetFramebufferSize(newWindow, &VIEWPORT_WIDTH, &VIEWPORT_HEIGHT);
    // Define the framebuffer width as float_t the viewport width.
    FRAMEBUFFER_WIDTH = VIEWPORT_WIDTH;
    // Define the framebuffer height as float_t the viewport width (so that framebuffer is a square).
    FRAMEBUFFER_HEIGHT = VIEWPORT_WIDTH;
    // Define the text height as 1/26 of the viewport height (so we can fit approx 20 lines in the screen).
    TEXT_HEIGHT = VIEWPORT_HEIGHT / 26;
    // Define the text width as 1/80 of the viewport width (so we can fit approx 80 characters per line in the screen).
    TEXT_WIDTH = VIEWPORT_WIDTH / 80;

    // Set the option for sticky keys on the window to true. This means keys will remain in the pressed state until they're processed.
    // This allows us to catch key presses that may be missed if we didn't poll in time.
    glfwSetInputMode(newWindow, GLFW_STICKY_KEYS, true);
    glfwSetInputMode(newWindow, GLFW_STICKY_MOUSE_BUTTONS, true);

    // Set what the interval is for swapping buffers. A value of zero means the swap should be immediate.
    // A value of 1 means that a single screen refresh should occur before swapping buffers.
    glfwSwapInterval(SWAP_INTERVAL);

    // Return the newly created window
    return newWindow;
  }

  /**
   * Initialize GLEW library.
   * 
   * @return Whether the GLEW was successfully initialized or not.
   */
  bool initializeGlew()
  {
    // Setup GLEW as experimental mode so that we can initialize the core OpenGL profile.
    glewExperimental = true;
    // Initialize GLEW.
    if (glewInit() != GLEW_OK)
    {
      // Failed to initialize. Time to crash.
      std::cout << "Failed at window 3" << std::endl;
      exit(1);
    }

    int32_t numberOfExtensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numberOfExtensions);
    std::set<std::string> supportedExtensions;
    for (int32_t i = 0; i < numberOfExtensions; i++)
    {
      const auto extensionName = (const char *)glGetStringi(GL_EXTENSIONS, i);
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
    glCullFace(GL_BACK);

    return true;
  }

  WindowManager() : isGlfwInitialized(initializeGlfw()),
                    window(createWindow()),
                    isGlewInitialized(initializeGlew())
  {
  }

public:
  // Preventing copying the window manager, making sure only one instance can exist.
  WindowManager(const WindowManager &) = delete;

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
  GLFWwindow *getWindow() const
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
   * Toggle what the interval is for swapping buffers.
   * * A value of 0 means the swap should be immediate.
   * * A value of 1 means that a single screen refresh should occur before swapping buffers. 
   * * A value of 2 means that two screen refreshes should occur before swapping buffers. 
   */
  void toggleVsync()
  {
    SWAP_INTERVAL = SWAP_INTERVAL == 0 ? 2 : SWAP_INTERVAL == 2 ? 1 : 0;
    glfwSwapInterval(SWAP_INTERVAL);
  }

  /**
   * Set the color to be used to clear the screen/framebuffer.
   * 
   * @param color  The new color.
   */
  void setClearColor(const glm::vec4 &color)
  {
    glClearColor(color.r, color.g, color.b, color.a);
  }

  /**
   * Clear the screen/framebuffer
   * 
   * @param mask  What buffers to clear (color/depth/stencil).
   */
  void clearScreen(const GLbitfield &mask)
  {
    glClear(mask);
  }

  void enableBlending(const GLenum sFactor, const GLenum dFactor)
  {
    glDisable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(sFactor, dFactor);
  }

  void disableBlending()
  {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glDisable(GL_BLEND);
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
   * 
   * @return Whether the window has been requested to be closed or not.
   */
  bool isWindowCloseRequested() const
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

// Initialize the window manager singleton instance static variable.
WindowManager WindowManager::instance;

#endif