#include "window.h"
#include <print>

namespace Lumos
{
    Window::Window()
    {
        m_windowdata = {
            .m_width = 600,
            .m_height = 400,
            .m_title = "Lumos engine test."
        };

        if(!glfwInit()) {
            std::println("Failed to initialize GLFW");
            return ;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_window = glfwCreateWindow(m_windowdata.m_width, m_windowdata.m_height, m_windowdata.m_title.c_str(), nullptr, nullptr);
        if(!m_window) {
            std::println("failed to create window context");
            glfwTerminate();
            return ;
        }
        glfwMakeContextCurrent(m_window);
        // glfwSwapInterval(0);
        glfwSetWindowUserPointer(m_window, &m_windowdata);
        glfwSetFramebufferSizeCallback(m_window, framebufferCallback);
        glfwSetKeyCallback(m_window, keyCallback);

    }

    Window::Window(uint32_t width, uint32_t height, std::string title)
        :m_windowdata(width, height, title)
    {
        m_windowdata = {
            .m_width = width,
            .m_height = height,
            .m_title = title
        };

        if(!glfwInit()) {
            std::println("Failed to initialize GLFW");
            return ;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_window = glfwCreateWindow(m_windowdata.m_width, m_windowdata.m_height, m_windowdata.m_title.c_str(), nullptr, nullptr);
        if(!m_window) {
            std::println("failed to create window context");
            glfwTerminate();
            return ;
        }
        glfwMakeContextCurrent(m_window);
        // glfwSwapInterval(0);
        glfwSetWindowUserPointer(m_window, &m_windowdata);
        glfwSetFramebufferSizeCallback(m_window, framebufferCallback);
        glfwSetKeyCallback(m_window, keyCallback);

    }

    Window::~Window()
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    };

    GLFWwindow *Window::handle() const
    {
        return m_window;
    }

    uint32_t Window::width() const
    {
        return m_windowdata.m_width;
    }

    uint32_t Window::height() const
    {
        return m_windowdata.m_height;
    }

    bool Window::framebufferResized() const
    {
        return m_windowdata.m_framebufferresized;
    }
    void Window::framebufferSize(int& width, int& height)
    {
        glfwGetFramebufferSize(m_window, &width, &height);
    }

    bool Window::shouldClose() const
    {
        return glfwWindowShouldClose(m_window);
    }

    void Window::setWindowShouldClose(bool close)
    {
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
    }

    void Window::setWindowTitle(const std::string& title)
    {
        glfwSetWindowTitle(m_window, title.c_str());
    }

    void Window::setFramebufferResized(bool resized)
    {
        m_windowdata.m_framebufferresized = resized;
    }

    void Window::pollEvents()
    {
        glfwPollEvents();
    }

    void Window::waitEvents()
    {
        glfwWaitEvents();
    }

    void Window::swapBuffers()
    {
        glfwSwapBuffers(m_window);
    }

    void Window::makeCtxCurrent(GLFWwindow *ctx)
    {
        glfwMakeContextCurrent(ctx);
    }

    void Window::initWindow(uint32_t width, uint32_t height, std::string title)
    {
        if(!glfwInit()) {
            std::println("Failed to initialize GLFW");
            return ;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if(!m_window) {
            std::println("failed to create window");
            glfwTerminate();
            return ;
        }
        glfwMakeContextCurrent(m_window);
        glfwSetWindowUserPointer(m_window, &m_windowdata);
        glfwSetFramebufferSizeCallback(m_window, framebufferCallback);
        glfwSetKeyCallback(m_window, keyCallback);
    }

    void Window::framebufferCallback(GLFWwindow *window, int width, int height)
    {
        WindowData data = *(WindowData*)glfwGetWindowUserPointer(window);
        data.m_width = width;
        data.m_height = height;
        data.m_framebufferresized = true;
    }

    void Window::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        WindowData data = *(WindowData*)glfwGetWindowUserPointer(window);
        int state = glfwGetKey(window, key);
    }
}// namespace Lumos

