#include "lwvl/lwvl.hpp"

namespace lwvl {
    const char *source_to_string(GLenum source) {
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

    void draw_arrays(GLenum mode, GLsizei count, GLint first) {
        glDrawArrays(mode, first, count);
    }

    void draw_elements(GLenum mode, GLsizei count, GLenum type) {
        glDrawElements(mode, count, type, nullptr);
    }
}
