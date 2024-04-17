#include "lwvl/lwvl.hpp"

namespace lwvl {
    const char *source_to_string(const GLenum source) {
        switch(source) {
            case GL_DEBUG_SOURCE_API:
                return "[API]";
            case GL_DEBUG_SOURCE_APPLICATION:
                return "[Application]";
            case GL_DEBUG_SOURCE_OTHER:
                return "[Other]";
            case GL_DEBUG_SOURCE_SHADER_COMPILER:
                return "[ShaderCompiler]";
            case GL_DEBUG_SOURCE_THIRD_PARTY:
                return "[ThirdParty]";
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
                return "[WindowSystem]";
            default:
                return "[UNKNOWN]";
        }
    }

    void clear() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void draw_arrays(const GLenum mode, const GLsizei count, const GLint first) {
        glDrawArrays(mode, first, count);
    }

    void draw_elements(const GLenum mode, const GLsizei count, const GLenum type) {
        glDrawElements(mode, count, type, nullptr);
    }
}
