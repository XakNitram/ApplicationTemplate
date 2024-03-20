#include "lwvl/lwvl.hpp"

namespace lwvl {
    GLuint Program::reserve() {
        return glCreateProgram();
    }

    Program::Program(): id(reserve()) {}

    Program::Program(const GLuint n_id): id(n_id) {}

    Program::operator GLuint() const { return id; }

    Program::Program(Program &&other)  noexcept {
        glDeleteProgram(id);
        id = other.id;
        other.id = 0;
    }

    Program &Program::operator=(Program &&other) noexcept {
        glDeleteProgram(id);
        id = other.id;
        other.id = 0;
        return *this;
    }

    Program::~Program() {
        glDeleteProgram(id);
    }

    void Program::activate(const Program &program) {
        glUseProgram(static_cast<GLuint>(program));
    }

    void Program::clear() {
        glUseProgram(0);
    }

    void Program::link(const Program &_program, const Shader *stages, GLsizei count, LinkStatus &status) {
        GLuint program { static_cast<GLuint>(_program) };
        for (GLsizei i = 0; i < count; ++i) {
            const Shader& stage = stages[i];
            glAttachShader(program, static_cast<GLuint>(stage));
        }

        glLinkProgram(program);
        const GLint is_linked {
            [](GLuint program){
                GLint temp;
                glGetProgramiv(program, GL_LINK_STATUS, &temp);
                return temp;
            }(program)
        };

        if (is_linked == GL_FALSE) {
#ifdef LWVL_PRINT_ERRORS
            GLint max_length = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_length);

            // The maxLength includes the NULL character
            std::unique_ptr<GLchar[]> info_log {std::make_unique<GLchar[]>(max_length)};
            glGetProgramInfoLog(program, max_length, &max_length, std::addressof(info_log[0]));

            // Provide the infolog in whatever manner you deem best.
            std::cerr << "Failed to link program:" << '\n' << info_log << '\n';
#endif

            // Exit with failure.
            status = Program::LinkStatus::LinkFailed;
            return;
        }

        glValidateProgram(program);
        const GLint is_valid {
            [](GLuint program){
                GLint temp;
                glGetProgramiv(program, GL_VALIDATE_STATUS, &temp);
                return temp;
            }(program)
        };

        if (is_valid == GL_FALSE) {
#ifdef LWVL_PRINT_ERRORS
            GLint max_length = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_length);

            // The maxLength includes the NULL character
            std::unique_ptr<GLchar[]> info_log {std::make_unique<GLchar[]>(max_length)};
            glGetProgramInfoLog(program, max_length, &max_length, std::addressof(info_log[0]));

            // Provide the infolog in whatever manner you deem best.
            std::cerr << "Failed to validate program:" << '\n' << info_log << '\n';
#endif

            // Exit with failure.
            status = Program::LinkStatus::ValidationFailed;
            return;
        }

        for (GLsizei i = 0; i < count; ++i) {
            const Shader& stage = stages[i];
            glDetachShader(static_cast<GLuint>(program), static_cast<GLuint>(stage));
        }
    }

    Uniform Program::uniform(const Program &program, const char *name) {
        return { program, name };
    }

    Uniform Program::uniform(const Program &program, const std::string &name) {
        return { program, name };
    }

    GLint Program::uniform_location(const Program &program, const char *name) {
        return glGetUniformLocation(static_cast<GLuint>(program), name);
    }

    GLint Program::uniform_location(const Program &program, const std::string &name) {
        return glGetUniformLocation(static_cast<GLuint>(program), name.c_str());
    }


    GLuint Shader::reserve(const GLenum type) {
        return glCreateShader(type);
    }

    Shader::Shader(const GLenum type) : id(reserve(type)) {
    }

    Shader::Shader(const GLuint n_id, const GLenum) : id(n_id) {
    }

    Shader::operator GLuint() const {
        return id;
    }

    Shader::Shader(Shader &&other) noexcept {
        glDeleteShader(id);
        id = other.id;
        other.id = 0;
    }

    Shader &Shader::operator=(Shader &&other) noexcept {
        glDeleteShader(id);
        id = other.id;
        other.id = 0;
        return *this;
    }

    Shader::~Shader() {
        glDeleteShader(id);
    }


    Uniform::Uniform(const Program &program, const char *name) : location(Program::uniform_location(program, name)) {}

    Uniform::Uniform(const Program &program, const std::string &name) : location(Program::uniform_location(program, name)) {}

    Uniform::Uniform(GLint n_location) : location(n_location) {}
}
