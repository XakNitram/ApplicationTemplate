module;
#include "pch.hpp"
export module Window;


namespace core {
    export enum class WindowStatus {
        Success, GLFWUninitialized, WindowCreationFailed, OpenGLContextCreationFailed
    };

    export class Window {
        static Window *get_state(GLFWwindow *state) {
            return static_cast<Window*>(glfwGetWindowUserPointer(state));
        }

        GLFWwindow *data;
    public:
        explicit operator GLFWwindow*() const {
            return data;
        }

        explicit Window(GLFWwindow* p_window) : data(p_window) {}
        ~Window() {
            glfwSetWindowUserPointer(data, nullptr);
            glfwDestroyWindow(data);
            glfwPollEvents();
        }
    };

    export void glfw_update() {
        glfwPollEvents();
    }

    export Window window_create(const int width, const int height, const char* title, WindowStatus& status) {
        using enum WindowStatus;
        if (!glfwInit()) {
            status = GLFWUninitialized;
            return Window{nullptr};
        }

        constexpr int glfw_enable[2] {GLFW_FALSE, GLFW_TRUE};

        // Set GLFW window hints.
        // ****** Hard Constraints ******
        // Must match available capabilities exactly.
        glfwWindowHint(GLFW_STEREO, GLFW_FALSE);  // Specifies whether to use OpenGL stereoscopic rendering.

        // Specifies whether the framebuffer should be double buffered. You nearly always want to use double buffering.
        // Investigate to see why this doesn't work when false.
        glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

        // Specifies which context creation API to use to create the context.
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        //glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        //glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        // An extension loader library that assumes it knows which API was used to create the current context ...
        //  may fail if you change this hint. This can be resolved by having it load functions via glfwGetProcAddress.
        // Maybe have to change for linux.
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
        //glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_OSMESA_CONTEXT_API);
        //glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);

        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);  // Disable deprecated OpenGL operations.

        // Specifies which OpenGL profile to create the context for.
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
        //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);

        // ****** Soft Constraints ******
        // **** OpenGL ****
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

        // Specifies the robustness strategy to be used by the context.
        //glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS, GLFW_NO_RESET_NOTIFICATION);
        //glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS, GLFW_LOSE_CONTEXT_ON_RESET);
        //glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS, GLFW_NO_ROBUSTNESS);

        // Specifies the release behavior to be used by the context.
        // https://registry.khronos.org/OpenGL/extensions/KHR/KHR_context_flush_control.txt
        glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR, GLFW_ANY_RELEASE_BEHAVIOR);
        //glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR, GLFW_RELEASE_BEHAVIOR_FLUSH);
        //glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR, GLFW_RELEASE_BEHAVIOR_NONE);

        glfwWindowHint(GLFW_CONTEXT_NO_ERROR, GLFW_FALSE);
#ifdef FLOX_SHOW_DEBUG_INFO
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif // FLOX_SHOW_DEBUG_INFO

        // **** GLFW ****
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
        glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
        glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
        glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_TRUE);
        glfwWindowHint(GLFW_FLOATING, GLFW_FALSE);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);
        glfwWindowHint(GLFW_CENTER_CURSOR, GLFW_TRUE);
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
        glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_TRUE);
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, true);
        glfwWindowHint(GLFW_SAMPLES, 0);
        glfwWindowHint(GLFW_SRGB_CAPABLE, true);
        glfwWindowHint(GLFW_REFRESH_RATE, 60);

        /* Create a GLFW window and its OpenGL context. */
        GLFWwindow *p_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
        glfwMakeContextCurrent(p_window);
        if (p_window == nullptr) {
            glfwSetWindowUserPointer(p_window, nullptr);
            glfwDestroyWindow(p_window);
            glfwPollEvents();
            status = WindowCreationFailed;
            return Window{p_window};
        }

        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
            glfwSetWindowUserPointer(p_window, nullptr);
            glfwDestroyWindow(p_window);
            glfwPollEvents();
            status = OpenGLContextCreationFailed;
            return Window{p_window};
        }

        //glEnable(GL_MULTISAMPLE);

        glfwSwapInterval(1);

        // Output the current GLFW version.
        std::cout << "GLFW " << glfwGetVersionString() << std::endl;

        // Output the current OpenGL version.
        std::cout << "OpenGL " << glGetString(GL_VERSION) << std::endl;

        status = Success;
        return Window{p_window};
    }

    export bool window_should_close(const Window &window) {
        return glfwWindowShouldClose(static_cast<GLFWwindow*>(window));
    }

    export void window_should_close(const Window &window, bool should_close) {
        glfwSetWindowShouldClose(static_cast<GLFWwindow*>(window), should_close);
    }

    export void window_swap_buffers(const Window &window) {
        glfwSwapBuffers(static_cast<GLFWwindow*>(window));
    }

    export void window_clear() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    export void window_clear(const Window &window) {
        glfwMakeContextCurrent(static_cast<GLFWwindow*>(window));
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}
