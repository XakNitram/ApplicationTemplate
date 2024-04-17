module;
#include "pch.hpp"
export module Window;


namespace core {
    export struct Hints {
        const char *title{"ModelViewer"};
        const int width{800};
        const int height{640};
        GLFWmonitor *monitor{nullptr};
        GLFWwindow *share{nullptr};

        // Watch for bit-fields to be fixed in MSVC
        // https://developercommunity.visualstudio.com/t/Default-member-initializers-for-bit-fiel/10030064?q=export+bit+field
        // bool stereo : 1 {GLFW_FALSE};
        // bool double_buffered : 1 {GLFW_TRUE};

        bool stereo{GLFW_FALSE};
        bool double_buffered{GLFW_TRUE};
        bool resizable{GLFW_FALSE};
        bool visible{GLFW_TRUE};
        bool decorated{GLFW_TRUE};
        bool take_focus{GLFW_TRUE};
        bool iconify{GLFW_TRUE};
        bool floating{GLFW_FALSE};

        bool maximized{GLFW_FALSE};
        bool center_cursor{GLFW_TRUE};
        bool transparent{GLFW_FALSE};
        bool focus_on_show{GLFW_TRUE};
        bool scale_to_monitor{GLFW_TRUE};
        bool srgb_capable{GLFW_FALSE};
        // bool  : 2;

        bool opengl_forward_compatibility{GLFW_TRUE};
        bool opengl_generate_errors{GLFW_FALSE};
        bool opengl_debug_context{GLFW_TRUE};
        // bool  : 5;

        int client_api{GLFW_OPENGL_API};
        int context_creation_api{GLFW_NATIVE_CONTEXT_API};
        int opengl_profile{GLFW_OPENGL_CORE_PROFILE};
        int opengl_version_major{4};
        int opengl_version_minor{6};

        int multisample_samples{0};

        int refresh_rate{120};
        int swap_interval{0};

        Hints() = default;

        Hints(const char *title, const int width, const int height) :
            title(title), width(width), height(height) {}
    };

    export class Window final {
        GLFWwindow *p_window{nullptr};
        void *p_user{nullptr};

    public:
        //! Enum to record the success or failure of the
        enum class Status {
            GenericFailure = 0,
            GLFWUninitialized,
            WindowCreationFailed,
            ContextCreationFailed,
            Success
        };

        Window() = delete;

        explicit Window(Status &status, const Hints &hints = Hints{}, void *user_ptr = nullptr) noexcept;

        //! Manage an existing instance of a GLFW window.
        explicit Window(GLFWwindow *window, void *user_ptr = nullptr) noexcept :
            p_window(window), p_user(user_ptr) {}

        ~Window() {
            glfwSetWindowUserPointer(p_window, nullptr);
            glfwDestroyWindow(p_window);
            glfwPollEvents();
        }

        explicit operator GLFWwindow *() const { return p_window; }

        [[nodiscard]] bool should_close() const { return glfwWindowShouldClose(p_window); }

        void should_close(const bool close) const { glfwSetWindowShouldClose(p_window, close); }

        void swap() const { glfwSwapBuffers(p_window); }
        static void poll_events() { glfwPollEvents(); }
    };


    export class GLFW {
        GLFW() : init(glfwInit()) {}

        bool init;

    public:
        static GLFW &get() {
            static GLFW glfw{};
            return glfw;
        }

        ~GLFW() { glfwTerminate(); }

        [[nodiscard]] bool initialized() const { return init; }

        void restart() {
            glfwTerminate();
            init = glfwInit();
        }
    };


