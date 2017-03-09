#include "Window.hpp"

#include <GL/gl3w.h>
#include "Keyboard.hpp"
#include <stdexcept>

//==============================================================================
Window::Window()
{
    // Initialize GLFW
    if (glfwInit() != GL_TRUE) throw std::runtime_error("Failed to initialize GLFW.");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_SAMPLES, 16);

    // Create window
    m_window = glfwCreateWindow(720, 720, "The Window :)", nullptr, nullptr);
    if (m_window == nullptr) throw std::runtime_error("Failed to create window.");
    glfwMakeContextCurrent(m_window);

    glfwSwapInterval(1);

    // Initialize GL3W
    if (gl3wInit() != 0) throw std::runtime_error("Failed to initialize GL3W.");
    if (gl3wIsSupported(3, 1) != 1) throw std::runtime_error("OpenGL 3.1 is not supported.");

    glfwSetKeyCallback(m_window, Keyboard::setKey);

    // Init mouse
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // OpenGL settings
    glfwGetFramebufferSize(m_window, &fb_width, &fb_height);
    glViewport(0, 0, fb_width, fb_height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

//==============================================================================
Window::~Window()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

//==============================================================================
void Window::makeContextCurrent()
{
    glfwMakeContextCurrent(m_window);
}

//==============================================================================
double Window::aspectRatio() const
{
    return static_cast<double>(fb_width) / static_cast<double>(fb_height);
}

//==============================================================================
void Window::swapResizeClearBuffer()
{
    glfwSwapBuffers(m_window);

    glfwGetFramebufferSize(m_window, &fb_width, &fb_height);
    glViewport(0, 0, fb_width, fb_height);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

//==============================================================================
bool Window::exitRequested()
{
    return glfwWindowShouldClose(m_window) != 0;
}

//==============================================================================
void Window::scheduleExit()
{
    glfwSetWindowShouldClose(m_window, GL_TRUE);
}

//==============================================================================
void Window::pollEvents()
{
    glfwPollEvents();
}
