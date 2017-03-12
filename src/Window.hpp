#pragma once

struct GLFWwindow;

class Window
{
public:

    Window();
    ~Window();

    void makeContextCurrent();

    double aspectRatio() const;

    void swapResizeClearBuffer();

    bool exitRequested();
    void scheduleExit();

    void pollEvents();

private:
    GLFWwindow * m_window;
    int fb_width;
    int fb_height;

};
