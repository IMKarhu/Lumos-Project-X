#pragma once
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <string>

#ifdef _WIN32
    #ifdef CORE_EXPORT
    #define CORE_API __declspec(dllexport)
    #else
    #define CORE_API __declspec(dllimport)
    #endif
#else
    #define CORE_API
#endif

namespace Lumos
{
    class CORE_API Window
    {
        public:
            Window();
            Window(uint32_t width, uint32_t height, std::string title);
            Window(const Window& win) = delete;
            Window& operator=(const Window& win) = delete;
            ~Window();

            GLFWwindow *handle() const;
            uint32_t width() const;
            uint32_t height() const;
            bool framebufferResized() const;
            void framebufferSize(int& width, int& height);

            bool shouldClose() const;
            void setWindowShouldClose(bool close);
            void setWindowTitle(const std::string& title);
            void setFramebufferResized(bool resized);
            void setUserPtr(void* ptr);
            void pollEvents();
            void waitEvents();
            void swapBuffers();
            void makeCtxCurrent(GLFWwindow *ctx);
            void initWindow(uint32_t width, uint32_t height, std::string title);
        private:
            GLFWwindow *m_window = nullptr;
            struct WindowData
            {
                uint32_t m_width = 0;
                uint32_t m_height = 0;
                std::string m_title = "";
                bool m_framebufferresized = false;
            } m_windowdata;

            static void framebufferCallback(GLFWwindow *window, int width, int height);
            static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
            static void cursorCallback(GLFWwindow* window, double xpos, double ypos);
            static void mouseClickCallback(GLFWwindow* window, int button, int action, int mods);
    };
}