    /*! Create a new GLFW window.
     * @param [out] status Used to record the success or failure of window creation.
     * @param [in] hints Used to customize window creation.
     * @param [in] user_ptr Used to allow access to user state inside GLFW event callbacks.
     */
    Window::Window(Status &status, const Hints &hints, void *user_ptr) noexcept : p_user(user_ptr) {
        using enum Status;
#ifndef NDEBUG
        glfwSetErrorCallback([]([[maybe_unused]] int error_code, const char *description) {
            std::cerr << description << '\n';
        });
#endif

        if (!GLFW::get().initialized()) {
            status = GLFWUninitialized;
            return;
        }

        // Set GLFW window hints.
        // ****** Hard Constraints ******
        // * Must match available capabilities exactly.

        // Specifies whether to use OpenGL stereoscopic rendering.
        glfwWindowHint(GLFW_STEREO, hints.stereo);

        // Specifies whether the framebuffer should be double buffered. You nearly always want to use double buffering.
        glfwWindowHint(GLFW_DOUBLEBUFFER, hints.double_buffered);

        // Specifies which context creation API to use to create the context.
        glfwWindowHint(GLFW_CLIENT_API, hints.client_api);
        // glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        // glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        // An extension loader library that assumes it knows which API was used to create the current context may fail if you change this hint.
        //   This can be resolved by having it load functions via glfwGetProcAddress.
        // Maybe have to change for linux.
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, hints.context_creation_api);
        // glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_OSMESA_CONTEXT_API);
        // glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);

        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, hints.opengl_forward_compatibility);
        // Disable deprecated OpenGL operations.

        // Specifies which OpenGL profile to create the context for.
        glfwWindowHint(GLFW_OPENGL_PROFILE, hints.opengl_profile);
        // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
        // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);

        // ****** Soft Constraints ******
        // **** OpenGL ****
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, hints.opengl_version_major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, hints.opengl_version_minor);

        // Specifies the robustness strategy to be used by the context.
        // glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS, GLFW_NO_RESET_NOTIFICATION);
        // glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS, GLFW_LOSE_CONTEXT_ON_RESET);
        // glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS, GLFW_NO_ROBUSTNESS);

        // Specifies the release behavior to be used by the context.
        // https://registry.khronos.org/OpenGL/extensions/KHR/KHR_context_flush_control.txt
        // glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR, GLFW_ANY_RELEASE_BEHAVIOR);
        // glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR, GLFW_RELEASE_BEHAVIOR_FLUSH);
        // glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR, GLFW_RELEASE_BEHAVIOR_NONE);

        glfwWindowHint(GLFW_CONTEXT_NO_ERROR, hints.opengl_generate_errors);
#ifndef NDEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, hints.opengl_debug_context);
#endif // NDEBUG

        // **** GLFW ****
        glfwWindowHint(GLFW_RESIZABLE, hints.resizable);
        glfwWindowHint(GLFW_VISIBLE, hints.visible);
        glfwWindowHint(GLFW_DECORATED, hints.decorated);
        glfwWindowHint(GLFW_FOCUSED, hints.take_focus);
        glfwWindowHint(GLFW_AUTO_ICONIFY, hints.iconify);
        glfwWindowHint(GLFW_FLOATING, hints.floating);
        glfwWindowHint(GLFW_MAXIMIZED, hints.maximized);
        glfwWindowHint(GLFW_CENTER_CURSOR, hints.center_cursor);
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, hints.transparent);
        glfwWindowHint(GLFW_FOCUS_ON_SHOW, hints.focus_on_show);
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, hints.scale_to_monitor);
        glfwWindowHint(GLFW_SAMPLES, hints.multisample_samples);
        glfwWindowHint(GLFW_SRGB_CAPABLE, hints.srgb_capable);
        glfwWindowHint(GLFW_REFRESH_RATE, hints.refresh_rate);

        /* Create a GLFW window and its OpenGL context. */
        p_window = glfwCreateWindow(hints.width, hints.height, hints.title, hints.monitor, hints.share);
        glfwMakeContextCurrent(p_window);
        if (p_window == nullptr) {
            glfwSetWindowUserPointer(p_window, nullptr);
            glfwDestroyWindow(p_window);
            glfwPollEvents();
            status = WindowCreationFailed;
            return;
        }

        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
            glfwSetWindowUserPointer(p_window, nullptr);
            glfwDestroyWindow(p_window);
            glfwPollEvents();
            status = ContextCreationFailed;
            return;
        }

        if (hints.multisample_samples > 0) { glEnable(GL_MULTISAMPLE); }

        glfwSwapInterval(hints.swap_interval);

        // Output the current GLFW version.
        std::cout << "GLFW " << glfwGetVersionString() << std::endl;

        // Output the current OpenGL version.
        std::cout << "OpenGL " << glGetString(GL_VERSION) << std::endl;

        glfwSetKeyCallback(
            p_window, [](
                GLFWwindow *window, const int key,
                [[maybe_unused]] const int scancode,
                [[maybe_unused]] const int action,
                [[maybe_unused]] const int mods
            ) {
                if (key == GLFW_KEY_ESCAPE) { glfwSetWindowShouldClose(window, true); }
            }
        );

        status = Success;
    }
} // namespace core
