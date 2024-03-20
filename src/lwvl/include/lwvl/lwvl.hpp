#pragma once

// Figure out how to make this library independent.
#include <glad/glad.h>

#include <string>
#include <sstream>
#include <memory>


#ifndef NDEBUG

#include <iostream>


#define LWVL_PRINT_ERRORS
#endif

namespace lwvl {
    const char *source_to_string(GLenum source);


    class Buffer {
        GLuint id;

        static GLuint reserve();
    public:
        Buffer();
        explicit Buffer(GLuint);
        explicit operator GLuint() const;

        Buffer(const Buffer &) = delete;
        Buffer &operator=(const Buffer &) = delete;

        Buffer(Buffer &&) noexcept;
        Buffer &operator=(Buffer &&) noexcept;

        ~Buffer();

        template<typename T>
        static void fill(const Buffer &buffer, T const *data, const GLsizeiptr size, const GLenum usage) {
            glNamedBufferData(static_cast<GLuint>(buffer), size, data, usage);
        }

        template<class Iterator>
        static void fill(const Buffer &buffer, const Iterator first, const Iterator last, const GLenum usage) {
            glNamedBufferData(static_cast<GLuint>(buffer), sizeof(*first) * (last - first), std::addressof(*first), usage);
        }

        template<typename T>
        static void const_fill(const Buffer &buffer, T const *const data, const GLsizeiptr size, const GLbitfield usage = 0) {
            glNamedBufferStorage(static_cast<GLuint>(buffer), size, data, usage);
        }

        template<class Iterator>
        static void const_fill(const Buffer &buffer, const Iterator first, const Iterator last, const GLbitfield usage = 0) {
            glNamedBufferStorage(static_cast<GLuint>(buffer), sizeof(*first) * (last - first), std::addressof(*first), usage);
        }

        template<typename T>
        static void fill_slice(const Buffer &buffer, T const *const data, const GLsizeiptr size, const GLsizei offset = 0) {
            glNamedBufferSubData(static_cast<GLuint>(buffer), offset, size, data);
        }

        template<class Iterator>
        static void fill_slice(const Buffer &buffer, const Iterator first, const Iterator last, const GLsizei offset = 0) {
            glNamedBufferSubData(static_cast<GLuint>(buffer), offset, sizeof(*first) * (last - first), std::addressof(*first));
        }
    };


    class VertexArray {
        GLuint id;

        static GLuint reserve();
    public:
        VertexArray();
        explicit VertexArray(GLuint);
        explicit operator GLuint() const;

        VertexArray(const VertexArray &) = delete;
        VertexArray &operator=(const VertexArray &) = delete;

        VertexArray(VertexArray &&) noexcept;
        VertexArray &operator=(VertexArray &&) noexcept;

        ~VertexArray();

        static void add_buffer(const VertexArray &, const Buffer &, GLint binding_index, GLsizei stride, GLintptr offset = 0);
        static void add_element_buffer(const VertexArray &, const Buffer &);
        static void add_attribute(const VertexArray &, GLuint attribute_index, GLint dimension, GLenum type, GLuint offset = 0);
        static void use_binding(const VertexArray &, GLuint binding_index, GLuint attribute_index);
        static void activate(const VertexArray &);
        static void clear();
    };


    class Texture {
        GLuint id;

        static GLuint reserve(GLenum);
    public:
        explicit Texture(GLenum);
        explicit Texture(GLenum, GLuint);
        explicit operator GLuint() const;

        Texture(const Texture &) = delete;
        Texture &operator=(const Texture &) = delete;

        Texture(Texture &&) noexcept;
        Texture &operator=(Texture &&) noexcept;

        ~Texture();
    };


    class Shader;


    struct Uniform;


    class Program {
        GLuint id;

        static GLuint reserve();
    public:
        Program();
        explicit Program(GLuint);
        explicit operator GLuint() const;

        Program(const Program &) = delete;
        Program &operator=(const Program &) = delete;

        Program(Program &&) noexcept;
        Program &operator=(Program &&) noexcept;

        ~Program();

        enum class LinkStatus {
            Success,
            LinkFailed,
            ValidationFailed
        };

        static void link(const Program &, const Shader *stages, GLsizei count, LinkStatus &status);

        static Uniform uniform(const Program &, const char *name);
        static Uniform uniform(const Program &, const std::string &name);

        static GLint uniform_location(const Program &, const char *name);
        static GLint uniform_location(const Program &, const std::string &name);

        static void activate(const Program &);
        static void clear();
    };


    class Shader {
        GLuint id;

        static GLuint reserve(GLenum type);
    public:
        explicit Shader(GLenum type);
        Shader(GLuint, GLenum type);
        explicit operator GLuint() const;

        Shader(const Shader &) = delete;
        Shader &operator=(const Shader &) = delete;

        Shader(Shader &&) noexcept;
        Shader &operator=(Shader &&) noexcept;

        ~Shader();
    };


    void draw_arrays(GLenum mode, GLsizei count, GLint first = 0);
    void draw_elements(GLenum mode, GLsizei count, GLenum type);


    struct Uniform {
        GLint location;

        Uniform(const Program &, const char *name);
        Uniform(const Program &, const std::string &name);
        explicit Uniform(GLint);
    };
}
