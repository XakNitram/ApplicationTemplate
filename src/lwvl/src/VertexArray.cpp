#include "lwvl/lwvl.hpp"


namespace lwvl {
    GLuint VertexArray::reserve() {
        GLuint temp;
        glCreateVertexArrays(1, &temp);
        return temp;
    }

    VertexArray::VertexArray() : id(reserve()) {}

    VertexArray::VertexArray(GLuint n_id) : id(n_id) {}

    VertexArray::operator GLuint() const {
        return id;
    }

    VertexArray::VertexArray(VertexArray &&other)  noexcept {
        glDeleteVertexArrays(1, &id);
        id = other.id;
        other.id = 0;
    }

    VertexArray &VertexArray::operator=(VertexArray &&other) noexcept {
        glDeleteVertexArrays(1, &id);
        id = other.id;
        other.id = 0;
        return *this;
    }

    VertexArray::~VertexArray() {
        glDeleteVertexArrays(1, &id);
    }

    void VertexArray::add_buffer(
        const VertexArray &vao, const Buffer &vbo, GLint bi, GLsizei stride, GLintptr offset
    ) {
        glVertexArrayVertexBuffer(static_cast<GLuint>(vao), bi, static_cast<GLuint>(vbo), offset, stride);
    }

    void VertexArray::add_element_buffer(const VertexArray &vao, const Buffer &vbo) {
        glVertexArrayElementBuffer(static_cast<GLuint>(vao), static_cast<GLuint>(vbo));
    }

    void VertexArray::add_attribute(
        const VertexArray &vao, GLuint ai, GLint dimensions, GLenum type, GLuint offset
    ) {
        glEnableVertexArrayAttrib(static_cast<GLuint>(vao), ai);

        // The "type" parameter refers to the type of the data in the buffer, and the function called here represents the type of the data in the shader.
        // I'm assuming here that the user intends them to be the same because I have never personally wanted OpenGL to massage ints into floats or floats into ints.
        switch (type) {
            case GL_INT:
            case GL_INT_2_10_10_10_REV:
            case GL_UNSIGNED_INT:
            case GL_UNSIGNED_INT_2_10_10_10_REV:
            case GL_UNSIGNED_INT_10F_11F_11F_REV: {
                glVertexArrayAttribIFormat(static_cast<GLuint>(vao), ai, dimensions, type, offset);
                break;
            }
            case GL_DOUBLE: {
                glVertexArrayAttribLFormat(static_cast<GLuint>(vao), ai, dimensions, type, offset);
                break;
            }
#ifdef NDEBUG
                default: { __assume(false); }
#else
            default: {
                std::cerr << "Invalid type passed to vertex_array_add_attribute." << '\n';
            }
#endif
            case GL_HALF_FLOAT:
            case GL_FLOAT: {
                glVertexArrayAttribFormat(static_cast<GLuint>(vao), ai, dimensions, type, GL_FALSE, offset);
                break;
            }
        }
    }

    void VertexArray::use_binding(const VertexArray &vao, GLuint binding_index, GLuint attribute_index) {
        glVertexArrayAttribBinding(static_cast<GLuint>(vao), attribute_index, binding_index);
    }

    void VertexArray::activate(const VertexArray &vao) {
        glBindVertexArray(static_cast<GLuint>(vao));
    }

    void VertexArray::clear() {
        glBindVertexArray(0);
    }
}
