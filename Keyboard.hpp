#pragma once

#include <GLFW/glfw3.h>

class Keyboard
{
public:
    Keyboard() = delete;

    enum class KeyStatus : bool { PRESSED = true, RELEASED = false };

    //==========================================================================
    // Setter GLFW callback function
    static void setKey(GLFWwindow * window, int key, int scancode, int action, int mods)
    {
        // if unknown key do nothing
        if (key < 0 || key >= GLFW_KEY_LAST) return;

        // Set pressed keys
        if (action == GLFW_PRESS)
            s_keys[key] = KeyStatus::PRESSED;
        else if (action == GLFW_RELEASE)
            s_keys[key] = KeyStatus::RELEASED;

        // Quit on escape
        if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
            glfwSetWindowShouldClose(window, GL_TRUE);
    }

    //==========================================================================
    static KeyStatus getKeyStatus(int key)
    {
        if (key < 0 || key >= GLFW_KEY_LAST)
            return KeyStatus::RELEASED;

        return s_keys[key];
    }

private:
    static_assert(GLFW_KEY_LAST > 0);
    static KeyStatus s_keys[GLFW_KEY_LAST];

};