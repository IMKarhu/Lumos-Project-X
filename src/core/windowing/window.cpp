#include "window.h"
#include "events.h"

namespace Lumos
{
    Window::~Window()
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    };

    bool Window::initWindow(int width, int height, const std::string& title)
    {
        bool result;
        if(!glfwInit()) {
            throw std::runtime_error("Failed to initialize glfw");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if(!m_window) {
            glfwTerminate();
            throw std::runtime_error("Failed to create window");
        }
        glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, windowResizeCallback);
        glfwSetKeyCallback(m_window, keyCallback);
        glfwSetCursorPosCallback(m_window, cursorCallback);
        glfwSetMouseButtonCallback(m_window, mouseClickCallback);
        glfwSetScrollCallback(m_window, scrollcallback);
        glfwGetFramebufferSize(m_window, &m_width, &m_height);
        result = true;
        return result;
    }

    GLFWwindow *Window::handle() const
    {
        return m_window;
    }

    int Window::width() const
    {
        return m_width;
    }

    int Window::height() const
    {
        return m_height;
    }

    std::vector<const char*> Window::instanceExtensions()
    {
        uint32_t count = 0;
        const char** glfwextensions;
        glfwextensions = glfwGetRequiredInstanceExtensions(&count);
        std::vector<const char*> extensions(glfwextensions, glfwextensions + count);
        return extensions;
    }

    bool Window::shouldClose() const
    {
        return glfwWindowShouldClose(m_window);
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

    void Window::setWindowTitle(const std::string& title)
    {
        glfwSetWindowTitle(m_window, title.c_str());
    }

    void Window::setWindowShouldClose(bool close)
    {
        glfwSetWindowShouldClose(m_window, close);
    }

    void Window::setResizeCallback(std::function<void(int, int)> callback)
    {
        m_resizecallback = std::move(callback);
    }


    /*********PRIVATE********/
    void Window::windowResizeCallback(GLFWwindow *window, int width, int height)
    {
        auto* w = static_cast<Window*>(glfwGetWindowUserPointer(window));
        w->m_width = width;
        w->m_height = height;
        w->m_windowresized = true;
        if (w->m_resizecallback) {
            w->m_resizecallback(width, height);
        }
    }

    void Window::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        auto* eventbus = static_cast<EventBus*>(glfwGetWindowUserPointer(window));
        if (eventbus) {
            KeyPressEvent event;
            event.key = key;
            event.scancode = scancode;
            event.action = action;
            event.mods = mods;
            // eventbus->publish(event);
        }
    }

    void Window::cursorCallback(GLFWwindow* window, double xpos, double ypos)
    {
        auto* eventbus = static_cast<EventBus*>(glfwGetWindowUserPointer(window));
        static bool firstmouse = true;
        static float lastx = 0.0f;
        static float lasty = 0.0f;
        if (firstmouse) {
            lastx = xpos;
            lasty = ypos;
            firstmouse = false;
        }

        float xoffset = xpos - lastx;
        float yoffset = lasty - ypos;

        if (eventbus) {
            CursorMoveEvent event;
            event.x = xoffset;
            event.y = yoffset;
            // eventbus->publish(event);
            CursorMoveRawEvent rawevent;
            rawevent.x = xpos;
            rawevent.y = ypos;
            // eventbus->publish(rawevent);
        }
        lastx = xpos;
        lasty = ypos;
    }

    void Window::mouseClickCallback(GLFWwindow* window, int button, int action, int mods)
    {
        auto* eventbus = static_cast<EventBus*>(glfwGetWindowUserPointer(window));
        if (eventbus) {
            MouseClickEvent event;
            event.button = button;
            event.action = action;
            event.mods = mods;
            if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            // eventbus->publish(event);
        }
    }
    void Window::scrollcallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        auto* eventbus = static_cast<EventBus*>(glfwGetWindowUserPointer(window));
        // std::println("scrolling x and y offsets: x:{}    y:{}", xoffset, yoffset);
        if (eventbus) {
            std::println("helo");
            ScrollEvent event;
            event.xoffset = xoffset;
            event.yoffset = yoffset;
            // eventbus->publish(event);
        }
    }
}// namespace Lumos

