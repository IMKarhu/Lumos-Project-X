#pragma once
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <string>
#include <functional>

namespace Lumos
{
    class Window
    {
        public:
            Window() = default;
            Window(const Window& win) = delete;
            Window& operator=(const Window& win) = delete;
            ~Window();

            bool initWindow(int width, int height, const std::string& title);
            GLFWwindow *handle() const;
            int width() const;
            int height() const;
            std::vector<const char*> instanceExtensions();

            bool shouldClose() const;
            void pollEvents();
            void waitEvents();
            void swapBuffers();
            void setWindowTitle(const std::string& title);
            void setWindowShouldClose(bool close);
            void windowDimensions(int& width, int& height);
            void setResizeCallback(std::function<void(int, int)> callback);
            void setScrollCallback(std::function<void(double, double)> callback);
        private:
            GLFWwindow *m_window = nullptr;
            int m_width = 0;
            int m_height = 0;
            std::string m_title = "";
            bool m_windowresized = false;
            std::function<void(int, int)> m_resizecallback;
            std::function<void(int, bool)> m_keycallback;
            std::function<void(double, double)> m_cursorposcallback;
            std::function<void(int, int, int)> m_cursorclickcallback;
            std::function<void(double, double)> m_scrollcallback;

            static void windowResizeCallback(GLFWwindow *window, int width, int height);
            static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
            static void cursorCallback(GLFWwindow* window, double xpos, double ypos);
            static void mouseClickCallback(GLFWwindow* window, int button, int action, int mods);
            static void scrollcallback(GLFWwindow* window, double offset, double yoffset);
    };
}
