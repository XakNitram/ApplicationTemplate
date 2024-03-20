#include "lwvl/lwvl.hpp"

namespace lwvl {
    Buffer::Buffer() : id(reserve()) {}

    GLuint Buffer::reserve() {
        GLuint temp;
        glCreateBuffers(1, &temp);
        return temp;
    }

    Buffer::Buffer(GLuint n_id) : id(n_id) {}

    Buffer::operator GLuint() const {
        return id;
    }

    Buffer::Buffer(Buffer &&other)  noexcept {
        glDeleteBuffers(1, &id);
        id = other.id;
        other.id = 0;
    }

    Buffer &Buffer::operator=(Buffer &&other) noexcept {
        // I don't know where to fall on this.
        // Destroying the buffer here frees up buffers for use if the user keeps moving buffers onto the first one.
        // However, assigning this->id to other.id allows the destructor for that object to clean up the buffer at the end of the scope.
        // Maybe some static boolean that could disable destruction here.
        glDeleteBuffers(1, &id);
        id = other.id;
        other.id = 0;
        return *this;
    }

    Buffer::~Buffer() {
        // Deleting id = 0 has no effect. Don't need to check.
        glDeleteBuffers(1, &id);
    }
}
